cmake_minimum_required(VERSION 3.12)
project(NC)

set(CMAKE_CXX_STANDARD 14)

add_executable(NC main.cpp src/FileReader.h src/Fitter.h src/SevenMethods/CompressionConfig.h src/SevenMethods/Type.h src/FileWriter.h src/FloatCompressor.h src/ArithmeticCoder.h)