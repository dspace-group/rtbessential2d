cmake_minimum_required(VERSION 3.20)

set(WINPCAP_LIBRARY_WPCAP "not set" CACHE PATH "Path to WinPCap library")

project(rtbessential2d)
add_subdirectory(SOEM)

add_library(rtbessential2dctl SHARED lib/rtbtest.c)
target_link_libraries(rtbessential2dctl PUBLIC soem)

if(WIN32)
  target_include_directories(rtbessential2dctl PUBLIC SOEM/oshw/win32/wpcap/Include)
  target_link_directories(rtbessential2dctl PUBLIC ${WINPCAP_LIBRARY_WPCAP})
  target_link_libraries(rtbessential2dctl PUBLIC wpcap.lib)
  target_compile_definitions(rtbessential2dctl PUBLIC LIBEXPORT_EXPORTS)  
elseif(UNIX AND NOT APPLE)
endif()
