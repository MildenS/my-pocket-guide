#pragma once

#include <cassandra.h>
#include <string>
#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>

namespace MPG
{
    struct CassClusterDeleter
    {
        void operator()(CassCluster *ptr) const
        {
            cass_cluster_free(ptr);
        }
    };

    struct CassSessionDeleter
    {
        void operator()(CassSession *ptr) const
        {
            cass_session_free(ptr);
        }
    };

    struct CassFutureDeleter
    {
        void operator()(CassFuture *ptr) const
        {
            cass_future_free(ptr);
        }
    };

    struct CassStatementDeleter
    {
        void operator()(CassStatement *ptr) const
        {
            cass_statement_free(ptr);
        }
    };

    struct CassIteratorDeleter
    {
        void operator()(CassIterator *ptr) const
        {
            cass_iterator_free(ptr);
        }
    };

    struct CassUuidGenDeleter
    {
        void operator()(CassUuidGen *ptr) const
        {
            cass_uuid_gen_free(ptr);
        }
    };

    class QueryResultHandler
    {
    public:
        explicit QueryResultHandler(const CassResult *result) : result_(result) {}
        ~QueryResultHandler()
        {
            if (result_)
            {
                cass_result_free(result_);
            }
        }
        const CassResult *get() const { return result_; }

    private:
        const CassResult *result_;
    };

    struct CassUuidEqual {
        bool operator()(const CassUuid& lhs, const CassUuid& rhs) const {
            return lhs.time_and_version == rhs.time_and_version && lhs.clock_seq_and_node == rhs.clock_seq_and_node;
        }
    };

    struct DatabaseResponse
    {
        std::string exhibit_id;
        std::string exhibit_name;
        std::string exhibit_description;
        std::vector<uint8_t> exhibit_image;
        size_t percentage_of_confidance;
    };

    struct DatabaseRequest
    {
        std::string exhibit_title;
        std::string exhibit_description;
        std::vector<uint8_t> exhibit_image;
        cv::Mat exhibit_descriptor;
    };

    struct DatabaseChunk
    {
        std::vector<DatabaseResponse> exhibits;
        std::string next_chunk_token;
        bool is_last_chunk;
    };

    struct MatcherPool
    {
        std::mutex mtx;
        std::condition_variable cv;
        std::queue<cv::Ptr<cv::DescriptorMatcher>> pool;
    };

    struct PooledMatcher
    {
        cv::Ptr<cv::DescriptorMatcher> matcher;
        std::shared_ptr<MatcherPool> origin_pool_ptr;
    };

}

namespace std {
    template<>
    struct hash<CassUuid> {
        std::size_t operator()(const CassUuid& uuid) const noexcept {
            std::size_t h1 = std::hash<cass_uint64_t>{}(uuid.time_and_version);
            std::size_t h2 = std::hash<cass_uint64_t>{}(uuid.clock_seq_and_node);
    
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };
}