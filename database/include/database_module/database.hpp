#pragma once

#include <database_module/database_utils.hpp>
#include <cassandra.h>
#include <memory>


/**
* \brief Namespace for MPG database classes and functions
*/
namespace MPGDatabase
{

class DatabaseModule
{

public:

    DatabaseModule();

    ~DatabaseModule();


private:

    std::unique_ptr <CassCluster, CassClusterDeleter> cluster_ptr;
    std::unique_ptr <CassSession, CassSessionDeleter> session_ptr;
    std::unique_ptr <CassFuture, CassFutureDeleter> connect_future_ptr;
};

}