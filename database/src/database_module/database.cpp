#include "database_module/database.hpp"
#include <iostream>
#include <thread>

#include <opencv2/imgcodecs.hpp>


namespace MPG
{

    /**
     * \brief Constructor of DatabaseModule class
     * \todo Add config and logger 
     */
    DatabaseModule::DatabaseModule()
    {
        config = std::make_shared<Config>();
        logger = std::make_shared<Logger>();
        cluster_ptr.reset(cass_cluster_new());
        session_ptr.reset(cass_session_new());
        id_generator_ptr.reset(cass_uuid_gen_new());
        cass_cluster_set_contact_points(cluster_ptr.get(), "localhost");
        logger->LogInfo("Database module created");
    }

    DatabaseModule::DatabaseModule(std::shared_ptr<Config> conf, std::shared_ptr<Logger> log)
    {
        config = conf;
        log = logger;
        DatabaseModule();
    }

    /**
     * \brief Method for init connection to database (must be call after construction of DatabaseModule object)
     * \todo Add config and logger
     */
    [[nodiscard]] bool DatabaseModule::init()
    {
        if (!ConnectToDatabase(config->max_connect_retries, config->connect_retry_delay_ms))
        {
            logger->LogCritical("Cannot connect to database\n");
            return false;
        }

        if (!loadDatabase())
        {
            logger->LogCritical("Error load local database\n");
            return false;
        }

        if (!initMatchersPool())
        {
            logger->LogCritical("Error init matchrs pool\n");
            return false;
        }
        logger->LogInfo("Database module initialized");
        return true;
    }

    /**
     * \brief Method for connect to database
     * \todo add config and logger
     * \param[in] max_retries Count of tries of connect to databse
     * \param[in] retry_delay_ms Delay in ms between 2 tries connecting to database 
     */
    [[nodiscard]] bool DatabaseModule::ConnectToDatabase(size_t max_retries, size_t retry_delay_ms)
    {
        for (size_t i = 0; i < max_retries; ++i)
        {
            FuturePtr connect_future_ptr;
            connect_future_ptr.reset(cass_session_connect(session_ptr.get(), cluster_ptr.get()));
            CassError rc = cass_future_error_code(connect_future_ptr.get());
            if (rc == CASS_OK)
                return true;
            logger->LogWarning("Connection failed (attempt " + std::to_string(i + 1) + "/" + std::to_string(max_retries) +
             "). Retrying in "  + std::to_string(retry_delay_ms) + " ms...\n");
            const char *message;
            size_t message_length;
            cass_future_error_message(connect_future_ptr.get(), &message, &message_length);
            logger->LogWarning("Connection error: " + std::string(message, message_length));
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_delay_ms));
        }
        return false;
    }

    DatabaseModule::~DatabaseModule()
    {
        logger->LogInfo("Finish work of database module");
    }

    
    [[nodiscard]] bool DatabaseModule::loadDatabase()
    {
        local_database_descriptor = cv::Mat(0, 32, CV_8UC1); // 32 - size of ORB descriptor
        local_descriptor_to_id_map.clear();

        StatementPtr load_database_statement_ptr;
        load_database_statement_ptr.reset(cass_statement_new("select id, descriptor from mpg_keyspace.exhibits", 0));
        FuturePtr query_future_ptr;
        query_future_ptr.reset(cass_session_execute(session_ptr.get(), load_database_statement_ptr.get()));

        CassError rc = cass_future_error_code(query_future_ptr.get());
        if (rc != CASS_OK)
        {
            const char *message;
            size_t message_length;
            cass_future_error_message(query_future_ptr.get(), &message, &message_length);

            logger->LogError("Query error (" + std::string(cass_error_desc(rc)) + "): " + std::string(message, message_length));
            return false;
        }

        QueryResultPtr result(cass_future_get_result(query_future_ptr.get()));
        
        IteratorPtr iter;
        iter.reset(cass_iterator_from_result(result.get()));

        while(cass_iterator_next(iter.get()))
        {
            const CassRow* row = cass_iterator_get_row(iter.get());

            if (!loadDatabaseHelper(row))
                return false;
        }

        return true;
    }

    [[nodiscard]] bool DatabaseModule::loadDatabaseHelper(const CassRow* row)
    {
        CassUuid id;
        cass_value_get_uuid(cass_row_get_column_by_name(row, "id"), &id);

        const cass_byte_t *descriptor_data = nullptr;
        size_t descriptor_size = 0;
        cass_value_get_bytes(cass_row_get_column_by_name(row, "descriptor"), &descriptor_data, &descriptor_size);
        constexpr size_t descriptor_length = 32; // ORB descriptor for one keypoint has 32 bytes length
        if (descriptor_size % descriptor_length != 0)
        {
            logger->LogError("DatabaseModule: Descriptor size mismatch: expected multiple of 32, got " + std::to_string(descriptor_size));
            return false;
        }
        const int num_keypoints = static_cast<int>(descriptor_size / descriptor_length);
        std::vector<uint8_t> descriptor_buffer(descriptor_data, descriptor_data + descriptor_size);
        cv::Mat descriptor_temp(num_keypoints, descriptor_length, CV_8UC1, descriptor_buffer.data());
        cv::Mat descriptor = descriptor_temp.clone();

        local_database_descriptor.push_back(descriptor);
        for (int i = 0; i < descriptor.rows; ++i)
            local_descriptor_to_id_map.push_back(id);

        return true;
    }

    [[nodiscard]] std::optional<CassUuid> DatabaseModule::findExhibitUuid(const cv::Mat& exhibit_descriptor)
    {
        PooledMatcher matcher = getMatcher();
        std::vector< std::vector<cv::DMatch> > knn_matches;
        knn_matches.reserve(config->max_matches_count);
        const int k = 2;

        matcher.matcher->knnMatch(exhibit_descriptor, knn_matches, k);
        returnMatcher(matcher);       

        if (knn_matches.size() == 0)
        {
            return std::nullopt;
        }
        
        const float ratio_threshold = config->match_ratio_threshold;
        std::unordered_map<CassUuid, uint, std::hash<CassUuid>, CassUuidEqual> good_matches;

        for (const auto& match: knn_matches)
        {
            if (match[0].distance < ratio_threshold * match[1].distance)
            {
                CassUuid best_match_id = local_descriptor_to_id_map[match[0].trainIdx];
                if (good_matches.find(best_match_id) == good_matches.end())
                    good_matches[best_match_id] = 1;
                else
                    good_matches[best_match_id]++; 
            }
        }

        if (good_matches.size() == 0)
        {
            return std::nullopt;
        }
        CassUuid best_id;
        uint best_count = 0;
        for (const auto& [id, count]: good_matches)
        {
            if (count > best_count)
            {
                best_count = count;
                best_id = id;
            }
        }

        return best_id;
    }

    [[nodiscard]] std::optional<DatabaseResponse> DatabaseModule::getExhibit(const cv::Mat& description)
    {
        /**
         * table struct:
         * * id CassUuid
         * * descriptor blob
         * * image blob
         * * height int
         * * width int
         * * title text
         * * desciption text
         */
        std::optional<CassUuid> exhibit_id = findExhibitUuid(description);
        if (!exhibit_id.has_value())
        {
            logger->LogError("DatabaseModule: Couldn't found id for exhibit");
            return std::nullopt;
        }


        StatementPtr get_exhibit_statement_ptr;
        get_exhibit_statement_ptr.reset(cass_statement_new("select image, height, width, title, description from mpg_keyspace.exhibits where id=?", 1));
        cass_statement_bind_uuid(get_exhibit_statement_ptr.get(), 0, exhibit_id.value());
        FuturePtr query_future_ptr;
        query_future_ptr.reset(cass_session_execute(session_ptr.get(), get_exhibit_statement_ptr.get()));

        CassError rc = cass_future_error_code(query_future_ptr.get());
        if (rc != CASS_OK)
        {
            const char *message;
            size_t message_length;
            cass_future_error_message(query_future_ptr.get(), &message, &message_length);

            logger->LogError("DatabaseModule: Query error (" + std::string(cass_error_desc(rc)) + "): " + 
                    std::string(message, message_length));
            return std::nullopt;
        }

        QueryResultPtr result(cass_future_get_result(query_future_ptr.get()));

        const CassRow* row = cass_result_first_row(result.get());  

        return getExhibitHelper(row);
    }

    [[nodiscard]] std::optional<DatabaseResponse> DatabaseModule::getExhibitHelper(const CassRow* row)
    {
        const cass_byte_t *image_data = nullptr;
        size_t image_size_bytes = 0;
        int image_widht, image_height;
        if (CassError err = cass_value_get_bytes(cass_row_get_column_by_name(row, "image"), &image_data, &image_size_bytes); err != CASS_OK)
        {
            logError(err, "Failed to get image from database row");
            return std::nullopt;
        };
        if (CassError err = cass_value_get_int32(cass_row_get_column_by_name(row, "height"), &image_height); err != CASS_OK)
        {
            logError(err, "Failed to get image height from database row");
            return std::nullopt;
        }
        if (CassError err = cass_value_get_int32(cass_row_get_column_by_name(row, "width"), &image_widht); err != CASS_OK)
        {
            logError(err, "Failed to get image width from database row");
            return std::nullopt;
        }
        
        std::vector<uint8_t> image_buffer(image_data, image_data + image_size_bytes);
        cv::Mat image_temp(image_height, image_widht, CV_8UC3, image_buffer.data());
        cv::Mat exhibit_image = image_temp.clone();

        const char *title_data;
        size_t title_length;
        CassError err = cass_value_get_string(cass_row_get_column_by_name(row, "title"), &title_data, &title_length);
        if (err != CASS_OK)
        {
            logError(err, "Failed to get title from database row");
            return std::nullopt;
        }
        std::string exhibit_title(title_data, title_length);


        const char *decription_data;
        size_t description_length;
        err = cass_value_get_string(cass_row_get_column_by_name(row, "title"), &decription_data, &description_length);
        if (err != CASS_OK)
        {
            logError(err, "Failed to get description from database row");
            return std::nullopt;
        }
        std::string exhibit_description(decription_data, description_length);
        
        DatabaseResponse response;
        response.exhibit_description = exhibit_description;
        response.exhibit_image = exhibit_image;
        response.exhibit_name = exhibit_title;
        return response;
    }

    void DatabaseModule::logError(CassError err, const std::string& context)
    {
        logger->LogError("[Cassandra Error] " + std::string(cass_error_desc(err)));
        if (!context.empty())
        {
            logger->LogError(" | Context: " + context);
        }
    }


    [[nodiscard]] bool DatabaseModule::addExhibit(const DatabaseRequest& exhibit_data)
    {
        StatementPtr add_exhibit_statement_ptr;
        add_exhibit_statement_ptr.reset(
            cass_statement_new("insert into mpg_keyspace.exhibits (id, image, height, width, title, description, descriptor) values (?, ?, ?, ?, ?, ?, ?)", 7));
        CassUuid exhibit_id;
        cass_uuid_gen_random(id_generator_ptr.get(), &exhibit_id);
        if (auto err = cass_statement_bind_uuid(add_exhibit_statement_ptr.get(), 0, exhibit_id); err != CASS_OK)
        {
            logError(err, "Bind id to add new exhibit query");
            return false;
        }
        if (auto err = cass_statement_bind_bytes(add_exhibit_statement_ptr.get(), 1, 
                                  reinterpret_cast<const cass_byte_t*>(exhibit_data.exhibit_image.data),
                                  exhibit_data.exhibit_image.total() * exhibit_data.exhibit_image.elemSize()); err != CASS_OK)
        {
            logError(err, "Bind image data to add new exhibit query");
            return false;
        }

        cv::Size exhibit_image_size = exhibit_data.exhibit_image.size();
        if (auto err = cass_statement_bind_int32(add_exhibit_statement_ptr.get(), 2, exhibit_image_size.height); err != CASS_OK)
        {
            logError(err, "Bind image height to add new exhibit query");
            return false;
        }
        if (auto err = cass_statement_bind_int32(add_exhibit_statement_ptr.get(), 3, exhibit_image_size.width); err != CASS_OK)
        {
            logError(err, "Bind image width to add new exhibit query");
            return false;
        }
        if (auto err = cass_statement_bind_string(add_exhibit_statement_ptr.get(), 4, exhibit_data.exhibit_title.c_str()); err != CASS_OK)
        {
            logError(err, "Bind image title to add new exhibit query");
            return false;
        }
        if (auto err = cass_statement_bind_string(add_exhibit_statement_ptr.get(), 5, exhibit_data.exhibit_description.c_str()); err != CASS_OK)
        {
            logError(err, "Bind image description to add new exhibit query");
            return false;
        }
        if (auto err = cass_statement_bind_bytes(add_exhibit_statement_ptr.get(), 6, 
                                  reinterpret_cast<const cass_byte_t*>(exhibit_data.exhibit_descriptor.data),
                                  exhibit_data.exhibit_image.total() * exhibit_data.exhibit_descriptor.elemSize()); err != CASS_OK)
        {
            logError(err, "Bind image descriptor to add new exhibit query");
            return false;
        }

        FuturePtr query_future_ptr;
        query_future_ptr.reset(cass_session_execute(session_ptr.get(), add_exhibit_statement_ptr.get()));

        CassError rc = cass_future_error_code(query_future_ptr.get());
        if (rc != CASS_OK)
        {
            const char *message;
            size_t message_length;
            cass_future_error_message(query_future_ptr.get(), &message, &message_length);

            logger->LogError("DatabaseModule: Query error (" + std::string(cass_error_desc(rc)) + "): " + 
                        std::string(message, message_length));
            return false;
        }

        std::unique_lock<std::mutex> ul(local_database_mtx);
        local_database_descriptor.push_back(exhibit_data.exhibit_descriptor);
        for (int i = 0; i < exhibit_data.exhibit_descriptor.rows; ++i)
        {
            local_descriptor_to_id_map.push_back(exhibit_id);
        }

        initMatchersPool();

        return true;
    }

    bool DatabaseModule::initMatchersPool()
    {
        std::shared_ptr<MatcherPool> new_pool = std::make_shared<MatcherPool>();
        const size_t pool_size = config->matchers_pool_size;

        for (size_t i = 0; i < pool_size; ++i)
        {
            MatcherPtr matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::BRUTEFORCE_HAMMING);
            matcher->add(local_database_descriptor);
            matcher->train();
            new_pool->pool.push(matcher);
        }

        {
            std::lock_guard<std::mutex> lock(matchers_pool_switch_mtx);
            matchers_pool = new_pool; 
        }

        return true;
    }

    PooledMatcher DatabaseModule::getMatcher()
    {
        std::shared_ptr<MatcherPool> current_pool;
        {
            std::lock_guard<std::mutex> lock(matchers_pool_switch_mtx);
            current_pool = matchers_pool;
        }

        std::unique_lock<std::mutex> ul(current_pool->mtx);
        current_pool->cv.wait(ul, [&current_pool]
                              { return !current_pool->pool.empty(); });

        MatcherPtr matcher = current_pool->pool.front();
        current_pool->pool.pop();

        return {matcher, current_pool};
    }

    void DatabaseModule::returnMatcher(PooledMatcher matcher)
    {
        const auto &origin_pool = matcher.origin_pool_ptr;

        {
            std::lock_guard<std::mutex> lg(origin_pool->mtx);
            origin_pool->pool.push(matcher.matcher);
        }

        origin_pool->cv.notify_one();
    }
}