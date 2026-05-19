cmake_minimum_required(VERSION 3.30)

project(113final003)

enable_testing()

list(APPEND CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

if(NOT DEFINED SOURCE_ROOT)
  set(SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/solution" CACHE STRING "學生程式碼的根目錄")
  message(STATUS "Set SOURCE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}/solution")
  message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
endif()

file(GLOB CXX_SOURCE_FILES
  "${SOURCE_ROOT}/*.cxx"
  "${SOURCE_ROOT}/*.cpp"
  "${SOURCE_ROOT}/*.cc"
  "${SOURCE_ROOT}/*.c"
)

include(AddJudge)
AddJudge("case1")
AddJudge("case2")
AddJudge("case3")
AddJudge("case4")
AddJudge("case5")