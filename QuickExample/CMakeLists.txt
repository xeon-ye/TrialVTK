include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if (WIN32)
  set(GTEST_ROOT "C:/Program Files/googletest-distribution")
endif()

add_subdirectory(QuickExample)

add_subdirectory(viewcore)
