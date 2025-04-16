#include "database_module/database.hpp"
#include <iostream>


namespace MPGDatabase
{

    DatabaseModule::DatabaseModule()
    {
        cluster_ptr.reset(cass_cluster_new());
        session_ptr.reset(cass_session_new());
        cass_cluster_set_contact_points(cluster_ptr.get(), "172.19.0.2");
        connect_future_ptr.reset(cass_session_connect(session_ptr.get(), cluster_ptr.get()));

        /* This operation will block until the result is ready */
        CassError rc = cass_future_error_code(connect_future_ptr.get());

        if (rc != CASS_OK)
        {
            /* Display connection error message */
            const char *message;
            size_t message_length;
            cass_future_error_message(connect_future_ptr.get(), &message, &message_length);
            fprintf(stderr, "Connect error: '%.*s'\n", (int)message_length, message);
        }

        CassStatement *statement = cass_statement_new("SELECT * FROM system.local", 0);
        CassFuture* future = cass_session_execute(session_ptr.get(), statement);
        rc = cass_future_error_code(future);

        if (rc != CASS_OK)
        {
            const char *message;
            size_t message_length;
            cass_future_error_message(future, &message, &message_length);
            std::cerr << "Query error: " << std::string(message, message_length) << std::endl;
            cass_future_free(future);
            cass_statement_free(statement);
        }
        cass_future_free(future);
        cass_statement_free(statement);
    }

    DatabaseModule::~DatabaseModule()
    {
        std::cout << "Finish work of database module\n";
    }

}