#pragma once

#include <QMainWindow>

#include <vtkSmartPointer.h>
#include <vtkImageViewer2.h>

class Ui_App;

class App : public QMainWindow {
  Q_OBJECT
 public:
  App(int argc, char *argv[]);
  ~App() override;
 public slots:
  virtual void slotExit();
 private Q_SLOTS:
  void onLoadClicked();

 private:
  void SetupUI();
  void PopulateMenus();

  vtkSmartPointer<vtkImageViewer2> m_imgViewer;

  // Designer form
  Ui_App *ui;
};
