#pragma once

#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <QDebug>
#include <QMap>
#include <QVector>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QWidget>

class Ui_Registration;

#include <Registration/reslicecallback.h>

#include <Registration/runnable.hpp>
#include <Registration/SegRunner.hpp>
#include <Registration/SurfRunner.hpp>

#include <Registration/RangeSlider.h>
#include <Registration/transformmodel.h>

#include <vtkSmartPointer.h>
#include <vtkResliceImageViewer.h>
#include <vtkImagePlaneWidget.h>
#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkImageViewer2.h>
#include <vtkImageReader2.h>

#include <vtkSeedWidget.h>

#include <vtkEventQtSlotConnect.h>

class App : public QMainWindow {
  Q_OBJECT
 public:
  App(int argc, char *argv[]);
  ~App() override;

 public slots:
  void SeedsUpdated(vtkObject* obj, unsigned long event, void* calldata, void* clientData);

  void SeedsUpdateData(vtkObject* obj, unsigned long event, void* calldata, void* clientData);

  void TransformationUpdated(int index);

  virtual void slotExit();
  void resizeEvent(QResizeEvent* event);
  void FileLoad(const QString &files, int type=0);
  void setZoom(int zoom);

  void dumpImages();
  void dumpImageBackBuffers(int index);
  void dumpImageOffscreen();

  void segmSliderChanged(int index);

  void AddSeedsToView1();
  void AddSeedsToView( int );
  void ClearSeedsInView1();
  void ClearSeedsInView(int i);

  void SliderLow(int value);
  void SliderHigh(int value);

  void keyPressEvent(QKeyEvent *event);

 private Q_SLOTS:
  void updateSurface();

  void updateChildWidgets();
  void updateSegChildWidgets();
  void updateSurfChildWidgets();

  void Render();
  void onLoadMRClicked();
  void onLoadUSClicked();
  void onLoadVesselsClicked();

  void checkIfDone();
  void checkIfSegDone();
  void checkIfSurfDone();

  void onSegClick();
  void onSegStartClick();

  void onSegStartInView(int);

  void onSegCancelClick();

  void onSurfClick();
  void onSurfStartClick();
  void onSurfCancelClick();


  void onRegClick();
  void onApplyPresetClick();
  void onRegStartClick();
  void onCancelClick();

  void updateProgressBar(int progressPercent);
  void updateSegProgressBar(int progressPercent);

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
  Ui_Registration *ui;

 private:
  QVariantMap data;
  volatile bool stopped;
  volatile bool segStopped;
  volatile bool surfStopped;


  void (App::*regDelegate) ();
  void (App::*segDelegate) ();
  void (App::*surfDelegate) ();

  vtkSmartPointer<vtkResliceCursorCallback> cbk;

  int retval;
  RegRunner* regrunner;

  SegRunner* segRunner;
  SurfRunner* surfRunner;

  RangeSlider* thresholdsSlider;

  QHBoxLayout* m_sliderLayout;
  QLineEdit* m_sliderLblHigh;
  QLineEdit* m_sliderLblLow;
  QWidget* m_pWidget;

  TransformModel* transModel;

  // MR view stuff
  vtkSmartPointer<vtkImageData> m_dummy;
  vtkSmartPointer<vtkResliceImageViewer> m_riw[3];
  vtkSmartPointer< vtkImagePlaneWidget > m_planeWidget[3];

  // US view stuff
  vtkSmartPointer<vtkImageData> m_dummy1;
  vtkSmartPointer<vtkResliceImageViewer> m_riw_us[3];

  vtkSmartPointer<vtkImageData> m_segmentation;
  vtkSmartPointer<vtkPolyData> m_polydata;

  vtkSmartPointer<vtkEventQtSlotConnect> Connections;

  vtkSmartPointer<vtkEventQtSlotConnect> ConnectionsTest;


  // Segmentation stuff
  vtkSmartPointer<vtkSeedWidget> m_seeds[3];

  vtkSmartPointer<vtkActor> m_vessels;

  // Ugly explicit data (just for testing).
  float m_vsum;
  float m_vsum2;

};

