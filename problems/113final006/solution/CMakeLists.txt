cmake_minimum_required(VERSION 3.28)

project(113final006)

add_executable(entrypoint)

target_include_directories(
  entrypoint
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/runtime
)

file(GLOB CXX_SOURCE_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

target_sources(
  entrypoint
  PUBLIC
  ${CXX_SOURCE_FILES}
)