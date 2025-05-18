#include <server/server.hpp>

int main (int argc, char** argv)
{

    std::shared_ptr<MPG::Config> conf = std::make_shared<MPG::Config>();
    std::shared_ptr<MPG::Logger> logger = std::make_shared<MPG::Logger>();
    MPG::Server server(conf, logger);

    server.start();

    while(true) {}


    return 0;
}