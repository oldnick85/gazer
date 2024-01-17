# syntax=docker/dockerfile:1
ARG UBUNTU_VERSION=23.10
FROM ubuntu:${UBUNTU_VERSION} AS builder_gazer
ARG GAZER_COMPILER=gcc
RUN DEBIAN_FRONTEND=noninteractive\
    apt-get update &&\
    apt-get -y upgrade
RUN DEBIAN_FRONTEND=noninteractive\
    apt-get install -y $I2PD_COMPILER
RUN DEBIAN_FRONTEND=noninteractive\ 
    apt-get install -y \
    git \
    make \
    cmake \
    debhelper
WORKDIR /BUILD_GAZER/
COPY src ./src
COPY support ./support
COPY CMakeLists.txt .
RUN bash support/requirements.sh
RUN cmake -DCMAKE_BUILD_TYPE=Release .
RUN cmake --build .

FROM alpine:latest
#   Port to listen for connections
EXPOSE 16699
WORKDIR /GAZER/
COPY --from=builder_gazer /BUILD_GAZER/gazer .
CMD ["gazer"]