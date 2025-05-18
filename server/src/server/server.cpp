#include <server/server.hpp>
#include "wfrest/base64.h"

namespace MPG
{

Server::Server(const std::shared_ptr<Config>& conf, const std::shared_ptr<Logger>& log)
{
    core_ptr = std::make_unique<Core>(conf, log);
    server_ptr = std::make_unique<wfrest::HttpServer>();

    config_ptr = conf;
    logger_ptr = log;

    server_ptr->POST("/add-exhibit", bind(&Server::addExhibit, this));
    server_ptr->POST("/get-exhibit", bind(&Server::getExhibit, this));

    logger_ptr->LogInfo("Server: server created!");
}

int Server::start()
{
    return server_ptr->start(8888);
}


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
    auto exhibit_info = core_ptr->getExhibit(exhibit_image);
    if (!exhibit_info.has_value())
    {
        resp->set_status(HttpStatusBadRequest);
    }
    else
    {
        nlohmann::json data_json;
        data_json["exhibit_title"] = exhibit_info.value().exhibit_name;
        data_json["exhibit_description"] = exhibit_info.value().exhibit_description;
        data_json["exhibit_image"] = wfrest::Base64::encode(exhibit_info.value().exhibit_image.data(), 
                                     exhibit_info.value().exhibit_image.size());
        resp->Json(data_json.dump());
    }
}


}