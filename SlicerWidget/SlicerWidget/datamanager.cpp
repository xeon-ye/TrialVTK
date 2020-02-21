/**
 * @file   datamanager.cpp
 * @author Jens Munk Hansen <jens.munk.hansen@gmail.com>
 * @date   Thu Feb 20 00:33:37 2020
 *
 * @brief
 *
 * Copyright 2020
 *
 */

#include <SlicerWidget/datamanager.h>
#include <SlicerWidget/ui_datamanager.h>

#include <SlicerWidget/reslicecallback.h>

#include "QVTKOpenGLWidget.h"
#include <QDir>
#include <QSettings>

#include <vtkCellPicker.h>
#include <vtkDICOMImageReader.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageReader2.h>
#include <vtkInteractorStyleRubberBandZoom.h>
#include <vtkMetaImageReader.h>
#include <vtkNrrdReader.h>

#include <vtkPlane.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

#include <vtkResliceCursor.h>
#include <vtkResliceCursorActor.h>
#include <vtkResliceCursorLineRepresentation.h>
#include <vtkResliceCursorPolyDataAlgorithm.h>
#include <vtkResliceCursorWidget.h>

#include <vtkResliceImageViewer.h>

DataManager::DataManager(QWidget *parent) : QWidget(parent), ui(new Ui::DataManager) {
  m_riw[0] = nullptr;
  m_riw[1] = nullptr;
  m_riw[2] = nullptr;
  m_planeWidget[0] = nullptr;
  m_planeWidget[1] = nullptr;
  m_planeWidget[2] = nullptr;

  ui->setupUi(this);

  // Create reslice image widgets
  QVTKOpenGLWidget *ppVTKOGLWidgets[4] = {
    this->ui->view0,
    this->ui->view1,
    this->ui->view2,
    this->ui->view3
  };

  for (size_t i = 0; i < 3; i++) {
    m_riw[i] = vtkSmartPointer<vtkResliceImageViewer>::New();
    vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
    m_riw[i]->SetRenderWindow(renderWindow);
    ppVTKOGLWidgets[i]->SetRenderWindow(m_riw[i]->GetRenderWindow());

    m_riw[i]->SetupInteractor(
      ppVTKOGLWidgets[i]->GetRenderWindow()->GetInteractor());

    vtkSmartPointer<vtkInteractorStyleRubberBandZoom> style =
      vtkSmartPointer<vtkInteractorStyleRubberBandZoom>::New();
    if (i == 0) {
      m_riw[i]->GetInteractor()->SetInteractorStyle(style);
    }

    // Disable interactor until data are present
    ppVTKOGLWidgets[i]->GetInteractor()->Disable();
  }

  // Disable interactor
  ppVTKOGLWidgets[3]->GetInteractor()->Disable();

  // Setup cursors and orientation of reslice image widgets
  for (int i = 0; i < 3; i++) {
    vtkResliceCursorLineRepresentation *rep =
      vtkResliceCursorLineRepresentation::SafeDownCast(
        m_riw[i]->GetResliceCursorWidget()->GetRepresentation());

    // Make all reslice image viewers share the same reslice cursor object.
    m_riw[i]->SetResliceCursor(m_riw[0]->GetResliceCursor());

    rep->GetResliceCursorActor()->GetCursorAlgorithm()->SetReslicePlaneNormal(i);
    m_riw[i]->SetSliceOrientation(i);
    m_riw[i]->SetResliceModeToAxisAligned();
  }

  // Create 3D viewer
  vtkSmartPointer<vtkCellPicker> picker =
    vtkSmartPointer<vtkCellPicker>::New();
  picker->SetTolerance(0.005);

  vtkSmartPointer<vtkProperty> ipwProp =
    vtkSmartPointer<vtkProperty>::New();

  vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;

  this->ui->view3->SetRenderWindow(renderWindow);

  vtkSmartPointer<vtkRenderer> ren =
    vtkSmartPointer<vtkRenderer>::New();
  // Why both a GL and conventional
  this->ui->view3->GetRenderWindow()->AddRenderer(ren);

  vtkRenderWindowInteractor *iren = this->ui->view3->GetInteractor();

  // Should be disabled!!!
  iren->ReInitialize();

  for (int i = 0; i < 3; i++) {
    m_planeWidget[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();

    // No need to add as an vtkActor to the renderer - it is a widget
    m_planeWidget[i]->SetInteractor(iren);
    m_planeWidget[i]->SetPicker(picker); // PICKER
    m_planeWidget[i]->RestrictPlaneToVolumeOn();
    double color[3] = { 0, 0, 0 };
    color[i] = 1;
    m_planeWidget[i]->GetPlaneProperty()->SetColor(color);

    color[0] /= 4.0;
    color[1] /= 4.0;
    color[2] /= 4.0;
    m_riw[i]->GetRenderer()->SetBackground(color);

    m_planeWidget[i]->SetTexturePlaneProperty(ipwProp); // PROPERTY
    m_planeWidget[i]->TextureInterpolateOff();
    m_planeWidget[i]->SetResliceInterpolateToLinear();

    m_planeWidget[i]->DisplayTextOn();
    m_planeWidget[i]->SetDefaultRenderer(ren); // RENDERER
    m_planeWidget[i]->SetWindowLevel(1358, -27);
    m_planeWidget[i]->On();
    m_planeWidget[i]->InteractionOn();

    // Disable interactors
    m_planeWidget[i]->GetInteractor()->Disable();
  }
  // Establish callbacks
  vtkSmartPointer<vtkResliceCursorCallback> cbk =
    vtkSmartPointer<vtkResliceCursorCallback>::New();

  for (int i = 0; i < 3; i++) {
    cbk->IPW[i] = m_planeWidget[i];
    cbk->RCW[i] = m_riw[i]->GetResliceCursorWidget();
    m_riw[i]->GetResliceCursorWidget()->AddObserver(
      vtkResliceCursorWidget::ResliceAxesChangedEvent, cbk);
    m_riw[i]->GetResliceCursorWidget()->AddObserver(
      vtkResliceCursorWidget::WindowLevelEvent, cbk);
    m_riw[i]->GetResliceCursorWidget()->AddObserver(
      vtkResliceCursorWidget::ResliceThicknessChangedEvent, cbk);
    m_riw[i]->GetResliceCursorWidget()->AddObserver(
      vtkResliceCursorWidget::ResetCursorEvent, cbk);
    m_riw[i]->GetInteractorStyle()->AddObserver(
      vtkCommand::WindowLevelEvent, cbk);

    // Make them all share the same color map.
    m_riw[i]->SetLookupTable(m_riw[0]->GetLookupTable());
    m_planeWidget[i]->GetColorMap()->SetLookupTable(m_riw[0]->GetLookupTable());
    m_planeWidget[i]->SetColorMap(
      m_riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap());
  }

  this->ui->view0->show();
  this->ui->view1->show();
  this->ui->view2->show();

  this->Render();
}

void DataManager::ResetViews() {
  // Reset the reslice image views
  for (int i = 0; i < 3; i++) {
    m_riw[i]->Reset();
  }

  // Also sync the Image plane widget on the 3D top right view with any
  // changes to the reslice cursor.
  for (int i = 0; i < 3; i++) {
    vtkPlaneSource *ps = static_cast<vtkPlaneSource *>(
                           m_planeWidget[i]->GetPolyDataAlgorithm());
    ps->SetNormal(m_riw[0]->GetResliceCursor()->GetPlane(i)->GetNormal());
    ps->SetCenter(m_riw[0]->GetResliceCursor()->GetPlane(i)->GetOrigin());

    // If the reslice plane has modified, update it on the 3D widget
    this->m_planeWidget[i]->UpdatePlacement();
  }

  // Render in response to changes.
  this->Render();
}

void DataManager::Render() {
  for (int i = 0; i < 3; i++) {
    m_riw[i]->Render();
  }
  this->ui->view3->GetRenderWindow()->Render();
}

// Change to public slot with string argument

void DataManager::FileLoad(const QString &files) {
  QDir directory = QDir(files);

  vtkSmartPointer<vtkImageReader2> reader;

  if (directory.exists()) {
    // Anticipate, it is a directory of DICOM files
    vtkSmartPointer<vtkDICOMImageReader>
    reader0 =
      vtkSmartPointer<vtkDICOMImageReader>::New();
    reader0->SetDirectoryName(files.toUtf8().constData());
    reader0->Update();
    reader = reader0;
  } else {
    QFileInfo info(files);
    if (info.completeSuffix() == QLatin1String("mhd")) {
      vtkNew<vtkMetaImageReader> reader2;
      reader2->SetFileName(files.toUtf8().constData());
      reader2->Update();
      reader = reader2;
    } else {
      vtkSmartPointer<vtkNrrdReader> reader1 =
        vtkSmartPointer<vtkNrrdReader>::New();
      reader1->SetFileName(files.toUtf8().constData());
      reader1->Update();
      reader = reader1;
    }
  }




  QVTKOpenGLWidget *ppVTKOGLWidgets[4] = {
    this->ui->view0,
    this->ui->view1,
    this->ui->view2,
    this->ui->view3
  };

  // Set input and enable interactors
  for (size_t i = 0; i < 3; i++) {
    m_riw[i]->SetInputData(reader->GetOutput());
    ppVTKOGLWidgets[i]->GetInteractor()->Enable();
    m_planeWidget[i]->SetInputConnection(reader->GetOutputPort());
    m_planeWidget[i]->RestrictPlaneToVolumeOn();

    m_planeWidget[i]->RestrictPlaneToVolumeOn();
    m_planeWidget[i]->GetInteractor()->Enable();
    m_planeWidget[i]->On();
    m_planeWidget[i]->InteractionOn();
  }

  ppVTKOGLWidgets[3]->GetInteractor()->Enable();


  vtkRenderWindowInteractor *iren = this->ui->view3->GetInteractor();
  iren->ReInitialize();


  this->Render();

  this->ui->view0->show();
  this->ui->view1->show();
  this->ui->view2->show();

  this->ui->view3->show();

  this->ResetViews();



  this->resliceMode(1);
}


DataManager::~DataManager() {
  delete ui;
}

void DataManager::resliceMode(int mode) {
  for (int i = 0; i < 3; i++) {
    m_riw[i]->SetResliceMode(mode ? 1 : 0);
    m_riw[i]->GetRenderer()->ResetCamera();
    m_riw[i]->Render();
  }
}
