cmake_minimum_required(VERSION 3.14)
project(hw5)

set(CMAKE_CXX_STANDARD 14)
FILE(GLOB MySources *.cpp)
FILE(GLOB MyHeaders *.hpp)

ADD_EXECUTABLE(MyExecutable ${MySources} ${MyHeaders})
