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
    virtual std::optional<CoreResponse> getExhibit(const std::vector<uint8_t>& exhibit_image);
    virtual bool addExhibit(const CoreRequest& req);


protected:

    std::unique_ptr<DatabaseModule> db;

    bool initORBPool();
    ORBPtr getORB();
    void returnORB(ORBPtr orb);
    ORBPool orb_pool;



};


}