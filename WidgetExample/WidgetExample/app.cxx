/**
 * @file   app.cxx
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Mon Feb 17 20:29:28 2020
 *
 * @brief
 *
 * Copyright 2020 Jens Munk Hansen
 *
 */

#include <WidgetExample/ui_app.h>

#include <QSettings>

#include <cstdlib>
#include <iostream>

#include <WidgetExample/app.hpp>

#include <WidgetExample/filedialog.hpp>

#include <vtkSmartPointer.h>
#include <vtkImageViewer2.h>
#include <vtkDICOMImageReader.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>



App::~App() {}

void App::SetupUI() {
  this->ui = new Ui_App;
  this->ui->setupUi(this);

  this->ui->leftSplitter->setSizes(QList<int>({ INT_MAX, INT_MAX }));
  this->ui->splitter->setSizes(QList<int>({ INT_MAX, INT_MAX }));
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

  FileDialog w;
  qDebug() << MySettings.value(DEFAULT_DIR_KEY).toString();
  w.setDirectory(MySettings.value(DEFAULT_DIR_KEY).toString());

  int nMode = w.exec();
  QStringList fnames = w.selectedFiles();

  if (nMode != 0 && fnames.size() != 0) {
    int i = fnames.size();
    selectedFileOrDirectory = fnames.at(i-1);
    QFileInfo fi = QFileInfo(selectedFileOrDirectory);

    const std::string inputFilename = std::string(selectedFileOrDirectory.toUtf8().constData());

    vtkSmartPointer<vtkDICOMImageReader> reader =
        vtkSmartPointer<vtkDICOMImageReader>::New();
    reader->SetFileName(inputFilename.c_str());
    reader->Update();

    m_imgViewer->SetInputConnection(reader->GetOutputPort());
    m_imgViewer->Render();
    m_imgViewer->GetRenderer()->ResetCamera();
    m_imgViewer->Render();

    this->ui->vtkOpenGLView->GetInteractor()->Enable();

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


App::App(int argc, char* argv[]) : m_imgViewer(nullptr) {
  this->SetupUI();
  this->PopulateMenus();

  vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
  this->ui->vtkOpenGLView->SetRenderWindow(renderWindow);

  // Visualize
  m_imgViewer =
      vtkSmartPointer<vtkImageViewer2>::New();
  m_imgViewer->SetRenderWindow(renderWindow);

  m_imgViewer->SetupInteractor(renderWindow->GetInteractor());

  this->ui->vtkOpenGLView->GetInteractor()->Disable();
}

void App::slotExit() {
  qApp->exit();
}
