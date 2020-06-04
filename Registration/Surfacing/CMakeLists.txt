cmake_minimum_required(VERSION 3.9.5)

if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
#  add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()

find_package(VTK COMPONENTS
  vtkCommonCore
  vtkFiltersSources
  vtkGUISupportQt
  vtkIOImage
  vtkInteractionImage
)

include(${VTK_USE_FILE})

# Set your files and resources here
set( Srcs
  surf.cxx
  ../Registration/whereami.c
)

set( Hdrs
  )

add_executable(Surf ${Srcs} ${Hdrs})

target_link_libraries(Surf ${VTK_LIBRARIES})

include(deploy_vtk_windows)
# windeployqt(Registration ${CMAKE_CURRENT_BINARY_DIR})

if (WIN32)
  windeployvtk(Surf $<$<CONFIG:Release>:${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}>$<$<NOT:$<CONFIG:Release>>:${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}>)
endif()
