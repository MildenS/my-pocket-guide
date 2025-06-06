#pragma once

#include <core_module/core.hpp>
#include <server/server_utils.hpp>

namespace MPG{

/**
 * \brief Class for work with network by HTTP queries
 */
class Server
{

public:

    Server(const std::shared_ptr<Config>& conf, const std::shared_ptr<Logger>& log);

    int start();
    void stop();

protected:

    void addExhibit(const wfrest::HttpReq* req, wfrest::HttpResp* resp);
    void getExhibit(const wfrest::HttpReq* req, wfrest::HttpResp* resp);
    void deleteExhibit(const wfrest::HttpReq* req, wfrest::HttpResp* resp);
    void getDatabaseChunk(const wfrest::HttpReq* req, wfrest::HttpResp* resp);

    std::unique_ptr<Core> core_ptr;
    std::unique_ptr<wfrest::HttpServer> server_ptr;
    std::shared_ptr<Config> config_ptr; 
    std::shared_ptr<Logger> logger_ptr;

};

}