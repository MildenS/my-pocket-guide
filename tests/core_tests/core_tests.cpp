#include <core_module/core.hpp>
#include <config.hpp>
#include <logger.hpp>

#include <gtest/gtest.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

#include <fstream>
#include <filesystem>
#include <algorithm>
#include <bits/stl_numeric.h>

using namespace MPG;

class CoreTestModule: public Core
{
public:

    using ::Core::addExhibit;
    using ::Core::getExhibit;

    CoreTestModule(const std::shared_ptr<Config>& conf, const std::shared_ptr<Logger>& log)
        : Core(conf, log) {}

};

CoreRequest request;
std::vector<std::vector<uint8_t>> exhibit_descr;
std::shared_ptr<Config> config;
std::shared_ptr<Logger> logger;



void makeTestData(const std::string& path);

TEST(MPGDataBaseTest, Init) {
    CoreTestModule core(config, logger);
}

TEST(MPGDataBaseTest, AddExhibit) {
    CoreTestModule core(config, logger);
    bool is_add = core.addExhibit(request);
    ASSERT_EQ(is_add, true);
}


TEST(MPGDataBaseTest, GetExhibit) {
    CoreTestModule core(config, logger);

    auto response = core.getExhibit(std::move(exhibit_descr[0]));
    ASSERT_NE(response, std::nullopt);
    ASSERT_EQ(response.value().exhibit_name, "test");
    std::cout << exhibit_descr[0].size() << " " << response.value().exhibit_image.size() << std::endl;
    cv::Mat img = cv::imdecode(response.value().exhibit_image, cv::IMREAD_COLOR);
    cv::imshow("deserialized_image", img);
    cv::waitKey(0);
}



int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Path to test data is required\n";
        std::abort();
    }
    makeTestData(std::string(argv[1]));
    config = std::make_shared<Config>();
    logger = std::make_shared<Logger>();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


void makeTestData(const std::string& path)
{
    std::filesystem::path data_path(path);
    if (!std::filesystem::exists(data_path) || !std::filesystem::is_directory(path))
    {
        std::cerr << "Invalid path to test data!\n";
        std::abort();
    }

    std::vector<std::vector<uint8_t>> exhibit_images;
    for (const auto image_path: std::filesystem::directory_iterator(path))
    {
        cv::Mat image = cv::imread(image_path.path());
        cv::Mat resized_image;
        cv::resize(image, resized_image, cv::Size(image.cols / 4, image.rows / 8));
        std::vector<uint8_t> buffer;
        cv::imencode(".jpg", resized_image, buffer);
        exhibit_images.push_back(buffer);
    }

    request.exhibit_main_image = exhibit_images[0];
    request.exhibit_title = "test";
    request.exhibit_description = "test exhibit";
    request.exhibit_descriptor_images = {exhibit_images[0], exhibit_images[1], exhibit_images[2]};
    exhibit_descr = {exhibit_images[3], exhibit_images[4], exhibit_images[5]};
}