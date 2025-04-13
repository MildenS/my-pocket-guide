#include <iostream>
#include <map>
#include <string>
#include <langinfo.h>
#include <clocale>
#include <database_module/database.hpp>

struct simple_class
{
  std::map<std::string, std::string> objects;
};

struct my_class
{
  my_class() {sim = new simple_class;}
  std::map<std::string, std::string>& get_objects() const
  {
    return sim->objects;
  }
  simple_class* sim;
};

int main()
{
  my_class a;
  auto objects = a.get_objects();
  for (const auto& [key, value]: objects)
  {
    std::cout << key << value << std::endl;
  }
  objects.emplace(std::make_pair("bbb", "bbb"));
  for (const auto& [key, value]: objects)
  {
    std::cout << key << value << std::endl;
  }

  std::string str ("я русская строка");

  std::cout << str << std::endl;
  std::cout << "я русский массив чаров" << std::endl;
  std::cin >> str;
  std::cout << str << std::endl;
  std::getchar();
  char* encoding = nl_langinfo(CODESET);
        printf("Encoding is %s\n", encoding);
        setlocale(LC_ALL, "");
        char *loc = setlocale(LC_ALL, NULL);
        printf("Encoding is %s\n", loc);
        
        
}




// #include <opencv2/opencv.hpp>
// #include <iostream>
// #include <string>

// using namespace cv;
// using namespace std;

// int main() {
//   // Загрузка изображения
//   Mat img = imread("/home/sklochkov/Downloads/input.jpg"); // Замените "input.jpg" на путь к вашему изображению

//   cout << "Image size: " << img.size() << endl;
//   cout << "Image type: " << img.type() << endl;
//   cout << "Image channesl count: " << img.channels() << endl;
//   cout << "Image element size: " << img.elemSize() << endl;
//   cout << "Image total: " << img.total() << endl;

//   if (img.empty()) {
//     cerr << "Could not open or find the image!" << endl;
//     return -1;
//   }

//   // Инициализация детектора и дескриптора ORB
//   Ptr<ORB> orb = ORB::create(20, 1.2f, 8, 31, 0, 2, ORB::HARRIS_SCORE, 31, 20); // Можно настроить параметры

//   // Обнаружение ключевых точек
//   vector<KeyPoint> keypoints;
//   orb->detect(img, keypoints);

//   // Вычисление дескрипторов
//   Mat descriptors;
//   orb->compute(img, keypoints, descriptors);

//   // Вывод информации о ключевых точках и дескрипторах
//   cout << "Number of keypoints: " << keypoints.size() << endl;
//   cout << "Descriptor size: " << descriptors.size() << endl;
//   cout << "Descriptor type: " << descriptors.type() << endl;
//   cout << "Descriptor срфттуды: " << descriptors.channels() << endl;

//   //  Вывод дескрипторов (для демонстрации,  в реальных приложениях обычно используется иначе)
//   // cout << "Descriptors:" << endl;
//   // for (int i = 0; i < descriptors.rows; ++i) {
//   //   for (int j = 0; j < descriptors.cols; ++j) {
//   //     cout << (int)descriptors.at<uchar>(i, j) << " ";
//   //   }
//   //   cout << endl;
//   // }

//   int capacity = descriptors.rows * descriptors.cols;
//   std::string descriptor_str;
//   descriptor_str.resize(capacity);
//   for (int i = 0; i < descriptors.rows; ++i) {
//     for (int j = 0; j < descriptors.cols; ++j) {
//       descriptor_str[i * descriptors.cols  + j] = descriptors.at<uchar>(i, j);
//     }
//   }
//   //std::cout << descriptor_str << std::endl;

//   std::string remade_descriptor_str(descriptor_str.c_str());

//   cv::Mat remade_desriptor(cv::Size(descriptors.cols, descriptors.rows), 0);
//   for (int i = 0; i < descriptors.rows; ++i) {
//     for (int j = 0; j < descriptors.cols; ++j) {
//       remade_desriptor.at<uchar>(i, j) = remade_descriptor_str[i * descriptors.cols  + j];
//     }
//   }

//   std::cout << "Success cv:Mat serialize " << (cv::countNonZero(remade_desriptor - descriptors) == 0) << std::endl;
//   //  Можно сохранить ключевые точки и дескрипторы для дальнейшего использования
//   //  FileStorage fs("keypoints.yml", FileStorage::WRITE);
//   //  fs << "keypoints" << keypoints;
//   //  fs << "descriptors" << descriptors;
//   //  fs.release();


//   // Отображение изображения с ключевыми точками (опционально)
//   Mat img_keypoints;
//   drawKeypoints(img, keypoints, img_keypoints, Scalar::all(-1), DrawMatchesFlags::DEFAULT);
//   imshow("Keypoints", img_keypoints);
//   waitKey(0);

//   return 0;
// }








// #include <cassandra.h>
// /* Use "#include <dse.h>" when connecting to DataStax Enterpise */
// #include <stdio.h>

// int main() {
//   /* Setup and connect to cluster */
//   CassFuture* connect_future = NULL;
//   CassCluster* cluster = cass_cluster_new();
//   CassSession* session = cass_session_new();

//   /* Add contact points */
//   cass_cluster_set_contact_points(cluster, "127.0.0.1");

//   /* Provide the cluster object as configuration to connect the session */
//   connect_future = cass_session_connect(session, cluster);

//   /* This operation will block until the result is ready */
//   CassError rc = cass_future_error_code(connect_future);

//   if (rc != CASS_OK) {
//     /* Display connection error message */
//     const char* message;
//     size_t message_length;
//     cass_future_error_message(connect_future, &message, &message_length);
//     fprintf(stderr, "Connect error: '%.*s'\n", (int)message_length, message);
//   }

//   /* Run queries... */

//   cass_future_free(connect_future);
//   cass_session_free(session);
//   cass_cluster_free(cluster);

//   return 0;
// }