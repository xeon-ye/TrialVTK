#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QWidget>

namespace Ui {
class DataManager;
}

#include <vtkSmartPointer.h>
#include <vtkResliceImageViewer.h>
#include <vtkImagePlaneWidget.h>
#include <vtkImageViewer2.h>

class DataManager : public QWidget {
  Q_OBJECT

 public:
  explicit DataManager(QWidget *parent = 0);
  ~DataManager();

 private:
  Ui::DataManager *ui;

  vtkSmartPointer<vtkResliceImageViewer> m_riw[3];
  vtkSmartPointer< vtkImagePlaneWidget > m_planeWidget[3];
};

#endif // DATAMANAGER_H
