#pragma once

#include <QMainWindow>
#include <QHBoxLayout>
#include <SlicerWidget/datamanager.h>

#include "vtkSmartPointer.h"
#include "vtkDistanceWidget.h"
#include "vtkContourWidget.h"

class Ui_App;

class App : public QMainWindow {
  Q_OBJECT
 public:
  App(int argc, char *argv[]);
  ~App() override;
 public slots:
  virtual void slotExit();
  void resizeEvent(QResizeEvent* event);

 private Q_SLOTS:
  void onLoadClicked();
  void onLoadSurfaceClicked();
  void onApplyPresetClick();
  
  void referenceViewChanged(int index);
  void ResetViews();
  virtual void AddDistanceMeasurementToView();
  virtual void AddDistanceMeasurementToView(int);
  virtual void AddContourWidgetToView();
  virtual void AddContourWidgetToView(int);

  void ClearDistanceView(int i);
  void ClearDistanceView();

  void ClearContour(int i);
  void ClearContour();

  void resliceMode(int);
  void showPlanes(int);
 private:
  void SetupUI();
  void PopulateMenus();

 public:
  // Designer form
  Ui_App *ui;

  QHBoxLayout* horizontalLayout;
  DataManager* datamanager;
  int iSlice = 0;
protected:
  vtkSmartPointer< vtkDistanceWidget > DistanceWidget[3];
  vtkSmartPointer< vtkContourWidget > ContourWidget[3];
  vtkSmartPointer< vtkActor > Surfaces[2];
};
