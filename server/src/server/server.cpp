#include <server/server.hpp>

namespace MPG
{

Server::Server(const std::shared_ptr<Config>& conf, const std::shared_ptr<Logger>& log)
{
    //core_ptr = std::make_unique<Core>(conf, log);
    server_ptr = std::make_unique<wfrest::HttpServer>();

    server_ptr->POST("/add-exhibit", bind(&Server::addExhibit, this));
    server_ptr->POST("/get-exhibit", bind(&Server::getExhibit, this));
}

int Server::start()
{
    return server_ptr->start(8888);
}


void Server::addExhibit(const wfrest::HttpReq* req, wfrest::HttpResp* resp)
{
    std::cout << "add exhibit\n";
}

void Server::getExhibit(const wfrest::HttpReq* req, wfrest::HttpResp* resp)
{
    std::cout << "get exhibit\n";
}


}