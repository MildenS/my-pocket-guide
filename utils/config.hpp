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

        //core params


        //server params

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
    }

}