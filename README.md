# My Pocket Guide

A brief overview of what this project does, its purpose, and what problems it solves.

## Dependencies

List of required tools, libraries, or frameworks to build and run the project:

- C++17 or higher  
- CMake 3.16+  
- OpenCV 4.2
- Libuv
- nlohmann json 
- SSL

To install dependencies on Debian/Ubuntu (you can skip it, if you use Docker):

```bash
sudo apt update
sudo apt install libuv1-dev libopencv-dev nlohmann-json3-dev libssl-dev
```
## Build

### With CMake

For building with CMake you should clone this repository to yout PC and build it:

```bash
git clone https://github.com/MildenS/my-pocket-guide.git
cd my-pocket-guide
mkdir build 
cd build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE
make -j8
```

### With Docker

For build and run with Docker you should clone this repository and use docker compose:

```bash
git clone https://github.com/MildenS/my-pocket-guide.git
cd my-pocket-guide
docker compose up
```
It will run cassandra, server and swagger-ui

## Documentation

In this project for code documentation I used Doxygen. For generate docs in html and latex format you should:

```bash
cd docs
doxygen
```

If you want use API documentation you make run it locally (with swagger-ui/swagger.yaml) or with docker compose:

```bash
docker compose up swagger-ui
```

