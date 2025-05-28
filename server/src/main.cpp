#include <server/server.hpp>
#include <csignal>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <iostream>

std::atomic<bool> running{true};
std::mutex mtx;
std::condition_variable cond_var;

void signal_handler(int signum)
{
    std::cout << "\nSignal " << signum << " received, shutting down..." << std::endl;
    running = false;
    cond_var.notify_one();
}

int main(int argc, char** argv)
{
    std::shared_ptr<MPG::Config> conf;
    if (argc < 2)
        conf = std::make_shared<MPG::Config>();
    else
    {
        conf = std::make_shared<MPG::Config>(argv[1]);
    }
    std::shared_ptr<MPG::Logger> logger = std::make_shared<MPG::Logger>();
    MPG::Server server(conf, logger);

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    if (server.start() != 0)
    {
        logger->LogError("Server: couldn't start server!");
        return 1;
    }

    logger->LogInfo("Server: server started!");
    std::unique_lock<std::mutex> lock(mtx);
    cond_var.wait(lock, [](){ return !running.load(); });

    return 0;
}
