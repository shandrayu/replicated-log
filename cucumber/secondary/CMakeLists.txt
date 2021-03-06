cmake_minimum_required(VERSION 3.1)

project(replicated_log_secondary)
set(PROJECT ${PROJECT_NAME})
string(TOLOWER "${PROJECT}" PROJECT_LC)

include (../common/cmake/mif.cmake)

set(COMMON_HEADERS
    ${COMMON_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/replicated_log_node/include/replicated_log_node/replicated_log_node.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/replicated_log_node/include/replicated_log_node/replicated_log_secondary.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/replicated_log_node/src/replicated_log_node.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/replicated_log_node/src/replicated_log_secondary.cpp
)

add_executable(${PROJECT_LC} ${HEADERS} ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_compile_features(${PROJECT_LC} PRIVATE cxx_std_17)
target_include_directories(${PROJECT_LC} PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/jsoncpp/dist)
target_include_directories(${PROJECT_LC} PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../common/replicated_log_node/include)
target_link_libraries(${PROJECT_LC} ${LIBRARIES})
