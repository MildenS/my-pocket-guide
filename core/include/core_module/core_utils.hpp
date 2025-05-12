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

inline std::optional<CoreResponse> getResponse(const DatabaseResponse& db_resp)
{
    std::optional<CoreResponse> resp;

    resp.value().exhibit_description = db_resp.exhibit_description;
    resp.value().exhibit_name = db_resp.exhibit_name;
    bool success = cv::imencode(".jpg", db_resp.exhibit_image, resp.value().exhibit_image);
    if (!success)
        std::cerr << "Core: invalid image of exhibit with name " << db_resp.exhibit_name << std::endl;
    return resp;
}

struct ORBPool
{
    std::mutex mtx;
    std::condition_variable cv;
    std::queue<cv::Ptr<cv::ORB>> pool;
};



}