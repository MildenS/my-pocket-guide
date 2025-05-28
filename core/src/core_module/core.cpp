#include <core_module/core.hpp>


namespace MPG
{

    /**
     * \brief Constructor of core class object
     * \param[in] conf Smart pointer to configuration of project
     * \param[in] log Smart pointer to global logger
     */
    Core::Core(const std::shared_ptr<Config>& conf, const std::shared_ptr<Logger>& log)
    {
        db = std::make_unique<DatabaseModule>(conf, log);
        db->init();

        config = conf;
        logger = log;
        initORBPool();
    }


     /**
     * \brief Method for get object info by its image
     * \param[in] exhibit_image Object image (.jpg)
     * \return Object info if success or std::nullopt
     */
    std::optional<CoreResponse> Core::getExhibit(std::vector<uint8_t>&& exhibit_image)
    {
        std::vector<cv::KeyPoint> kps;
        cv::Mat descr;
        cv::Mat exhibit_image_mat = cv::imdecode(exhibit_image, cv::IMREAD_GRAYSCALE);
        //kps.reserve(100);
        ORBPtr orb = getORB();
        orb->detectAndCompute(exhibit_image_mat, cv::noArray(), kps, descr);
        returnORB(orb);
        std::optional<DatabaseResponse> db_resp = db->getExhibit(descr);

        if (!db_resp)
            return std::nullopt;

        return getCoreResponse(db_resp.value());
    }


    /**
     * \brief Method for add new object to system
     * \param[in] req Object info
     * \return true if success 
     */
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


    /**
     * \brief Internal method for init ORB detectors pool
     * \return true if success
     */
    bool Core::initORBPool()
    {
        orb_pool.pool = std::queue<ORBPtr>();
        const size_t pool_size = config->orb_pool_size;

        for (size_t i = 0; i < pool_size; ++i)
        {
            ORBPtr orb = cv::ORB::create(config->orb_kps_count);
            
            orb_pool.pool.push(orb);
        }
        return true;
    }


    /**
     * \brief Internal method for get ORB detector from pool
     * \return ORB detector (wait untill at least one will be available)
     * 
     * Don't forget to return detector to pool!
     */
    Core::ORBPtr Core::getORB()
    {
        std::unique_lock<std::mutex> ul(orb_pool.mtx);
        orb_pool.cv.wait(ul, [this]
                              { return !orb_pool.pool.empty(); });

        ORBPtr orb = orb_pool.pool.front();
        orb_pool.pool.pop();

        return orb;
    }


    /**
     * \brief Internal method for return ORB detector to pool
     * \param[in] orb ORB detector object
     */
    void Core::returnORB(ORBPtr orb)
    {
        std::lock_guard<std::mutex> lg(orb_pool.mtx);
        orb_pool.pool.push(orb);

        orb_pool.cv.notify_one();
    }


    /**
     * \brief Method for get object info in core types from database module types
     * \param[in] db_resp Object info in database types
     * \return Object info in core types if successful or std::nullopt
     */  
    std::optional<CoreResponse> Core::getCoreResponse(const DatabaseResponse& db_resp)
    {
        CoreResponse resp;

        resp.exhibit_id = std::move(db_resp.exhibit_id);
        resp.exhibit_description = std::move(db_resp.exhibit_description);
        resp.exhibit_name = std::move(db_resp.exhibit_name);
        resp.exhibit_image = std::move(db_resp.exhibit_image);

        return resp;
    }

    /**
     * \brief Method for get object info in database module types from core types
     * \param[in] req Object info in core types
     * \return Object info in database module types success or std::nullopt
     */
    std::optional<DatabaseRequest> Core::getDatabaseRequest(const CoreRequest &req)
    {
        DatabaseRequest db_req;
        db_req.exhibit_description = std::move(req.exhibit_description);
        db_req.exhibit_title = std::move(req.exhibit_title);
        db_req.exhibit_image = std::move(req.exhibit_main_image);
        cv::Ptr<cv::ORB> orb = getORB();
        cv::Mat all_descriptor;
        std::vector<cv::KeyPoint> all_kps;

        for (const auto image_buffer : req.exhibit_descriptor_images)
        {
            cv::Mat image = cv::imdecode(image_buffer, cv::IMREAD_COLOR);
            cv::Mat curr_descriptor;
            std::vector<cv::KeyPoint> kps;
            orb->detectAndCompute(image, cv::noArray(), kps, curr_descriptor);

            all_descriptor.push_back(curr_descriptor);
            all_kps.insert(all_kps.end(), kps.begin(), kps.end());
        }

        std::vector<int> indices(all_kps.size());
        std::iota(indices.begin(), indices.end(), 0);

        std::sort(indices.begin(), indices.end(), [&](int a, int b)
                  { return all_kps[a].response > all_kps[b].response; });

        int top_n = std::min(static_cast<int>(config->max_descriptor_size), static_cast<int>(indices.size()));
        cv::Mat final_descriptors;

        for (int i = 0; i < top_n; ++i)
        {
            final_descriptors.push_back(all_descriptor.row(indices[i]));
        }

        db_req.exhibit_descriptor = std::move(final_descriptors);

        return db_req;
    }

    /**
     * \brief Method for delete object from database by its id
     * \param[in] exhibit_id Object id (cass uuid in string format)
     * \return true if success
     */
    bool Core::deleteExhibit(const std::string& exhibit_id)
    {
        return db->deleteExhibit(exhibit_id);
    }

    /**
     * \brief Method for get database chunk
     * \param[in] next_chunk_token Cassandra token for next chunk(page)
     * \return Database chunk if success or std::nullopt
     */
    std::optional<DatabaseChunk> Core::getDatabaseChunk(const std::string& next_chunk_token)
    {
        return db->getDatabaseChunk(next_chunk_token);

    }
}