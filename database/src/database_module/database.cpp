#include "database_module/database.hpp"
#include <iostream>
#include <thread>
#include <cassert>


namespace MPGDatabase
{

    /**
     * \brief Constructor of DatabaseModule class
     * \todo Add config and logger 
     */
    DatabaseModule::DatabaseModule()
    {
        cluster_ptr.reset(cass_cluster_new());
        session_ptr.reset(cass_session_new());
        cass_cluster_set_contact_points(cluster_ptr.get(), "localhost");
        
        if (!ConnectToDatabase(20, 10000))
        {
            std::cerr << "Cannot connect to database\n";
            std::abort();
        }

        loadDatabase();
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
            std::cerr << "Connection failed (attempt " << i + 1 << "/" << max_retries << "). Retrying in " << retry_delay_ms << " ms...\n";
            const char *message;
            size_t message_length;
            cass_future_error_message(connect_future_ptr.get(), &message, &message_length);
            std::cerr << "Connection error: " << std::string(message, message_length) << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_delay_ms));
        }
        return false;
    }

    DatabaseModule::~DatabaseModule()
    {
        std::cout << "Finish work of database module\n";
    }

    
    void DatabaseModule::loadDatabase()
    {
        local_database.clear();

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

            std::cerr << "Query error (" << cass_error_desc(rc) << "): "
                      << std::string(message, message_length) << std::endl;
            std::abort();
        }

        QueryResultPtr result(cass_future_get_result(query_future_ptr.get()));
        
        IteratorPtr iter;
        iter.reset(cass_iterator_from_result(result.get()));

        while(cass_iterator_next(iter.get()))
        {
            const CassRow* row = cass_iterator_get_row(iter.get());

            CassUuid id;
            cass_value_get_uuid(cass_row_get_column_by_name(row, "id"), &id);

            const cass_byte_t *descriptor_data = nullptr;
            size_t descriptor_size = 0;
            cass_value_get_bytes(cass_row_get_column_by_name(row, "descriptor"), &descriptor_data, &descriptor_size);
            if (descriptor_size != 100 * 32) {
                std::cerr << "Descriptor size mismatch: expected 3200 (32 x 100), got " << descriptor_size << std::endl;
                std::abort();
            }
            std::vector<uint8_t> descriptor_buffer(descriptor_data, descriptor_data + descriptor_size);
            cv::Mat descriptor_temp(100, 32, CV_8UC1, descriptor_buffer.data());
            cv::Mat descriptor = descriptor_temp.clone();
            local_database.insert({id, std::move(descriptor)});
        }


    }

    [[nodiscard]] std::optional<CassUuid> DatabaseModule::findExhibitUuid(const cv::Mat& description)
    {
        std::optional<CassUuid> id = std::nullopt;


        return id;
    }

    [[nodiscard]] std::optional<DatabaseResponse> DatabaseModule::getExhibit(const CassUuid& exhibit_id)
    {
        std::optional<DatabaseResponse> response = std::nullopt;



        return response;
    }

}