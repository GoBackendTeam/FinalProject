cmake_minimum_required(VERSION 3.10)

set(TARGET 005-solution)

file(GLOB CXX_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

add_executable(${TARGET} ${CXX_SOURCES})

target_include_directories(${TARGET}
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/shared"
)

add_test(
  NAME "${TARGET}-test"
  COMMAND ${TARGET}
)