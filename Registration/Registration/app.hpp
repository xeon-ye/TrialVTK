#pragma once

#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <QDebug>
#include <QMap>
#include <QVector>

class Ui_Registration;

#include <Registration/reslicecallback.h>

#include <Registration/runnable.hpp>

//#include <Registration/QRangeSlider.hpp>
//#include <Registration/superslider.h>
#include <Registration/RangeSlider.h>

#include <vtkSmartPointer.h>
#include <vtkResliceImageViewer.h>
#include <vtkImagePlaneWidget.h>
#include <vtkImageData.h>
#include <vtkImageViewer2.h>
#include <vtkImageReader2.h>

#include <vtkSeedWidget.h>

class App : public QMainWindow {
  Q_OBJECT
 public:
  App(int argc, char *argv[]);
  ~App() override;
 
 public slots:
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

 private Q_SLOTS:
  void updateChildWidgets();
  void Render();
  void onLoadMRClicked();
  void onLoadUSClicked();
  void onLoadVesselsClicked();

  void checkIfDone();

  void onSegClick();

  void onRegClick();
  void onApplyPresetClick();
  void onRegStartClick();
  void onCancelClick();

  void updateProgressBar(int progressPercent);

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

  void (App::*regDelegate) ();

  vtkSmartPointer<vtkResliceCursorCallback> cbk;

  int retval;
  RegRunner* regrunner;
  RangeSlider* thresholdsSlider;

  // MR view stuff
  vtkSmartPointer<vtkImageData> m_dummy;
  vtkSmartPointer<vtkResliceImageViewer> m_riw[3];
  vtkSmartPointer< vtkImagePlaneWidget > m_planeWidget[3];

  // US view stuff
  vtkSmartPointer<vtkImageData> m_dummy1;
  vtkSmartPointer<vtkResliceImageViewer> m_riw_us[3];

  // Segmentation stuff
  vtkSmartPointer<vtkSeedWidget> m_seeds[3];
};
