include (../common/cmake/settings.cmake)

set (THITD_PARTY_PATH "${CMAKE_SOURCE_DIR}/../third_party")
set (MIF_LIB_PATH "${THITD_PARTY_PATH}/mif")
include (${MIF_LIB_PATH}/cmake/options.cmake)

set (MIF_LIB_THIRD_PARTY_PATH "${THITD_PARTY_PATH}/mif/third_party_sources")

include_directories (SYSTEM ${MIF_LIB_THIRD_PARTY_PATH}/zlib/include)
link_directories (${MIF_LIB_THIRD_PARTY_PATH}/zlib/lib)

include_directories (SYSTEM ${MIF_LIB_THIRD_PARTY_PATH}/boost)
link_directories (${MIF_LIB_THIRD_PARTY_PATH}/boost/lib)

include_directories (SYSTEM ${MIF_LIB_THIRD_PARTY_PATH}/event/include)
link_directories (${MIF_LIB_THIRD_PARTY_PATH}/event/lib)

if (MIF_WITH_POSTGRESQL)
    include_directories (SYSTEM ${MIF_LIB_THIRD_PARTY_PATH}/libpq/include)
    link_directories (${MIF_LIB_THIRD_PARTY_PATH}/libpq/lib)
endif()

if (MIF_WITH_SQLITE)
    include_directories (SYSTEM ${MIF_LIB_THIRD_PARTY_PATH}/sqlite/include)
    link_directories (${MIF_LIB_THIRD_PARTY_PATH}/sqlite/lib)
endif()

include_directories (SYSTEM ${MIF_LIB_THIRD_PARTY_PATH}/mif/include)
link_directories (${MIF_LIB_THIRD_PARTY_PATH}/mif/lib)

include_directories (SYSTEM ${MIF_LIB_PATH}/include)
link_directories (${MIF_LIB_PATH}/lib)
