#pragma once

#include <opencv2/opencv.hpp>

#include <database_module/database_utils.hpp>
#include <vector>


namespace MPG
{

struct CoreResponse
{
    std::string exhibit_name;
    std::string exhibit_description;
    std::vector<uint8_t> exhibit_image;
    size_t percentage_of_confidance;
};

struct CoreRequest
{

    std::string exhibit_title;
    std::string exhibit_description;
    std::vector<uint8_t> exhibit_main_image;
    std::vector<std::vector<uint8_t>> exhibit_descriptor_images;
};

struct ORBPool
{
    std::mutex mtx;
    std::condition_variable cv;
    std::queue<cv::Ptr<cv::ORB>> pool;
};



}