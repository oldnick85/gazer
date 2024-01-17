cmake_minimum_required(VERSION 3.14)
project(gazer
    VERSION 1.0.0
    DESCRIPTION "Simple statistic collector with web interface"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GAZER_SRC
    src/main.cpp
    src/application.cpp
    src/global.cpp
    src/config/config.cpp
    src/actors/collector/collector.cpp
    src/actors/data/data.cpp
    src/actors/http/httpserver.cpp)

set(EXTERNALDIR external)

add_executable(gazer ${GAZER_SRC})

target_include_directories(gazer PRIVATE
    src
    src/config
    src/http
    src/adapters/verification
    src/adapters/database
    src/api
    src/common
    src/actor_system
    ${EXTERNALDIR}/json/single_include
    ${EXTERNALDIR}/cpp-httplib
    ${EXTERNALDIR}/spdlog/include)