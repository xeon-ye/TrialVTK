project(PickingManager)

find_package(VTK REQUIRED NO_MODULE)

include(${VTK_USE_FILE})

# Set your files and resources here
set( Srcs
  TestPickingManager.cxx
)

add_executable(TestPickingManager TestPickingManager.cxx)

target_link_libraries(TestPickingManager ${VTK_LIBRARIES})

