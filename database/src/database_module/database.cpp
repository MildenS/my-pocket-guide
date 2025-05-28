#include "database_module/database.hpp"
#include <iostream>
#include <thread>

#include <opencv2/imgcodecs.hpp>


namespace MPG
{

    /**
     * \brief Constructor of database class object
     * \param[in] conf Smart pointer to configuration of project
     * \param[in] log Smart pointer to global logger
     */
    DatabaseModule::DatabaseModule(const std::shared_ptr<Config>& conf, const std::shared_ptr<Logger>& log)
    {
        config = conf;
        logger = log;
        if (!config)
            config = std::make_shared<Config>();
        if (!logger)
            logger = std::make_shared<Logger>();
        cluster_ptr.reset(cass_cluster_new());
        session_ptr.reset(cass_session_new());
        id_generator_ptr.reset(cass_uuid_gen_new());
        cass_cluster_set_contact_points(cluster_ptr.get(), config->database_host.c_str());
        cass_log_set_callback(DatabaseModule::logCallback, static_cast<void*>(logger.get()));
        logger->LogInfo("Database module created");
    }

    /**
     * \brief Method for init connection to database (must be call after construction of DatabaseModule object)
     * \return true if all init function return true
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
            logger->LogCritical("Error init matchers pool\n");
            return false;
        }
        logger->LogInfo("Database module initialized");
        return true;
    }

    /**
     * \brief Method for connect to database
     * \param[in] max_retries Count of tries of connect to databse
     * \param[in] retry_delay_ms Delay in ms between 2 tries connecting to database
     * \return true if connection to database was successful 
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

    
    /**
     * \brief Method for load local database (all descriptors and std::map for mapping descriptors and ids)
     * \return true if loading successful
     */
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

    /**
     * \brief Internal method for loading database
     * \param[in] row Cassanra row for getting data for local database
     * Load one cassandra row
     */
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

        logger->LogInfo(std::string("Load database local_descriptor_to_id_map.size ") + std::to_string(local_descriptor_to_id_map.size()));
        return true;
    }

    /**
     * \brief Internal method for searchind id of object by it's descriptor
     * \param[in] exhibit_descriptor Descriptor of object (must be ORB)
     * \return id of object if search was successful or std::nullopt in other way
     */
    [[nodiscard]] std::optional<CassUuid> DatabaseModule::findExhibitUuid(const cv::Mat& exhibit_descriptor)
    {
        PooledMatcher matcher = getMatcher();
        std::vector< std::vector<cv::DMatch> > knn_matches;
        //knn_matches.reserve(local_descriptor_to_id_map.size());
        const int k = config->count_matches_knn;

        matcher.matcher->knnMatch(exhibit_descriptor, knn_matches, k);
        returnMatcher(matcher);       

        if (knn_matches.size() == 0)
        {
            return std::nullopt;
        }
        
        //const float ratio_threshold = config->match_ratio_threshold;
        std::unordered_map<CassUuid, uint, std::hash<CassUuid>, CassUuidEqual> good_matches;

        for (const auto& match: knn_matches)
        {
            CassUuid best_match_id = local_descriptor_to_id_map[match[0].trainIdx];
            if (good_matches.find(best_match_id) == good_matches.end())
                good_matches[best_match_id] = 1;
            else
                good_matches[best_match_id]++;
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

    /**
     * \brief Method for getting object info from database by it's ORB descriptor
     * \param[in] description ORB descriptor of object
     * \return Object info if successful or std::nullopt in another way
     * In process of work call findExhibitUuid
     */
    [[nodiscard]] std::optional<DatabaseResponse> DatabaseModule::getExhibit(const cv::Mat& description)
    {
        /**
         * table struct:
         * * id CassUuid
         * * descriptor blob
         * * image blob
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
        get_exhibit_statement_ptr.reset(cass_statement_new("select image, title, description from mpg_keyspace.exhibits where id=?", 1));
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
        if (row == nullptr)
        {
            logger->LogError("DatabaseModule: nullptr cass row");
            return std::nullopt;
        }
            

        std::optional resp = getExhibitHelper(row);
        if (resp.has_value())
        {
            char id_str[37]; // 37 - size of cass uuid in string format
            cass_uuid_string(exhibit_id.value(), id_str);
            resp.value().exhibit_id = std::string(id_str);
        }

        return resp;
    }

    /**
     * \brief Internal method for get object info from cassandra row
     * \param[in] row raw pointer to cassandra row
     * \return object info if successful or std::nullopt in another way
     */
    [[nodiscard]] std::optional<DatabaseResponse> DatabaseModule::getExhibitHelper(const CassRow* row)
    {
        const cass_byte_t *image_data = nullptr;
        size_t image_size_bytes = 0;
        if (CassError err = cass_value_get_bytes(cass_row_get_column_by_name(row, "image"), &image_data, &image_size_bytes); err != CASS_OK)
        {
            logError(err, "Failed to get image from database row");
            return std::nullopt;
        };
        
        std::vector<uint8_t> image_buffer(image_data, image_data + image_size_bytes);

        const char *title_data;
        size_t title_length = 0;
        CassError err = cass_value_get_string(cass_row_get_column_by_name(row, "title"), &title_data, &title_length);
        if (err != CASS_OK)
        {
            logError(err, "Failed to get title from database row");
            return std::nullopt;
        }
        std::string exhibit_title(title_data, title_length);


        const char *decription_data;
        size_t description_length = 0;
        err = cass_value_get_string(cass_row_get_column_by_name(row, "description"), &decription_data, &description_length);
        if (err != CASS_OK)
        {
            logError(err, "Failed to get description from database row");
            return std::nullopt;
        }
        std::string exhibit_description(decription_data, description_length);
        
        DatabaseResponse response;
        response.exhibit_description = std::move(exhibit_description);
        response.exhibit_image = std::move(image_buffer);
        response.exhibit_name = std::move(exhibit_title);
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


    /**
     * \brief Method for adding new object to database (with updating local database)
     * \param[in] exhibit_data Object data
     * \return true if successful or false if was errors
     */
    [[nodiscard]] bool DatabaseModule::addExhibit(const DatabaseRequest& exhibit_data)
    {
        size_t title_size = exhibit_data.exhibit_title.size();
        size_t desc_size = exhibit_data.exhibit_description.size();

        size_t image_size = exhibit_data.exhibit_image.size();
        size_t descriptor_size = exhibit_data.exhibit_descriptor.total() * exhibit_data.exhibit_descriptor.elemSize();

        size_t total_bytes = title_size + desc_size + image_size + descriptor_size;

        double total_mb = static_cast<double>(total_bytes) / (1024.0 * 1024.0);
        logger->LogInfo("Add exhibit request size: " + std::to_string(total_mb));
        std::cout << sizeof(exhibit_data) << std::endl;

        


        StatementPtr add_exhibit_statement_ptr;
        add_exhibit_statement_ptr.reset(
            cass_statement_new("insert into mpg_keyspace.exhibits (id, image, title, description, descriptor) values (?, ?, ?, ?, ?)", 5));
        CassUuid exhibit_id;
        cass_uuid_gen_random(id_generator_ptr.get(), &exhibit_id);
        if (auto err = cass_statement_bind_uuid(add_exhibit_statement_ptr.get(), 0, exhibit_id); err != CASS_OK)
        {
            logError(err, "Bind id to add new exhibit query");
            return false;
        }
        if (auto err = cass_statement_bind_bytes(add_exhibit_statement_ptr.get(), 1, 
                                  reinterpret_cast<const cass_byte_t*>(exhibit_data.exhibit_image.data()),
                                  exhibit_data.exhibit_image.size()); err != CASS_OK)
        {
            logError(err, "Bind image data to add new exhibit query");
            return false;
        }

        if (auto err = cass_statement_bind_string(add_exhibit_statement_ptr.get(), 2, exhibit_data.exhibit_title.c_str()); err != CASS_OK)
        {
            logError(err, "Bind image title to add new exhibit query");
            return false;
        }
        if (auto err = cass_statement_bind_string(add_exhibit_statement_ptr.get(), 3, exhibit_data.exhibit_description.c_str()); err != CASS_OK)
        {
            logError(err, "Bind image description to add new exhibit query");
            return false;
        }
        if (auto err = cass_statement_bind_bytes(add_exhibit_statement_ptr.get(), 4, 
                                  reinterpret_cast<const cass_byte_t*>(exhibit_data.exhibit_descriptor.data),
                                  exhibit_data.exhibit_descriptor.total() * exhibit_data.exhibit_descriptor.elemSize()); err != CASS_OK)
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
            size_t message_length = 0;
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
        ul.unlock();

        logger->LogInfo(std::string
            ("Add exhibit database local_descriptor_to_id_map.size ") + std::to_string(local_descriptor_to_id_map.size()));

        initMatchersPool();

        return true;
    }

    /**
     * \brief Method for delete object from database (with updating local database)
     * \param[in] exhibit_id id of object for delete
     * \return true if successful, either false
     */
    bool DatabaseModule::deleteExhibit(const std::string& exhibit_id)
    {
        CassUuid id;
        cass_uuid_from_string(exhibit_id.c_str(), &id);
        CassUuidEqual id_equal;
        auto id_equal_pred = [&id, &id_equal](const CassUuid& other_id)
        {
            return id_equal(id, other_id);
        };

        if (std::find_if(local_descriptor_to_id_map.begin(), local_descriptor_to_id_map.end(), id_equal_pred) == 
                         local_descriptor_to_id_map.end())
        {
            logger->LogError("DatabaseModule: cannot delete exhibit with id " + exhibit_id + ", not found");
            return false;
        }

        StatementPtr delete_exhibit_statement_ptr;
        delete_exhibit_statement_ptr.reset(
            cass_statement_new("delete from mpg_keyspace.exhibits where id=?", 1));

        if (auto err = cass_statement_bind_uuid(delete_exhibit_statement_ptr.get(), 0, id); err != CASS_OK)
        {
            logError(err, "Bind id to delete exhibit query");
            return false;
        }

        FuturePtr query_future_ptr;
        query_future_ptr.reset(cass_session_execute(session_ptr.get(), delete_exhibit_statement_ptr.get()));

        CassError rc = cass_future_error_code(query_future_ptr.get());
        if (rc != CASS_OK)
        {
            const char *message;
            size_t message_length;
            cass_future_error_message(query_future_ptr.get(), &message, &message_length);

            logger->LogError("DatabaseModule: delete query error (" + std::string(cass_error_desc(rc)) + "): " + 
                        std::string(message, message_length));
            return false;
        }



        cv::Mat updated_descriptors;
        std::vector<CassUuid> updated_id_map;
        std::unique_lock<std::mutex> ul(local_database_mtx);
        for (size_t i = 0; i < local_descriptor_to_id_map.size(); ++i)
        {
            if (!id_equal(local_descriptor_to_id_map[i], id))
            {
                updated_descriptors.push_back(local_database_descriptor.row(i));
                updated_id_map.push_back(local_descriptor_to_id_map[i]);
            }
        }

        local_database_descriptor = std::move(updated_descriptors);
        local_descriptor_to_id_map = std::move(updated_id_map);
        ul.unlock();
        logger->LogInfo(std::string
            ("Delete exhibit database local_descriptor_to_id_map.size ") + std::to_string(local_descriptor_to_id_map.size()));
        logger->LogInfo(std::string
            ("Delete exhibit database local_descriptor.rows ") + std::to_string(local_database_descriptor.rows));

        initMatchersPool();

        return true;
    }

     /**
     * \brief Method for get data chunk from database
     * \param[in] next_chunk_token string version of next database page token (it is empty if you need first chunk)
     * \return Chunk of database if successful, either std::nullopt
     */
    std::optional<DatabaseChunk> DatabaseModule::getDatabaseChunk(const std::string& next_chunk_token)
    {
        std::string query = "select id, image, title, description from mpg_keyspace.exhibits";
        StatementPtr  get_chunk_statement(cass_statement_new(query.c_str(), 0));
        const int chunk_size = config->database_chunk_size;
        cass_statement_set_paging_size(get_chunk_statement.get(), chunk_size);

        if (!next_chunk_token.empty())
        {
            CassError rc = cass_statement_set_paging_state_token(
                get_chunk_statement.get(), next_chunk_token.data(), next_chunk_token.size());
            if (rc != CASS_OK)
            {
                logError(rc, "Failed to set paging token in get chunk method");
                return std::nullopt;
            }
        }

        FuturePtr query_future_ptr;
        query_future_ptr.reset(cass_session_execute(session_ptr.get(), get_chunk_statement.get()));

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

        DatabaseChunk chunk;
        CassIterator *it = cass_iterator_from_result(result.get());
        while (cass_iterator_next(it))
        {
            const CassRow *row = cass_iterator_get_row(it);
            auto current_exhibit = getDatabaseChunkHelper(row);
            if (current_exhibit.has_value())
            {
                chunk.exhibits.push_back(current_exhibit.value());
            }
        }
        cass_iterator_free(it);

        const char *paging_state = nullptr;
        size_t paging_state_length = 0;
        if (cass_result_has_more_pages(result.get()) &&
            cass_result_paging_state_token(result.get(), &paging_state, &paging_state_length) == CASS_OK)
        {
            chunk.next_chunk_token.assign(paging_state, paging_state + paging_state_length);
            chunk.is_last_chunk = false;
        }
        else
        {
            chunk.next_chunk_token = "";
            chunk.is_last_chunk = true;
        }

        return chunk;
    }

     /**
     * \brief Internal method for get chunk record info from cassandra row
     * \param[in] row Raw pinter to cassandra row with record info
     * \return Object info if successful or std::nullopt
     */
    std::optional<DatabaseResponse> DatabaseModule::getDatabaseChunkHelper(const CassRow* row)
    {
        CassUuid id;
        if (CassError err = cass_value_get_uuid(cass_row_get_column_by_name(row, "id"), &id); err != CASS_OK)
        {
            logError(err, "Failed to get id from database row for chunk");
            return std::nullopt;
        };
        char id_str[37]; // 37 - cass uuid standart sise in string form
        cass_uuid_string(id, id_str);
        std::string exhibit_id(id_str);

        const cass_byte_t *image_data = nullptr;
        size_t image_size_bytes = 0;
        if (CassError err = cass_value_get_bytes(cass_row_get_column_by_name(row, "image"), &image_data, &image_size_bytes); err != CASS_OK)
        {
            logError(err, "Failed to get image from database row for chunk");
            return std::nullopt;
        };
        
        std::vector<uint8_t> image_buffer(image_data, image_data + image_size_bytes);

        const char *title_data;
        size_t title_length;
        CassError err = cass_value_get_string(cass_row_get_column_by_name(row, "title"), &title_data, &title_length);
        if (err != CASS_OK)
        {
            logError(err, "Failed to get title from database row for chunk");
            return std::nullopt;
        }
        std::string exhibit_title(title_data, title_length);


        const char *decription_data;
        size_t description_length;
        err = cass_value_get_string(cass_row_get_column_by_name(row, "description"), &decription_data, &description_length);
        if (err != CASS_OK)
        {
            logError(err, "Failed to get description from database row for chunk");
            return std::nullopt;
        }
        std::string exhibit_description(decription_data, description_length);
        
        DatabaseResponse response;
        response.exhibit_id = std::move(exhibit_id);
        response.exhibit_description = std::move(exhibit_description);
        response.exhibit_image = std::move(image_buffer);
        response.exhibit_name = std::move(exhibit_title);
        return response;
    }


     /**
     * \brief Internal method for create and train cv matchers for searching objects in local database
     * \return true if successful, either false
     */
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

     /**
     * \brief Internal method for get matcher from pool
     * \return Matcher object (wait while it will be available)
     * 
     * Don't forget to return mathcer to pool!
     */
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

    /**
     * \brief Internal method for return matcher to pool
     */
    void DatabaseModule::returnMatcher(PooledMatcher matcher)
    {
        const auto &origin_pool = matcher.origin_pool_ptr;

        {
            std::lock_guard<std::mutex> lg(origin_pool->mtx);
            origin_pool->pool.push(matcher.matcher);
        }

        origin_pool->cv.notify_one();
    }

    void DatabaseModule::logCallback(const CassLogMessage* message, void* data)
    {
        Logger *logger_cb = static_cast<Logger *>(data);
        std::string log_message = std::string("Database module: ") +
           message->function + "): " +
           message->message + "\n";
        switch (message->severity)
        {
        case CASS_LOG_ERROR :
        {
            logger_cb->LogError(log_message);
            break;
        }
        case CASS_LOG_INFO :
        {
            logger_cb->LogInfo(log_message);
            break;
        }
        case CASS_LOG_WARN :
        {
            logger_cb->LogWarning(log_message);
            break;
        }
        case CASS_LOG_CRITICAL :
        {
            logger_cb->LogCritical(log_message);
            break;
        }
        default:
            break;
        } 

    }
}