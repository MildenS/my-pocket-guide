#include <server/server.hpp>

namespace MPG
{

/**
     * \brief Constructor of server class object
     * \param[in] conf Smart pointer to configuration of project
     * \param[in] log Smart pointer to global logger
*/
Server::Server(const std::shared_ptr<Config>& conf, const std::shared_ptr<Logger>& log)
{
    core_ptr = std::make_unique<Core>(conf, log);
    server_ptr = std::make_unique<wfrest::HttpServer>();

    config_ptr = conf;
    logger_ptr = log;

    server_ptr->POST("/add-exhibit", bind(&Server::addExhibit, this));
    server_ptr->POST("/get-exhibit", bind(&Server::getExhibit, this));
    server_ptr->DELETE("/delete-exhibit", bind(&Server::deleteExhibit, this));
    server_ptr->GET("/get-database-chunk", bind(&Server::getDatabaseChunk, this));

    logger_ptr->LogInfo("Server: server created!");
}

/**
     * \brief Method for start server on config's port
     * \return 0 if server started success
*/
int Server::start()
{
    // WFGlobalSettings settings = GLOBAL_SETTINGS_DEFAULT;
    // settings.poller_threads = 2; 
    // settings.handler_threads = 6;  
    // settings.dns_threads = 2;
	// WORKFLOW_library_init(&settings);
    
    return server_ptr->start(config_ptr->server_port);
}

/**
     * \brief Method for stop server
*/
void Server::stop()
{
    server_ptr->stop();
    logger_ptr->LogInfo("Server: server stopped");
}

/**
     * \brief Method for processing "add-exhibit" route

     HTTP query must have next fields in body (multi-form):
        - main-image (.jpg image)
        - some image-*number* (.jpg images) - train images
        - title - string with exhibit title
        - description - string with exhibit description
*/
void Server::addExhibit(const wfrest::HttpReq* req, wfrest::HttpResp* resp)
{
    logger_ptr->LogInfo("Server: Start adding new exhibit");
    std::vector<uint8_t> exhibit_main_image;
    std::vector<std::vector<uint8_t>> exhibit_train_images;
    std::string exhibit_title, exhibit_description;
    auto& files = req->form();
    for (const auto& [key, file_info]: files)
    {
        const auto& [file_name, file_body] = file_info;
        if (key.find("main-image") != std::string::npos)
        {
            exhibit_main_image = std::vector<uint8_t>(file_body.begin(), file_body.end());
        }
        else if (key.find("image-") != std::string::npos) //params with name "image-*" where * is number of image
        {
            exhibit_train_images.push_back(std::vector<uint8_t>(file_body.begin(), file_body.end()));
        }
        else if (key.find("title") != std::string::npos)
        {
            exhibit_title = file_body;
        }
        else if (key.find("description") != std::string::npos)
        {
            exhibit_description = file_body;
        } 
        else
        {
            logger_ptr->LogWarning("Server: invalid add exhibit request param with name " + key);
        }
    }

    std::cout << "Image vector size: " << (double)exhibit_main_image.size() / (1024. * 1024.) << std::endl;

    CoreRequest core_request;
    core_request.exhibit_description = std::move(exhibit_description);
    core_request.exhibit_descriptor_images = std::move(exhibit_train_images);
    core_request.exhibit_main_image = std::move(exhibit_main_image);
    core_request.exhibit_title = std::move(exhibit_title);
    bool is_added = core_ptr->addExhibit(core_request);

    if (is_added)
    {
        resp->set_status(HttpStatusCreated);
    }
    else
    {
        resp->set_status(HttpStatusBadRequest);
    }
    
}

/**
     * \brief Method for processing "get-exhibit" route

     HTTP query must have next fields in body (multi-form):
        - exhibit-image (.jpg image) - image for searching
*/
void Server::getExhibit(const wfrest::HttpReq* req, wfrest::HttpResp* resp)
{
    logger_ptr->LogInfo("Server: Start getting exhibit");
    std::vector<uint8_t> exhibit_image;
    auto& files = req->form();
    for (const auto& [key, file_info]: files)
    {
        const auto& [file_name, file_body] = file_info;
        if (key.find("exhibit-image") != std::string::npos)
        {
            exhibit_image = std::vector<uint8_t>(file_body.begin(), file_body.end());
        }
        else
        {
            logger_ptr->LogWarning("Server: invalid add exhibit request param with name " + key);
        }
    }
    auto exhibit_info = core_ptr->getExhibit(std::move(exhibit_image));
    if (!exhibit_info.has_value())
    {
        resp->set_status(HttpStatusBadRequest);
    }
    else
    {
        nlohmann::json data_json;// = std::move(exhibit_info.value());
        data_json["exhibit_id"] = std::move(exhibit_info.value().exhibit_id);
        data_json["exhibit_title"] = std::move(exhibit_info.value().exhibit_name);
        data_json["exhibit_description"] = std::move(exhibit_info.value().exhibit_description);
        data_json["exhibit_image"] = std::move(wfrest::Base64::encode(exhibit_info.value().exhibit_image.data(), 
                                     exhibit_info.value().exhibit_image.size()));
        resp->Json(data_json.dump());
    }
}

/**
     * \brief Method for processing "delete-exhibit" route

    HTTP query must have next fields in params:
        - exhibit-id (cass uuid in string format) - image id for deleting
*/
void Server::deleteExhibit(const wfrest::HttpReq* req, wfrest::HttpResp* resp)
{
    logger_ptr->LogInfo("Server: Start delete exhibit");
    auto& exhibit_id = req->query("exhibit-id");
    if (exhibit_id == "")
    {
        resp->set_status(HttpStatusBadRequest);
        resp->String("Empty exhibit-id");
        return;
    }
    bool is_del = core_ptr->deleteExhibit(exhibit_id);
    if (!is_del)
    {
        resp->set_status(HttpStatusBadRequest);
    }
}

/**
     * \brief Method for processing "get-database-chunk" route

     HTTP quety must have next fields in params:
        - next-chunk-token (base64-encoded) - cass page noken (empty for first chunk)
*/
void Server::getDatabaseChunk(const wfrest::HttpReq* req, wfrest::HttpResp* resp)
{
    logger_ptr->LogInfo("Server: Start get database chunk");
    auto& encoded_token = req->query("next-chunk-token");  
    auto next_chunk_token = wfrest::Base64::decode(encoded_token);
    auto chunk = core_ptr->getDatabaseChunk(next_chunk_token);
    if (!chunk.has_value())
    {
        resp->set_status(HttpStatusBadRequest);
        resp->String("No chunk or empty chunk");
        return;
    }
    nlohmann::json data_json;
    data_json["next_chunk_token"] = std::move(wfrest::Base64::encode(reinterpret_cast<const unsigned char*>
                                    (chunk.value().next_chunk_token.data()), 
                                    chunk.value().next_chunk_token.size()));
    data_json["is_last_chunk"] = std::move(chunk.value().is_last_chunk);
    data_json["exhibits"] = std::move(chunk.value().exhibits);
    resp->Json(data_json.dump());
}


void to_json(nlohmann::json& j, const DatabaseResponse& db_resp) {
    j = nlohmann::json{
        {"exhibit_id", std::move(db_resp.exhibit_id)},
        {"exhibit_title", std::move(db_resp.exhibit_name)},
        {"exhibit_description", std::move(db_resp.exhibit_description)},
        {"exhibit_image", wfrest::Base64::encode(db_resp.exhibit_image.data(), 
                                     db_resp.exhibit_image.size())}
    };
}

}