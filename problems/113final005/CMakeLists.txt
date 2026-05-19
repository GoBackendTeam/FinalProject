cmake_minimum_required(VERSION 3.30)

project(113final005)

enable_testing()

list(APPEND CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

if(NOT DEFINED SOURCE_ROOT)
  set(SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/solution" CACHE STRING "Root of student source code")
  message(STATUS "Set SOURCE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}/solution")
  message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
endif()

file(GLOB CXX_SOURCE_FILES
  "${SOURCE_ROOT}/*.cxx"
  "${SOURCE_ROOT}/*.cpp"
  "${SOURCE_ROOT}/*.cc"
  "${SOURCE_ROOT}/*.c"
)
message(STATUS "Source files: ${CXX_SOURCE_FILES}")

include(AddJudge)
AddJudge("case1")
AddJudge("case2")
AddJudge("case3")
AddJudge("case4")
AddJudge("case5")