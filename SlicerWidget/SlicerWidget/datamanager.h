/**
 * @file   datamanager.h
 * @author Jens Munk Hansen <jens.munk.hansen@gmail.com>
 * @date   Thu Feb 20 00:34:22 2020
 *
 * @brief
 *
 * Copyright 2020
 *
 */
#pragma once

#include <QWidget>

namespace Ui {
class DataManager;
}

#include <vtkSmartPointer.h>
#include <vtkResliceImageViewer.h>
#include <vtkImagePlaneWidget.h>
#include <vtkImageData.h>
#include <vtkImageViewer2.h>
#include <vtkImageReader2.h>

#include <SlicerWidget/reslicecallback.h>

class DataManager : public QWidget {
  Q_OBJECT

 public:
  explicit DataManager(QWidget *parent = 0);
  ~DataManager();

 public Q_SLOTS:
  void Render();
  void FileLoad(const QString& files);
  void SetReferenceSlice(int iSlice);

 private Q_SLOTS:

 private:
  void FileLoad1(const vtkSmartPointer<vtkImageReader2>& files);
  void resliceMode(int mode);
  void ResetViews();
  Ui::DataManager *ui;

  //  vtkSmartPointer<vtkImageData> m_dummy;
  vtkSmartPointer<vtkResliceImageViewer> m_riw[3];
  // vtkSmartPointer<vtkImageViewer2> m_riw[3];
  vtkSmartPointer< vtkImagePlaneWidget > m_planeWidget[3];
  vtkSmartPointer< vtkResliceCursorCallback > m_cbk;
};
