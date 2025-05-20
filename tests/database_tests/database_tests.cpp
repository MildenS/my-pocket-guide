#include <database_module/database.hpp>
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

class DatabaseTestModule: public DatabaseModule
{
public:

    using ::DatabaseModule::addExhibit;
    using ::DatabaseModule::getExhibit;
    using ::DatabaseModule::findExhibitUuid;

    DatabaseTestModule(const std::shared_ptr<Config>& conf, const std::shared_ptr<Logger>& log)
        : DatabaseModule(conf, log) {}

};

DatabaseRequest request;
std::vector<cv::Mat> exhibit_descr;
std::shared_ptr<Config> config;
std::shared_ptr<Logger> logger;



void makeTestData(const std::string& path);

TEST(MPGDataBaseTest, Init) {
    DatabaseTestModule db(config, logger);
    bool is_init = db.init();
    ASSERT_EQ(is_init, true);
}

TEST(MPGDataBaseTest, AddExhibit) {
    DatabaseTestModule db(config, logger);
    bool is_init = db.init();
    ASSERT_EQ(is_init, true);
    bool is_add = db.addExhibit(request);
    ASSERT_EQ(is_add, true);
}

TEST(MPGDataBaseTest, GetExhibit) {
    DatabaseTestModule db(config, logger);
    bool is_init = db.init();
    ASSERT_EQ(is_init, true);
    //bool is_add = db.addExhibit(request);
    //ASSERT_EQ(is_add, true);

    auto response = db.getExhibit(exhibit_descr[0]);
    ASSERT_NE(response, std::nullopt);
    ASSERT_EQ(response.value().exhibit_name, "test");
    cv::Mat deser_image = cv::imdecode(response.value().exhibit_image, cv::IMREAD_COLOR);
    cv::imshow("deserialized_image", deser_image);
    cv::waitKey(0);
}



int main(int argc, char** argv)
{
    // if (argc < 2)
    // {
    //     std::cerr << "Path to test data is required\n";
    //     std::abort();
    // }
    // makeTestData(std::string(argv[1]));
    config = std::make_shared<Config>();
    logger = std::make_shared<Logger>();
    makeTestData("../../../data/test_data/1");
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

    std::vector<cv::Mat> exhibit_images;
    cv::Ptr<cv::ORB> orb = cv::ORB::create(100);
    cv::Mat all_descriptor;
    std::vector<cv::KeyPoint> all_kps;
    size_t counter = 0;
    for (const auto image_path: std::filesystem::directory_iterator(path))
    {
        cv::Mat image = cv::imread(image_path.path());
        cv::Mat resized_image;
        cv::resize(image, resized_image, cv::Size(image.cols / 4, image.rows / 8));
        exhibit_images.push_back(resized_image);

        cv::Mat curr_descriptor;
        std::vector<cv::KeyPoint> kps;
        orb->detectAndCompute(resized_image, cv::noArray(), kps, curr_descriptor);

        if (counter < 3)
        {
            all_descriptor.push_back(curr_descriptor);
            all_kps.insert(all_kps.end(), kps.begin(), kps.end());
        }
        else
        {
            exhibit_descr.push_back(curr_descriptor);
        }

        ++counter;
    }

    //request.exhibit_image = exhibit_images[0];
    cv::imencode(".jpg", exhibit_images[0], request.exhibit_image);
    request.exhibit_title = "test";
    request.exhibit_description = "test exhibit";

    std::vector<int> indices(all_kps.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::sort(indices.begin(), indices.end(), [&](int a, int b) {
        return all_kps[a].response > all_kps[b].response;
    });

    int top_n = std::min(100, static_cast<int>(indices.size()));
    cv::Mat final_descriptors;

    for (int i = 0; i < top_n; ++i) {
        final_descriptors.push_back(all_descriptor.row(indices[i]));
    }

    request.exhibit_descriptor = final_descriptors;

}