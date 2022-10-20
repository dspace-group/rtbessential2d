cmake_minimum_required(VERSION 3.20)

project(rtbessential2d)
add_subdirectory(SOEM)

add_library(rtbessential2dctl SHARED lib/rtbessential2d.c lib/logic/rtblogic/rtblogic.c)
target_link_libraries(rtbessential2dctl PUBLIC soem)
set_property(TARGET soem PROPERTY POSITION_INDEPENDENT_CODE ON)

if(WIN32)
    target_include_directories(rtbessential2dctl PUBLIC SOEM/oshw/win32/wpcap/Include)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        target_link_directories(rtbessential2dctl PUBLIC SOEM/oshw/win32/wpcap/Lib/x64)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        target_link_directories(rtbessential2dctl PUBLIC SOEM/oshw/win32/wpcap/Lib)
    endif()
    
    target_link_libraries(rtbessential2dctl PUBLIC wpcap.lib)
    #set(OS_LIBS  Packet.lib Ws2_32.lib Winmm.lib)
    target_compile_definitions(rtbessential2dctl PUBLIC LIBEXPORT_EXPORTS)  
endif()
