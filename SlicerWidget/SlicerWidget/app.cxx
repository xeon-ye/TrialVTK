#include <SlicerWidget/ui_app.h>

#include <QSettings>
#include <QDebug>

#include <cstdlib>
#include <iostream>

#include <SlicerWidget/app.hpp>
#include <SlicerWidget/filedialog.hpp>

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
  const QString DEFAULT_DIR_KEY("SlicerWidgetDefaultDir");
  QSettings MySettings;

  QString selectedDirectory;

  FileDialog w;
  qDebug() << "default_dir:" << MySettings.value(DEFAULT_DIR_KEY).toString();
  w.setDirectory(MySettings.value(DEFAULT_DIR_KEY).toString());

  int nMode = w.exec();
  QStringList fnames = w.selectedFiles();

  if (nMode != 0 && fnames.size() != 0) {
    qDebug() << fnames;

    datamanager->FileLoad(fnames[0]);

    int i = fnames.size();
    QString selectedFileOrDirectory = fnames.at(i-1);

    QFileInfo fi = QFileInfo(selectedFileOrDirectory);

    if (fi.isDir()) {
      QDir currentDir(selectedFileOrDirectory);
      currentDir.cdUp(); // Maybe necessary
      MySettings.setValue(DEFAULT_DIR_KEY,
                          currentDir.absolutePath());
      qDebug() << currentDir.absolutePath();
    } else {
      QDir currentDir = QFileInfo(selectedFileOrDirectory).absoluteDir();
      qDebug() << currentDir.absolutePath();
      MySettings.setValue(DEFAULT_DIR_KEY,
                          currentDir.absolutePath());
    }
  }
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

