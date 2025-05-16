#include <wfrest/HttpServer.h>



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


}