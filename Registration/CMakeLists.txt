include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

#add_subdirectory(MI)
add_subdirectory(Registration)
add_subdirectory(Segmentation)
add_subdirectory(Surfacing)
