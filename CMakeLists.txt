cmake_minimum_required(VERSION 3.12)
project(NC)

set(CMAKE_CXX_STANDARD 14)

#set(CMAKE_CXX_FLAGS  -fno-stack-protector)

add_executable(compare compare.cpp src/FileReader.h src/FileWriter.h)

add_executable(NC main.cpp src/FileReader.h src/Fitter.h src/CompressionConfig.h src/Type.h src/FileWriter.h src/FloatCompressor.h src/ArithmeticCoder.h src/acoder.hxx src/acoder.cxx src/pcoder.hxx src/pcoder.cxx src/ahcoder.cxx src/ahcoder.hxx src/bhcoder.cxx src/bhcoder.hxx)