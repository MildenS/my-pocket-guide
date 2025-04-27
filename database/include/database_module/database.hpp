#pragma once

#include <database_module/database_utils.hpp>
#include <cassandra.h>
#include <memory>
#include <optional>
#include <unordered_map>
#include <optional>


/**
* \brief Namespace for MPG database classes and functions
*/
namespace MPGDatabase
{

class DatabaseModule
{
protected:

    using ClusterPtr = std::unique_ptr <CassCluster, CassClusterDeleter>;
    using SessionPtr = std::unique_ptr <CassSession, CassSessionDeleter>;
    using FuturePtr = std::unique_ptr <CassFuture, CassFutureDeleter>;
    using StatementPtr = std::unique_ptr <CassStatement, CassStatementDeleter>;
    using IteratorPtr = std::unique_ptr <CassIterator, CassIteratorDeleter>;

    using QueryResultPtr = QueryResultHandler; 

public:

    DatabaseModule();

    ~DatabaseModule();


protected:

    bool ConnectToDatabase(size_t max_retries = 10, size_t retry_delay_ms = 5000);
    void loadDatabase();

    std::optional<CassUuid> findExhibitUuid(const cv::Mat& description);
    std::optional<DatabaseResponse> getExhibit(const CassUuid& exhibit_id);

    ClusterPtr cluster_ptr;
    SessionPtr session_ptr;

    std::unordered_map<CassUuid, cv::Mat, std::hash<CassUuid>, CassUuidEqual> local_database;
};

}