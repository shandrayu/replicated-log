cmake_minimum_required(VERSION 2.8)

project(replicated_log_secondary)
set(PROJECT ${PROJECT_NAME})
string(TOLOWER "${PROJECT}" PROJECT_LC)

include (../common/cmake/mif.cmake)

set(COMMON_HEADERS
    ${COMMON_HEADERS}
)

set(HEADERS
    ${HEADERS}
    ${COMMON_HEADERS}
)

set(JSONCPP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/jsoncpp/dist/jsoncpp.cpp
)

set(SOURCES
    ${SOURCES}
    ${JSONCPP_SOURCES}
)

add_executable(${PROJECT_LC} ${HEADERS} ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_include_directories(${PROJECT_LC} PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/jsoncpp/dist)
target_link_libraries(${PROJECT_LC} ${LIBRARIES})
