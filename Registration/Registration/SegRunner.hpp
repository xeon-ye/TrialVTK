#pragma once

#include <cstdint>

#include <QRunnable>
#include <QProgressBar>

#include <QString>
#include <QVariant>
#include <QMap>

#include <vtkSmartPointer.h>
#include <vtkImageData.h>

class SegRunner : public QRunnable {
 public:
  SegRunner(QWidget *receiver,
            QMap<QString, QVariant> data,
            vtkImageData* pData,
            vtkSmartPointer<vtkImageData>& pOutData,
            int* retval,
            volatile bool *stopped);
  void run();
 private:

  void externalRun();
  void internalRun();

  QWidget* receiver;
  vtkImageData* pData;
  vtkSmartPointer<vtkImageData>& pOutData;
  int* retval;
  volatile bool* stopped;
  QMap<QString, QVariant> data;
};

