#pragma once

#include <database_module/database_utils.hpp>
#include <cassandra.h>
#include <memory>
#include <optional>
#include <unordered_map>
#include <optional>
#include <queue>
#include <opencv2/features2d/features2d.hpp>
#include <mutex>
#include <condition_variable>


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

    using MatcherPtr = cv::Ptr<cv::DescriptorMatcher>;

public:

    DatabaseModule();

    ~DatabaseModule();

    bool init();

    std::optional<CassUuid> findExhibitUuid(const cv::Mat& description);
    std::optional<DatabaseResponse> getExhibit(const CassUuid& exhibit_id);
    bool addExhibit(const DatabaseRequest& exhibit_data);


protected:

    bool ConnectToDatabase(size_t max_retries = 10, size_t retry_delay_ms = 5000);
    bool loadDatabase();


    ClusterPtr cluster_ptr;
    SessionPtr session_ptr;

    //std::unordered_map<CassUuid, cv::Mat, std::hash<CassUuid>, CassUuidEqual> local_database;

    cv::Mat local_database_descriptor;
    std::vector<CassUuid> local_descriptor_to_id_map;

private:

    bool loadDatabaseHelper(const CassRow* row);

    void logGetExhibitError(CassError err, const std::string& context);
    std::optional<DatabaseResponse> getExhibitHelper(const CassRow* row);

    bool initMatchersPool();
    MatcherPtr getMatcher();
    void returnMatcher(MatcherPtr matcher);

    std::queue<MatcherPtr> matchers_pool;
    std::mutex matchers_pool_mtx;
    std::condition_variable matchers_pool_cv;

};

}