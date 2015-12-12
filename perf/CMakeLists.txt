include_directories ("${CMAKE_CURRENT_LIST_DIR}/support")
# The support header file depends on Halide.h
include_directories("${HALIDE_DIR}/include")
include_directories("${HALIDE_DIR}/tools")
include_directories("${CMAKE_SOURCE_DIR}/include")

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})
set(CMAKE_CURRENT_BINARY ${CMAKE_SOURE_DIR}/perf)

macro(add_test testname source_file)
  add_executable (${testname} ${source_file})
  target_link_libraries(${testname} ${PNG_LIBRARY} Lightroom "${HALIDE_DIR}/bin/libHalide.so")
endmacro()

macro(add_perf_run testname source_file)
  add_executable (${testname} ${source_file})
  target_link_libraries(${testname} ${PNG_LIBRARY} Lightroom "${HALIDE_DIR}/bin/libHalide.so")
endmacro()

add_test(perfGaussian gaussian.cpp)
add_test(perfBilateral bilateral.cpp)
add_test(perfHDR hdr_tonemap.cpp)
