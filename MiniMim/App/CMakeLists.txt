# Set your files and resources here
set( Srcs
  main.cxx
  app.cxx
  filedialog.cxx
  )

set( Hdrs app.hpp
  )

set( MOC_Hdrs app.hpp )

set( UIs app.ui)

qt5_wrap_ui(UI_Srcs ${UIs})

# CMAKE_AUTOMOC in ON so the MocHdrs will be automatically wrapped.
add_executable(MiniMim ${Srcs} ${Hdrs} ${UI_Srcs} ${MOC_Hdrs} .clang-format)

if (WIN32)
  #get_target_property(_outdir SlicerWidget RUNTIME_OUTPUT_DIRECTORY)
  #message(${_outdir})
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/MiniMim.bat.in" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/MiniMim.bat" @ONLY)
endif()

target_link_libraries(MiniMim SlicerWidget2)
target_link_libraries(MiniMim Qt5::Core Qt5::Gui Qt5::Widgets)
target_link_libraries(MiniMim ${VTK_LIBRARIES})
