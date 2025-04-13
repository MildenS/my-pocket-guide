#pragma once

#include <cassandra.h>

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
}