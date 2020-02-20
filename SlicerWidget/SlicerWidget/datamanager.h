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
#include <vtkImageViewer2.h>

class DataManager : public QWidget {
  Q_OBJECT

 public:
  explicit DataManager(QWidget *parent = 0);
  ~DataManager();

 public Q_SLOTS:
  void Render();
  void OnLoadClicked();

 private Q_SLOTS:

 private:
  Ui::DataManager *ui;

  vtkSmartPointer<vtkResliceImageViewer> m_riw[3];
  vtkSmartPointer< vtkImagePlaneWidget > m_planeWidget[3];
};
