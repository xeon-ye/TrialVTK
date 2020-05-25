cmake_minimum_required(VERSION 3.9.5)

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

if (ITKVtkGlue_LOADED)
  find_package(VTK REQUIRED)
  include(${VTK_USE_FILE})
else()
  find_package(ItkVtkGlue REQUIRED)
  include(${ItkVtkGlue_USE_FILE})
  set(Glue ItkVtkGlue)
endif()

add_executable(SegmGrow MACOSX_BUNDLE segm.cxx)
target_link_libraries(SegmGrow
  ${Glue}  ${VTK_LIBRARIES} ${ITK_LIBRARIES})

if (WIN32)
  add_custom_command(TARGET SegmGrow POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:SegmGrow> $<TARGET_FILE_DIR:Registration>/$<CONFIGURATION:Registration>)
endif()
