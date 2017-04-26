project( WavDecoder )
cmake_minimum_required(VERSION 3.1)

message( STATUS "Selected toolchain [${CMAKE_CXX_COMPILER_ID}] on [${CMAKE_SYSTEM_NAME}]")
message( STATUS "CMake Binary dir: ${CMAKE_BINARY_DIR}")
set( HEADERS BandPassFilters.h FileUtils.h LockDetector.h Integrators.h LowPassFilters.h WavFormat.h CostasLoop.h )

add_executable( WavReader WavReader.cpp )
add_executable( WavWriter WavWriter.cpp )

add_custom_command( OUTPUT sample.wav DEPENDS ${HEADERS} WavWriter COMMAND ${CMAKE_BINARY_DIR}/WavWriter sample.bit sample.wav )

