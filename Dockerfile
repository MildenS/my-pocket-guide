FROM ubuntu:20.04 AS build

ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC



RUN apt-get update && \
    apt-get install -y build-essential g++ zlib1g-dev libssl-dev cmake git libuv1-dev libopencv-dev nlohmann-json3-dev


COPY . /app

WORKDIR /app/build

RUN git clone --recursive https://github.com/wfrest/wfrest && \
  cd wfrest/workflow && \
  make && \
  make install && \
  cd .. && \
  make && \
  make install

WORKDIR /app/build

  RUN  cmake .. && \
  make


RUN ls -l /app
RUN ls -l /app/build
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib



CMD ["/app/build/server/MyPocketGuideServer", "/app/data/configs/base_config.json"]
