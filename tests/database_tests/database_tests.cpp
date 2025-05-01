#include <database_module/database.hpp>
#include <gtest/gtest.h>

#include <fstream>
#include <filesystem>

using namespace MPGDatabase;

class DatabaseTestModule: public DatabaseModule
{
public:

    using ::DatabaseModule::addExhibit;
    using ::DatabaseModule::getExhibit;
    using ::DatabaseModule::findExhibitUuid;

};

std::vector<DatabaseRequest> requests;

void makeTestData(const std::string& path);

// Demonstrate some basic assertions.
TEST(MPGDataBaseTest, Init) {
    DatabaseTestModule db;
    bool is_init = db.init();
    ASSERT_EQ(is_init, true);
}




int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Path to test data is required\n";
        std::abort();
    }
    makeTestData(std::string(argv[1]));
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


void makeTestData(const std::string& path)
{
    std::filesystem::path data_path(path);
    if (!std::filesystem::exists(data_path))
    {
        std::cerr << "Invalid path to test data!\n";
        std::abort();
    }

    

}