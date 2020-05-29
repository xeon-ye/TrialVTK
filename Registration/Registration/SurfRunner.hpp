#pragma once

#include <cstdint>

#include <QRunnable>
#include <QProgressBar>

#include <QString>
#include <QVariant>
#include <QMap>

#include <vtkPolyData.h>
#include <vtkImageData.h>

class SurfRunner : public QObject, public QRunnable {
    Q_OBJECT
 public:
  SurfRunner(QWidget *receiver,
             QMap<QString, QVariant> data,
             vtkPolyData* pPolyData,
             vtkImageData* pData,
             int* retval,
             volatile bool *stopped);
  void run();
  void internalRun();
  void externalRun();
signals:
    void Done();
 private:
  QWidget* receiver;
  int* retval;
  volatile bool* stopped; // TODO: Should be atomic
  vtkPolyData* pPolyData;
  vtkImageData* pData;
  QMap<QString, QVariant> data;
};

