#pragma once

#include <database_module/database_utils.hpp>
#include <config.hpp>
#include <logger.hpp>
#include <cassandra.h>
#include <optional>

/**
* \brief Namespace for MPG database classes and functions
*/
namespace MPG
{

class DatabaseModule
{
protected:

    using ClusterPtr = std::unique_ptr <CassCluster, CassClusterDeleter>;
    using SessionPtr = std::unique_ptr <CassSession, CassSessionDeleter>;
    using FuturePtr = std::unique_ptr <CassFuture, CassFutureDeleter>;
    using StatementPtr = std::unique_ptr <CassStatement, CassStatementDeleter>;
    using IteratorPtr = std::unique_ptr <CassIterator, CassIteratorDeleter>;
    using IdGeneratorPtr = std::unique_ptr <CassUuidGen, CassUuidGenDeleter>;

    using QueryResultPtr = QueryResultHandler; 

    using MatcherPtr = cv::Ptr<cv::DescriptorMatcher>;


public:

    DatabaseModule();
    DatabaseModule(const std::shared_ptr<Config>& conf, const std::shared_ptr<Logger>& log);

    virtual ~DatabaseModule();

    virtual bool init();

    virtual std::optional<DatabaseResponse> getExhibit(const cv::Mat& description);
    virtual bool addExhibit(const DatabaseRequest& exhibit_data);
    virtual bool deleteExhibit(const std::string& exhibit_idid);


protected:

    virtual bool ConnectToDatabase(size_t max_retries = 10, size_t retry_delay_ms = 5000);
    virtual bool loadDatabase();

    virtual std::optional<CassUuid> findExhibitUuid(const cv::Mat& description);


    ClusterPtr cluster_ptr;
    SessionPtr session_ptr;
    IdGeneratorPtr id_generator_ptr;


    cv::Mat local_database_descriptor;
    std::vector<CassUuid> local_descriptor_to_id_map;

    std::shared_ptr<Config> config;
    std::shared_ptr<Logger> logger;

private:

    bool loadDatabaseHelper(const CassRow* row);

    static void logCallback(const CassLogMessage* message, void* data);

    void logError(CassError err, const std::string& context);
    std::optional<DatabaseResponse> getExhibitHelper(const CassRow* row);

    bool initMatchersPool();
    PooledMatcher getMatcher();
    void returnMatcher(PooledMatcher matcher);

    std::shared_ptr<MatcherPool> matchers_pool;
    std::mutex matchers_pool_switch_mtx;

    std::mutex local_database_mtx;

};

}