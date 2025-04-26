#pragma once

#include <cassandra.h>
#include <string>
#include <memory>
#include <opencv2/core.hpp>

namespace MPGDatabase
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

    struct DatabaseResponse
    {
        std::string exhibit_name;
        std::string exhibit_description;
        cv::Mat exhibit_image;
        size_t percentage_of_confidance;
    };

}

namespace std {
    template<>
    struct hash<CassUuid> {
        std::size_t operator()(const CassUuid& uuid) const noexcept {
            std::size_t h1 = std::hash<cass_uint64_t>{}(uuid.time_and_version);
            std::size_t h2 = std::hash<cass_uint64_t>{}(uuid.clock_seq_and_node);
    
            // Хорошее комбинирование двух хешей
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };
}