function(AddJudge testFilename)
  set(TARGET_NAME "${PROJECT_NAME}-${testFilename}")
  set(PRIVATE_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_${testFilename}_")
  set(CASE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/spec/${testFilename}.h")
  set(CASE_DST "${PRIVATE_INCLUDE_DIR}/case.h")

  file(SHA3_256 ${CASE_SRC} SECRET)
  file(MAKE_DIRECTORY "${PRIVATE_INCLUDE_DIR}")

  message("Create ${TARGET_NAME} Build Context")
  configure_file(${CASE_SRC} ${CASE_DST} @ONLY)
  message(STATUS "Copy ${CASE_SRC} to ${CASE_DST}")

  add_executable(${TARGET_NAME} ${CXX_SOURCE_FILES})

  
  message(STATUS "Add include: ${PRIVATE_INCLUDE_DIR}")
  message(STATUS "Add include: ${SOURCE_ROOT}")
  message(STATUS "Add include: ${SOURCE_ROOT}/include")
  message(STATUS "Add include: ${SOURCE_ROOT}/runtime")
  target_include_directories(${TARGET_NAME} BEFORE PRIVATE
    "${PRIVATE_INCLUDE_DIR}"
  )

  target_include_directories(${TARGET_NAME} PRIVATE
    "${SOURCE_ROOT}"
    "${SOURCE_ROOT}/include"
    "${SOURCE_ROOT}/runtime"
  )

  message(STATUS "Generated target ${TARGET_NAME} using ${testFilename}.h")
  message(STATUS "Add include: ${SOURCE_ROOT}")
  add_test(NAME "${TARGET_NAME}-Test" COMMAND ${TARGET_NAME})
  message(STATUS "Added test target: ${TARGET_NAME}")
endfunction()
