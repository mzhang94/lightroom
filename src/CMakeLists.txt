set(SRC
  image.cpp
  util.cpp
  point.cpp
  stencil.cpp
  domain.cpp
  schedule.cpp
  demosaic.cpp
  mkhdr.cpp
  tonemap.cpp
  )

add_library(Lightroom ${SRC})
find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})
include_directories ("${CMAKE_CURRENT_LIST_DIR}/support")
# The support header file depends on Halide.h
include_directories("${HALIDE_DIR}/include")
include_directories("${HALIDE_DIR}/tools")
include_directories(${CMAKE_SOURCE_DIR}/include)
