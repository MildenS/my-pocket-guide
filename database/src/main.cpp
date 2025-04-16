#include <iostream>
#include <map>
#include <string>
#include <langinfo.h>
#include <clocale>
#include <database_module/database.hpp>

int main()
{
  MPGDatabase::DatabaseModule db;       
        
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
