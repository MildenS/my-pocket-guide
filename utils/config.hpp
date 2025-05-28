#pragma once

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

namespace MPG
{

    /**
     * \brief Class for MPG config. Consist of database, core and networks parameters.
     */
    struct Config
    {
        inline Config();
        inline Config(const std::string& path);

        //database params
        size_t matchers_pool_size;
        size_t max_connect_retries;
        size_t connect_retry_delay_ms;
        size_t max_matches_count;
        float match_ratio_threshold;
        std::string database_host;
        size_t count_matches_knn;
        size_t database_chunk_size;

        //core params

        size_t orb_pool_size;
        size_t orb_kps_count;
        size_t max_descriptor_size;

        //server params

        size_t server_port;
    };

    /**
     * \brief Default constructor for config with default values of parameters.
     */
    inline Config::Config()
    {
        matchers_pool_size = 10;
        max_connect_retries = 20;
        connect_retry_delay_ms = 10000;
        max_matches_count = 100;
        match_ratio_threshold = 0.75f;
        database_host = "localhost";
        count_matches_knn = 2;
        database_chunk_size = 10;

        orb_pool_size = 10;
        orb_kps_count = 100;
        max_descriptor_size = 100;

        server_port = 8888;
    }
    
    /**
     * \brief Constructor for config from file.
     * \param[in] path Path to config file (.json type) 
     */
    inline Config::Config(const std::string& path)
    {
        std::ifstream fin(path);
        if (!fin.is_open())
        {
            std::cerr << "Invalid config path\n";
            std::abort();
        }
        nlohmann::json config_json;
        fin >> config_json;
        fin.close();

        matchers_pool_size = config_json["matchers_pool_size"];
        max_connect_retries = config_json["max_connect_retries"];
        connect_retry_delay_ms = config_json["connect_retry_delay_ms"];
        max_matches_count = config_json["max_matches_count"];
        match_ratio_threshold = config_json["match_ratio_threshold"];
        database_host = config_json["database_host"];
        count_matches_knn = config_json["count_matches_knn"];
        database_chunk_size = config_json["database_chunk_size"];

        orb_pool_size = config_json["orb_pool_size"];
        orb_kps_count = config_json["orb_kps_count"];
        max_descriptor_size = config_json["max_descriptor_size"];

        server_port = config_json["server_port"];
    }

}