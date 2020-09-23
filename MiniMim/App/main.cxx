#include <QApplication>
#include <QSurfaceFormat>

#include <QVTKOpenGLWidget.h>

#include "app.hpp"
//#include <SlicerWidget/ui_app.h>
//#include <SlicerWidget/datamanager.h>

int main( int argc, char** argv ) {
  // needed to ensure appropriate OpenGL context is created for VTK rendering.
  // QSurfaceFormat::setDefaultFormat(QVTKOpenGLWidget::defaultFormat());

  QSurfaceFormat format;
  format.setMajorVersion(3);
  format.setMinorVersion(2);
  format.setDepthBufferSize(1);
  format.setStencilBufferSize(1);
  format.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(format);


  // QT Stuff
  QApplication app( argc, argv );

  App myApp(argc, argv);

  app.setApplicationName("SlicerWidget");
  app.setOrganizationName("Sexy Soft");
  app.setOrganizationDomain("www.sexysoft.com");

  myApp.show();




  return app.exec();
}

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
