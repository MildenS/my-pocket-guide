#pragma once

#include <core_module/core.hpp>
#include <server/server_utils.hpp>

namespace MPG{

class Server
{

public:

    Server();

    int start();

protected:

    void addExhibit(wfrest::HttpReq* req, wfrest::HttpResp* resp);
    void getExhibit(wfrest::HttpReq* req, wfrest::HttpResp* resp);


private:

    std::unique_ptr<Core> core_ptr;
    std::unique_ptr<wfrest::HttpServer> server_ptr;

};

}