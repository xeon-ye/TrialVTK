#include <SlicerWidget/ui_app.h>

#include <QSettings>

#include <cstdlib>
#include <iostream>

#include <SlicerWidget/app.hpp>

App::~App() {}

void App::SetupUI() {
  this->ui = new Ui_App;
  this->ui->setupUi(this);

  datamanager = new DataManager(this);
  horizontalLayout = new QHBoxLayout(this->ui->centralwidget);
  horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
  horizontalLayout->addWidget(this->datamanager);
}

void App::PopulateMenus() {
  connect(this->ui->actionExit, SIGNAL(triggered()),
          this, SLOT(slotExit()));
  connect(this->ui->actionOpen, SIGNAL(triggered()),
          this, SLOT(onLoadClicked()));
}

void App::onLoadClicked() {
  // TODO: Make 3D viewer work
  const QString DEFAULT_DIR_KEY("default_dir");
  QSettings MySettings;

  QString selectedFileOrDirectory;
}


App::App(int argc, char* argv[]) {
  this->SetupUI();
  this->PopulateMenus();
}

void App::slotExit() {
  qApp->exit();
}

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
