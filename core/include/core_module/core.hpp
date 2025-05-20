#pragma once

#include <database_module/database.hpp>
#include <core_module/core_utils.hpp>
#include <config.hpp>
#include <logger.hpp>

#include <opencv2/features2d/features2d.hpp>

namespace MPG

{

class Core
{

public:

    using ORBPtr = cv::Ptr<cv::ORB>;

    Core(const std::shared_ptr<Config>& conf, const std::shared_ptr<Logger>& log);
    virtual std::optional<CoreResponse> getExhibit(std::vector<uint8_t>&& exhibit_image);
    virtual bool addExhibit(const CoreRequest& req);
    virtual ~Core(){};


protected:

    std::unique_ptr<DatabaseModule> db;

    std::shared_ptr<Config> config;
    std::shared_ptr<Logger> logger;

    bool initORBPool();
    ORBPtr getORB();
    void returnORB(ORBPtr orb);
    ORBPool orb_pool;

private:
    std::optional<CoreResponse> getCoreResponse(const DatabaseResponse& db_resp);
    std::optional<DatabaseRequest> getDatabaseRequest(const CoreRequest& db_resp);
};


}