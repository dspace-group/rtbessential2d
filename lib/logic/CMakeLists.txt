cmake_minimum_required(VERSION 3.20)

set(MATLAB_EXTERN_INCLUDE "not set" CACHE PATH "Path to MATLAB extern include dir")
set(MATLAB_SIMULINK_INCLUDE "not set" CACHE PATH "Path to MATLAB simulink include dir")
set(MATLAB_RTW_C_SRC "not set" CACHE PATH "Path to MATLAB rtw c src dir")

project(rtblogic)

add_library(rtblogic STATIC rtblogic.c)
target_include_directories(rtblogic PUBLIC .)
target_include_directories(rtblogic PUBLIC ${MATLAB_EXTERN_INCLUDE})
target_include_directories(rtblogic PUBLIC ${MATLAB_SIMULINK_INCLUDE})
target_include_directories(rtblogic PUBLIC ${MATLAB_RTW_C_SRC})