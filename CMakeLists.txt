cmake_minimum_required(VERSION 3.20)

project(rtbessential2d)
add_subdirectory(SOEM)

add_library(rtbessential2dctl SHARED lib/rtbessential2d.c lib/logic/rtblogic/rtblogic.c)
target_link_libraries(rtbessential2dctl PUBLIC soem)
set_property(TARGET soem PROPERTY POSITION_INDEPENDENT_CODE ON)

if(WIN32)
    add_definitions(-DWPCAP -DHAVE_REMOTE)
    find_path(WINPCAP_INCLUDE_DIR pcap.h
              ${WINPCAP_HINT_INCLUDE_DIR})
    find_library(WINPCAP_LIBRARY NAMES "wpcap"
                                 PATHS ${WINPCAP_HINT_LIB_DIR})

    target_include_directories(rtbessential2dctl PUBLIC SOEM/oshw/win32/wpcap/Include)
    target_link_libraries(rtbessential2dctl PUBLIC ${WINPCAP_LIBRARY})
    target_compile_definitions(rtbessential2dctl PUBLIC LIBEXPORT_EXPORTS)  
elseif(UNIX AND NOT APPLE)
endif()
