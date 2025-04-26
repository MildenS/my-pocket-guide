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
        cass_cluster_set_contact_points(cluster_ptr.get(), "172.19.0.2");
        
        if (!ConnectToDatabase(20, 10000))
        {
            std::cerr << "Cannot connect to database\n";
            std::abort();
        }
    }

    /**
     * \brief Method for connect to database
     * \todo add config and logger
     * \param[in] max_retries Count of tries of connect to databse
     * \param[in] retry_delay_ms Delay in ms between 2 tries connecting to database 
     */
    bool DatabaseModule::ConnectToDatabase(size_t max_retries, size_t retry_delay_ms)
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

    }

    CassUuid DatabaseModule::findExhibitUuid(const cv::Mat& description)
    {

    }

    DatabaseResponse DatabaseModule::getExhibit(const CassUuid& exhibit_id)
    {

    }

}