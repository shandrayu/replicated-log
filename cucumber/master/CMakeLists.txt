cmake_minimum_required(VERSION 3.1)

project(replicated_log_master)
set(PROJECT ${PROJECT_NAME})
string(TOLOWER "${PROJECT}" PROJECT_LC)

include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git GIT_TAG f4622efcb59d84071ae11404ae61bd821c1c344b) # the commit hash for 1.6.2
FetchContent_MakeAvailable(cpr)

include (../common/cmake/mif.cmake)

set(COMMON_HEADERS
    ${COMMON_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/replicated_log_node/include/replicated_log_node/replicated_log_node.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/replicated_log_node/include/replicated_log_node/replicated_log_master.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/countdown_latch/include/countdown_latch/countdown_latch.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/replicated_log_node/src/replicated_log_master.cpp
)

add_executable(${PROJECT_LC} ${HEADERS} ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_include_directories(${PROJECT_LC} PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/jsoncpp/dist)
target_include_directories(${PROJECT_LC} PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../common/replicated_log_node/include)
target_include_directories(${PROJECT_LC} PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../common/countdown_latch/include)
target_link_libraries(${PROJECT_LC} ${LIBRARIES} cpr::cpr)
