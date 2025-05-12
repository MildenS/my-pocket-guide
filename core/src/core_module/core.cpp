#include <core_module/core.hpp>


namespace MPG
{

    Core::Core(const std::shared_ptr<Config>& conf, const std::shared_ptr<Logger>& log)
    {
        db = std::make_unique<DatabaseModule>(conf, log);
        initORBPool();
    }


    std::optional<CoreResponse> Core::getExhibit(const std::vector<uint8_t>& exhibit_image)
    {
        std::vector<cv::KeyPoint> kps;
        cv::Mat descr;
        kps.reserve(100);
        ORBPtr orb = getORB();
        orb->detectAndCompute(exhibit_image, cv::noArray(), kps, descr);
        returnORB(orb);
        std::optional<DatabaseResponse> db_resp = db->getExhibit(descr);

        if (!db_resp)
            return std::nullopt;

        return getResponse(db_resp.value());
    }


    bool Core::addExhibit(const CoreRequest& req)
    {
        
    }


    bool Core::initORBPool()
    {
        orb_pool.pool = std::queue<ORBPtr>();
        const size_t pool_size = 10;

        for (size_t i = 0; i < pool_size; ++i)
        {
            ORBPtr orb = cv::ORB::create(100);
            
            orb_pool.pool.push(orb);
        }
        return true;
    }


    Core::ORBPtr Core::getORB()
    {
        std::unique_lock<std::mutex> ul(orb_pool.mtx);
        orb_pool.cv.wait(ul, [this]
                              { return !orb_pool.pool.empty(); });

        ORBPtr orb = orb_pool.pool.front();
        orb_pool.pool.pop();

        return orb;
    }


    void Core::returnORB(ORBPtr orb)
    {
        std::lock_guard<std::mutex> lg(orb_pool.mtx);
        orb_pool.pool.push(orb);

        orb_pool.cv.notify_one();
    }

}