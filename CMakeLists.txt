project( WavDecoder )
cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
      
message( STATUS "Selected toolchain [${CMAKE_CXX_COMPILER_ID}] on [${CMAKE_SYSTEM_NAME}]")
message( STATUS "CMake Binary dir: ${CMAKE_BINARY_DIR}")
set( HEADERS BandPassFilters.h FileUtils.h LockDetector.h Integrators.h LowPassFilters.h WavFormat.h CostasLoop.h )

add_executable( WavReader WavReader.cpp )
add_executable( WavWriter WavWriter.cpp )
add_executable( testBandFilters testBandFilters.cpp )
add_executable( testWaveGen testWaveGen.cpp )

target_compile_features(WavReader PRIVATE cxx_range_for)
target_compile_features(WavWriter PRIVATE cxx_range_for)

add_custom_command( OUTPUT sample.wav DEPENDS ${HEADERS} WavWriter COMMAND ${CMAKE_BINARY_DIR}/WavWriter sample.bit sample.wav )

