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

        return getCoreResponse(db_resp.value());
    }


    bool Core::addExhibit(const CoreRequest& req)
    {
        std::optional<DatabaseRequest> db_req = getDatabaseRequest(req);

        if (!db_req)
        {
            return false;
        }

        bool is_added = db->addExhibit(db_req.value());

        return is_added;
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

    std::optional<CoreResponse> Core::getCoreResponse(const DatabaseResponse& db_resp)
    {
        std::optional<CoreResponse> resp;

        resp.value().exhibit_description = db_resp.exhibit_description;
        resp.value().exhibit_name = db_resp.exhibit_name;
        bool success = cv::imencode(".jpg", db_resp.exhibit_image, resp.value().exhibit_image);
        if (!success)
            std::cerr << "Core: invalid image of exhibit with name " << db_resp.exhibit_name << std::endl;
        return resp;
    }

    std::optional<DatabaseRequest> Core::getDatabaseRequest(const CoreRequest &req)
    {
        DatabaseRequest db_req;
        db_req.exhibit_description = req.exhibit_description;
        db_req.exhibit_title = req.exhibit_title;
        db_req.exhibit_image = cv::imdecode(req.exhibit_main_image, cv::IMREAD_COLOR);
        cv::Ptr<cv::ORB> orb = getORB();
        cv::Mat all_descriptor;
        std::vector<cv::KeyPoint> all_kps;

        for (const auto image_buffer : req.exhibit_descriptor_images)
        {
            cv::Mat image = cv::imdecode(image_buffer, cv::IMREAD_COLOR);
            cv::Mat resized_image;
            //cv::resize(image, resized_image, cv::Size(image.cols / 4, image.rows / 8));

            cv::Mat curr_descriptor;
            std::vector<cv::KeyPoint> kps;
            orb->detectAndCompute(resized_image, cv::noArray(), kps, curr_descriptor);

        }

        std::vector<int> indices(all_kps.size());
        std::iota(indices.begin(), indices.end(), 0);

        std::sort(indices.begin(), indices.end(), [&](int a, int b)
                  { return all_kps[a].response > all_kps[b].response; });

        int top_n = std::min(100, static_cast<int>(indices.size()));
        cv::Mat final_descriptors;

        for (int i = 0; i < top_n; ++i)
        {
            final_descriptors.push_back(all_descriptor.row(indices[i]));
        }

        db_req.exhibit_descriptor = final_descriptors;

        return db_req;
    }
}