#include <QApplication>
#include <QSurfaceFormat>

// #include <QVTKOpenGLWidget.h>

#include "app.hpp"

int main( int argc, char** argv ) {
  // needed to ensure appropriate OpenGL context is created for VTK rendering.
  // QSurfaceFormat::setDefaultFormat(QVTKOpenGLWidget::defaultFormat());

  // QT Stuff
  QApplication app( argc, argv );

  App myApp(argc, argv);
  myApp.show();

  return app.exec();
}

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
