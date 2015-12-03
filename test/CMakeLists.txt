include_directories ("${CMAKE_CURRENT_LIST_DIR}/support")
# The support header file depends on Halide.h
include_directories("${HALIDE_DIR}/include")
include_directories("${HALIDE_DIR}/tools")
include_directories("${CMAKE_SOURCE_DIR}/include")

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})

macro(add_test testname source_file)
  add_executable (${testname} ${source_file})
  target_link_libraries(${testname} ${PNG_LIBRARY} Lightroom "${HALIDE_DIR}/bin/libHalide.so")
endmacro()

add_test (testPoint test_point.cpp)
add_test (testStencil test_stencil.cpp)
add_test (testDemosaic test_demosaic.cpp)
add_test (testImage test_image.cpp)
add_test (testHDR test_hdr.cpp)
