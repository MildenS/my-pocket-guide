#pragma once
#include <wfrest/HttpServer.h>
#include "wfrest/base64.h"



namespace MPG
{


template<typename TController>
auto bind(void (TController::*handler)(const wfrest::HttpReq*, wfrest::HttpResp*), TController *controller) -> wfrest::Handler
{
    using std::placeholders::_1, std::placeholders::_2;
    return static_cast<wfrest::Handler>(std::bind(handler, controller, _1, _2));
}

template<typename TController>
auto bind(void handler(const wfrest::HttpReq*, wfrest::HttpResp*), TController *controller) -> wfrest::Handler
{
    using std::placeholders::_1, std::placeholders::_2;
    return static_cast<wfrest::Handler>(std::bind(handler, controller, _1, _2));
}

inline void to_json(nlohmann::json& j, const DatabaseResponse& db_resp) {
    j = nlohmann::json{
        {"exhibit_id", std::move(db_resp.exhibit_id)},
        {"exhibit_title", std::move(db_resp.exhibit_name)},
        {"exhibit_description", std::move(db_resp.exhibit_description)},
        {"exhibit_image", wfrest::Base64::encode(db_resp.exhibit_image.data(), 
                                     db_resp.exhibit_image.size())}
    };
}


}