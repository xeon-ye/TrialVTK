#pragma once

#include <QMainWindow>
#include <QtWidgets/QHBoxLayout>

class Ui_App;

#include <vtkSmartPointer.h>
#include <vtkResliceImageViewer.h>
#include <vtkImagePlaneWidget.h>
#include <vtkImageData.h>
#include <vtkImageViewer2.h>
#include <vtkImageReader2.h>


class App : public QMainWindow {
  Q_OBJECT
 public:
  App(int argc, char *argv[]);
  ~App() override;
 public slots:
  virtual void slotExit();
  void resizeEvent(QResizeEvent* event);
  void FileLoad(const QString &files, int type=0);

 private Q_SLOTS:
  void Render();
  void onLoadMRClicked();
  void onLoadUSClicked();

 private:
  void setupMR();
  void setupUS();
  void SetupUI();
  void PopulateMenus();
  void FileLoadMR(const vtkSmartPointer<vtkImageReader2>& files);
  void FileLoadUS(const vtkSmartPointer<vtkImageReader2>& files);
  void resliceMode(int mode);
  void ResetViews();

 public:
  // Designer form
  Ui_App *ui;

 private:
  // MR view stuff
  vtkSmartPointer<vtkImageData> m_dummy;
  vtkSmartPointer<vtkResliceImageViewer> m_riw[3];
  vtkSmartPointer< vtkImagePlaneWidget > m_planeWidget[3];

  // US view stuff
  vtkSmartPointer<vtkImageData> m_dummy1;
  vtkSmartPointer<vtkResliceImageViewer> m_riw_us[3];
};
