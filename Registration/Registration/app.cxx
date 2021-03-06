// TODO: Fix callback such that US doesn't rescale 3D
//       Possibility to follow MR

// Overload vtkInteractorStyleImage::OnMouseWheelForward and post events to
// other modality
// or imageStyle->AddObserver(vtkCommand::MouseMoveEvent, callback);


// renderer->ResetCamera();
// renderer->GetActiveCamera()->Zoom(1.5);

/*
  vtkContourWidget contourWidget = new vtkContourWidget();
  contourWidget.SetInteractor(panel.getRenWin().getIren());
  contourWidget.FollowCursorOn();

  vtkOrientedGlyphContourRepresentation rep = new vtkOrientedGlyphContourRepresentation();
  contourWidget.SetRepresentation(rep);

  vtkImageActorPointPlacer placer = new vtkImageActorPointPlacer();
  placer.SetImageActor(panel.getImageViewer().GetImageActor());
  rep.SetPointPlacer(placer);

  contourWidget.EnabledOn();
  contourWidget.ProcessEventsOn();

  What this point placer does is to constrain the contour to the
  plane defined by the ImageActor. However, given that the same image
  actor is used for multiple slices, when you change the slice, the
  position of the image actor effectively changes and the contour
  updates itself to lie on the new plane.

  What you need is to use a vtkBoundedPlanePointPlacer with the bounding
  planes de-lienating the bounds of the image actor when the contour is
  defined. That way it will not change when you change the slice.

  Also to manage visibility of the contour (the contour will be visible
  for all slices in front of the defined slice), you could subclass the
  contour representation, insert a clipper that clips the polydata based
  on the bounding planes, that way, visibility is implicitly managed by
  the representation.

  vtkBoundedPlanePointPlacer placer = new vtkBoundedPlanePointPlacer();
          placer.SetProjectionNormalToZAxis();
          placer.SetProjectionPosition(panel.getImageViewer().GetImageActor().GetCenter()[2]);
          rep.SetPointPlacer(placer);

  This is almost right.  The contour now appears on the plane that I
  drew it on and the plane above it and that is it.  I just want the
  contour on the plane I drew it on.  I tried playing around with
  placer.SetPixelTolerance() but that did not make a difference.

  // Solution
  placer.RemoveAllBoundingPlanes();
  placer.SetProjectionNormalToZAxis();

  imageActor.GetBounds(bounds);
  plane = new vtkPLane()
      plane->SetOrigin( bounds[0], bounds[2], bounds[4] );
  plane->SetNormal( 0.0, 0.0, 1.0 );
  this->Placer->AddBoundingPlane( plane );

  plane2 = vtkPlane::New();
  plane2->SetOrigin( bounds[1], bounds[3], bounds[5] );
  plane2->SetNormal( 0.0, 0.0, -1.0 );
  placer.AddBoundingPlane( plane2 );
*/


#ifndef SQUARE
# define SQUARE(z) ((z) * (z))
#endif

#include <Registration/ui_app.h>
#include <Registration/reslicecallback.h>
#include <Registration/seedcallback.h>
#include <Registration/utils.hpp>
#include <Registration/config.h>

#include <QSettings>
#include <QDebug>

#include <cstdlib>
#include <iostream>

#include <Registration/app.hpp>
#include <Registration/filedialog.hpp>

#include <vtkAssemblyPath.h>

#include <vtkCamera.h>
#include <vtkCellPicker.h>
#include <vtkDICOMImageReader.h>
#include <vtkGenericOpenGLRenderWindow.h>
// #include <vtkGenericDataObjectReader.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkImageReader2.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkNamedColors.h>
#include <vtkMatrix4x4.h>
#include <vtkMetaImageReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkNrrdReader.h>

#include <vtkPlane.h>
#include <vtkPNGWriter.h>
#include <vtkPointData.h>

#include <vtkProperty.h>
#include <vtkProperty2D.h>

#include <vtkPropAssembly.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindowInteractor.h>

#include <vtkResliceCursor.h>
#include <vtkResliceCursorActor.h>
#include <vtkResliceCursorLineRepresentation.h>
#include <vtkResliceCursorPolyDataAlgorithm.h>
#include <vtkResliceCursorWidget.h>

#include <vtkResliceImageViewer.h>
#include <vtkResliceImageViewerMeasurements.h>



#include <vtkWindowToImageFilter.h>


App::~App() {}

void App::keyPressEvent(QKeyEvent *event) {
  QModelIndexList selectedRows = this->ui->tableView->selectionModel()->selectedRows();

#if 0
  // at least one entire row selected
  if(!selectedRows.isEmpty()){
    if(event->key() == Qt::Key_Insert)
      this->ui->tableView->model()->insertRows(selectedRows.at(0).row(),
                                               selectedRows.size());
    else if(event->key() == Qt::Key_Delete)
      this->ui->tableView->model()->removeRows(selectedRows.at(0).row(),
                                               selectedRows.size());
  }
#else
  // at least one cell selected
  if(!ui->tableView->selectionModel()->selectedIndexes().isEmpty()){
#if 0
    if(event->key() == Qt::Key_Delete){
      foreach (QModelIndex index, selectedIndexes())
          model()->setData(index, QString());
    }
#endif
    /* else */ if(event->matches(QKeySequence::Copy)){
      QString text;
      QItemSelectionRange range = ui->tableView->selectionModel()->selection().first();
      for (auto i = range.top() ; i <= range.bottom(); ++i) {
        QStringList rowContents;
        for (auto j = range.left(); j <= range.right(); ++j)
          rowContents << ui->tableView->model()->index(i,j).data().toString();
        text += rowContents.join("\t");
        text += "\n";
      }
      QApplication::clipboard()->setText(text);
    }
#if 0
    else if(event->matches(QKeySequence::Paste)) {
      QString text = QApplication::clipboard()->text();
      QStringList rowContents = text.split("\n", QString::SkipEmptyParts);

      QModelIndex initIndex = selectedIndexes().at(0);
      auto initRow = initIndex.row();
      auto initCol = initIndex.column();

      for (auto i = 0; i < rowContents.size(); ++i) {
        QStringList columnContents = rowContents.at(i).split("\t");
        for (auto j = 0; j < columnContents.size(); ++j) {
          model()->setData(model()->index(
              initRow + i, initCol + j), columnContents.at(j));
        }
      }
    }
    else
      QTableView::keyPressEvent(event);
#endif
  }
#endif
}

void App::onRegStartClick() {
  ui->btnReg->setEnabled(false);
  ui->progressBar->setValue(0);
  stopped = false;
  regrunner =
      new RegRunner(this, data, nullptr, nullptr,
                    &retval, &stopped);

  regrunner->setAutoDelete(false);
  QThreadPool::globalInstance()->start(regrunner);

  regDelegate = &App::onCancelClick;
  ui->btnReg->setText("Cancel");
  ui->btnReg->setEnabled(true);

  checkIfDone();
}

void App::onSegStartInView(int iView) {
    // Read again first seed in case you have moved it
    // TODO: Update data[seedX] upon moving seeds

    double origin[3];
    double spacing[3];
    int imageDims[3];

    vtkImageData* pImage = m_riw[iView]->GetInput();

    pImage->GetSpacing(spacing);
    pImage->GetOrigin(origin);
    pImage->GetDimensions(imageDims);

    auto rep = m_seeds[iView]->GetSeedRepresentation();

    if (rep->GetNumberOfSeeds() > 0) {
        double pos[3];
        rep->GetSeedWorldPosition(0, pos);
        int ix = (pos[0] - origin[0]) / spacing[0];
        int iy = (pos[1] - origin[1]) / spacing[1];
        int iz = (pos[2] - origin[2]) / spacing[2];

        data["seedX"] = ix;
        data["seedY"] = iy;
        data["seedZ"] = iz;
    }

    //  data["finput"] = QString("/home/jmh/bkmedical/data/CT/CT-Abdomen.mhd");
    //  ui->btnSeg->setEnabled(false);

    ui->segProgressBar->setValue(0);
    segStopped = false;
    segRunner =
        new SegRunner(this, data, pImage, m_segmentation, &retval, &segStopped);
    segRunner->setAutoDelete(false);
    QThreadPool::globalInstance()->start(segRunner);

    // segDelegate = &App::onCancelClick;
    ui->btnSeg->setText("Cancel");
    ui->btnSeg->setEnabled(true);

    checkIfSegDone();
}

void App::onSegStartClick() {
    return this->onSegStartInView(1);
}

void App::onSurfStartClick() {
  ClearSeedsInView1();

  ui->btnSurf->setEnabled(false);
  ui->segProgressBar->setValue(0);
  surfStopped = false;

  // TODO: Read sliders
  // data["low"] = this->thresholdsSlider->GetLowerValue();
  // data["high"] = this->thresholdsSlider->GetUpperValue();

#ifdef INTEGRATED_SURFACING

  int imageDims[3];
  m_segmentation->GetDimensions(imageDims);

  // Display dimensions
  std::cout << "Segmentation dimensions: " << imageDims[0] << " "
      << imageDims[1] << " "
      << imageDims[2] << std::endl;

  surfStopped = false;
  surfRunner =
      new SurfRunner(this, data, m_polydata, m_segmentation, &retval, &surfStopped);
#else
  surfRunner =
      new SurfRunner(this, data, nullptr, nullptr, &retval, &surfStopped);
#endif

  connect(this->surfRunner, SIGNAL(Done()), this, SLOT(updateSurface()));

  surfRunner->setAutoDelete(false);
  QThreadPool::globalInstance()->start(surfRunner);

  // segDelegate = &App::onCancelClick;
  ui->btnSurf->setText("Cancel");
  ui->btnSurf->setEnabled(true);

  checkIfSurfDone();
}



void App::segmSliderChanged(int value) {
  qDebug() << value;
}

// Works!!!
void App::onThreeClicked() {
  auto CTcursor = this->m_riw[0]->GetResliceCursor();
  auto UScursor = this->m_riw_us[0]->GetResliceCursor();
  for (int i = 0 ; i < 3 ; i++) {
    auto normal = CTcursor->GetPlane(i)->GetNormal();
    UScursor->GetPlane(i)->SetNormal(normal);
    auto origin = CTcursor->GetPlane(i)->GetOrigin();
    UScursor->GetPlane(i)->SetOrigin(origin);

  }
}
void App::onApplyPresetClick() {
  qDebug() << "preset";
  // CT Liver preset
  double window = 200;
  double level = 100; // [0,200]
  //window = -200;

  int index = this->ui->cboxPreset->currentIndex();

  if (index == 0) {
    double range[2];
    if (m_riw[0]) {
      m_riw[0]->GetInput()->GetScalarRange(range);
      window = range[1]-range[0];
      level = (range[0]+range[1])/2.0;
    }
  }
  for (size_t i = 0; i < 3 ; i++) {
    if (m_riw[i]) {
      m_riw[i]->SetColorWindow(window);
      m_riw[i]->SetColorLevel(level);
      m_riw[i]->Render();
    }
  }
}

void App::onSurfClick() {
  (this->*surfDelegate)();
}

void App::onSegClick() {
  (this->*segDelegate)();
}

void App::onRegClick() {
  (this->*regDelegate)();
}

void App::onCancelClick() {
  printf("Canceling\n");
  // Do something that interrupts registration
  regDelegate = &App::onRegClick;
}

void App::onSegCancelClick() {
  printf("Canceling\n");
  // Do something that interrupts registration
  segDelegate = &App::onSegClick;
}

void App::onSurfCancelClick() {
  printf("Canceling\n");
  // Do something that interrupts registration
  surfDelegate = &App::onSurfClick;
}

void App::updateProgressBar(int progressPercent) {
  ui->progressBar->setValue(progressPercent);
}

void App::updateSegProgressBar(int progressPercent) {
  ui->segProgressBar->setValue(progressPercent);
}

void App::checkIfSegDone() {
  // What if cancel happens before
  if (QThreadPool::globalInstance()->activeThreadCount()) {
    QTimer::singleShot(100, this, SLOT(checkIfSegDone()));
  } else {
    if (!segStopped) {
      segStopped = true;
      updateSegChildWidgets();
      // Update widget with results


      segDelegate = &App::onSegStartClick;
    }
    if (segRunner) {
      delete segRunner;
      segRunner = nullptr;
    }
    // Should only be set if segStopped = false
    this->ui->btnSurf->setEnabled(true);
  }
}

void App::checkIfSurfDone() {
  // What if cancel happens before
  if (QThreadPool::globalInstance()->activeThreadCount()) {
    QTimer::singleShot(100, this, SLOT(checkIfSurfDone()));
  } else {
    if (!surfStopped) {
      surfStopped = true;
      updateSurfChildWidgets();
      // Update widget with results



      surfDelegate = &App::onSurfStartClick;
    }
    if (surfRunner) {
      disconnect(this->surfRunner, SIGNAL(Done()), 0, 0);
      delete surfRunner;
      surfRunner = nullptr;
    }
  }
}


void App::updateSurface() {
#ifdef INTEGRATED_SURFACING

    std::cout << m_polydata->GetNumberOfCells() << std::endl;
    // TEST: Update 3D vessels here
    vtkSmartPointer<vtkPolyDataMapper> mapper =
        vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(m_polydata);

    if (m_vessels) {
        m_planeWidget[0]->GetDefaultRenderer()->RemoveActor(m_vessels);
        m_vessels = nullptr;
    }

    m_vessels =
        vtkSmartPointer<vtkActor>::New();

    m_vessels->SetMapper(mapper);
    auto prop = m_vessels->GetProperty();
    vtkSmartPointer<vtkNamedColors> namedColors =
        vtkSmartPointer<vtkNamedColors>::New();

    prop->SetColor(namedColors->GetColor3d("Red").GetData());

    m_planeWidget[0]->GetDefaultRenderer()->AddActor(m_vessels);
    this->ui->mrView3D->GetRenderWindow()->Render();
#endif
}

void App::checkIfDone() {
  // What if cancel happens before
  if (QThreadPool::globalInstance()->activeThreadCount()) {
    QTimer::singleShot(100, this, SLOT(checkIfDone()));
  } else {
    if (!stopped) {
      stopped = true;
      updateChildWidgets();
      // Update widget with results
      regDelegate = &App::onRegStartClick;
    }
    if (regrunner) {
      delete regrunner;
      regrunner = nullptr;
    }
  }
}

void App::updateChildWidgets() {
  if (stopped) {
    ui->btnReg->setEnabled(false);
    ui->btnReg->setText("R&egister");
    ui->btnReg->setEnabled(true);
    ui->progressBar->reset();
  } else {
    ui->btnReg->setEnabled(false);
    ui->progressBar->reset();
  }
}

void App::updateSegChildWidgets() {
  if (segStopped) {
    ui->btnSeg->setEnabled(false);
    ui->btnSeg->setText("S&eg");
    ui->btnSeg->setEnabled(true);
    ui->segProgressBar->reset();
  } else {
    ui->btnSeg->setEnabled(false);
    ui->segProgressBar->reset();
  }
}

void App::updateSurfChildWidgets() {
  if (surfStopped) {
    ui->btnSurf->setEnabled(false);
    ui->btnSurf->setText("M&esh");
    ui->btnSurf->setEnabled(true);
    ui->segProgressBar->reset();
  } else {
    ui->btnSurf->setEnabled(false);
    ui->segProgressBar->reset();
  }
}



void App::SetupUI() {
  this->ui = new Ui_Registration;
  this->ui->setupUi(this);

  this->ui->sliderZoom->setMinimum(50);
  this->ui->sliderZoom->setRange(0, 800);
  this->ui->sliderZoom->setSingleStep(10);
  this->ui->sliderZoom->setValue(100);
  this->ui->cboxPlane->setCurrentIndex(2);
  this->ui->cboxPreset->setCurrentIndex(0);

  this->thresholdsSlider =
    new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, this);
  this->thresholdsSlider->SetRange(100, 600);

  // TODO: Add labels
  this->m_pWidget = new QWidget(this);
  this->m_sliderLayout = new QHBoxLayout(this->m_pWidget);
  this->m_sliderLblLow = new QLineEdit(this);
  this->m_sliderLblHigh = new QLineEdit(this);
  this->m_sliderLblLow->setMaximumWidth(100);
  this->m_sliderLblHigh->setMaximumWidth(100);
  this->m_sliderLayout->addWidget(this->thresholdsSlider);
  this->m_sliderLayout->addWidget(this->m_sliderLblLow);
  this->m_sliderLayout->addWidget(this->m_sliderLblHigh);

  m_pWidget->setLayout(this->m_sliderLayout);

  this->ui->segmVertLayout->insertWidget(0, m_pWidget);


  this->ui->btnSeg->setEnabled(false);
  this->ui->btnSurf->setEnabled(false);
  this->ui->btnAddSeeds->setEnabled(false);

  this->transModel = new TransformModel(this);

  this->ui->tableView->setModel(this->transModel);

  ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  //  this->ui->tableView->setVisible(false);

  this->ui->tableView->setVisible(true);
  this->ui->tableView->resizeColumnsToContents();
  this->ui->tableView->resizeRowsToContents();

  //  this->ui->tableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
}

void App::dumpImages() {
  int index = this->ui->cboxPlane->currentIndex();
  this->dumpImageBackBuffers(index);
}
void App::dumpImageBackBuffers(int index) {
  assert(m_riw[index]);
  assert(m_riw_us[index]);

  vtkRenderWindow* renderWindow0 = m_riw[index]->GetRenderWindow();
  vtkRenderWindow* renderWindow1 = m_riw_us[index]->GetRenderWindow();

  renderWindow0->Render();
  renderWindow1->Render();

  // Screenshot
  vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter0 =
      vtkSmartPointer<vtkWindowToImageFilter>::New();
  vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter1 =
      vtkSmartPointer<vtkWindowToImageFilter>::New();
  windowToImageFilter0->SetInput(renderWindow0);
  windowToImageFilter1->SetInput(renderWindow1);

#if VTK_MAJOR_VERSION >= 8 || VTK_MAJOR_VERSION == 8 && VTK_MINOR_VERSION >= 90
  windowToImageFilter0->SetScale(1); // image quality
  windowToImageFilter1->SetScale(1); // image quality
#else
  windowToImageFilter0->SetMagnification(1); //image quality
  windowToImageFilter1->SetMagnification(1); //image quality
#endif
  windowToImageFilter0->SetInputBufferTypeToRGBA();
  windowToImageFilter1->SetInputBufferTypeToRGBA();

  int oldSB0 = renderWindow0->GetSwapBuffers();
  int oldSB1 = renderWindow1->GetSwapBuffers();

  renderWindow0->SwapBuffersOff();
  renderWindow1->SwapBuffersOff();

  windowToImageFilter0->ReadFrontBufferOff(); // read from the back buffer
  windowToImageFilter1->ReadFrontBufferOff(); // read from the back buffer
  windowToImageFilter0->Update();
  windowToImageFilter1->Update();

  renderWindow0->SetSwapBuffers(oldSB0);
  renderWindow1->SetSwapBuffers(oldSB1);

  int *size = renderWindow0->GetSize();
  std::cout << "size: " << size[0] << "x" << size[1] << std::endl;

  vtkSmartPointer<vtkPNGWriter> writer =
      vtkSmartPointer<vtkPNGWriter>::New();
  std::string filename = string_format("screenshotMR%02d.png", index);
  writer->SetFileName(filename.c_str());
  writer->SetInputConnection(windowToImageFilter0->GetOutputPort());
  writer->Write();

  data.insert("mr", filename.c_str());


  filename = string_format("screenshotUS%02d.png", index);

  writer->SetFileName(filename.c_str());
  writer->SetInputConnection(windowToImageFilter1->GetOutputPort());
  writer->Write();

  data.insert("us", filename.c_str());

  renderWindow0->Render();
  renderWindow1->Render();
}

void App::dumpImageOffscreen() {

  // Offscreen

  // Acquire data from framebuffer (or use mrView2)
  vtkRenderWindow* renderWindow = m_riw[2]->GetRenderWindow();
  vtkOpenGLRenderWindow* glRenderWindow =
      vtkOpenGLRenderWindow::SafeDownCast(renderWindow);

  // Offscreen rendering
  if (!glRenderWindow->SetUseOffScreenBuffers(true)) {
    glRenderWindow->DebugOn();
    glRenderWindow->SetUseOffScreenBuffers(true);
    glRenderWindow->DebugOff();
    std::cerr << "Unable to create a hardware frame buffer, the graphic board "
        "or driver can be too old:\n"
              << glRenderWindow->ReportCapabilities() << std::endl;
    exit(-1);
  }


  renderWindow->Render();

  vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter =
      vtkSmartPointer<vtkWindowToImageFilter>::New();
  windowToImageFilter->SetInput(renderWindow);
  windowToImageFilter->Update();
  
  vtkSmartPointer<vtkPNGWriter> writer = 
    vtkSmartPointer<vtkPNGWriter>::New();
  writer->SetFileName("screenshotOffscreen.png");
  writer->SetInputConnection(windowToImageFilter->GetOutputPort());
  writer->Write();

  glRenderWindow->SetUseOffScreenBuffers(false);

  renderWindow->Render();
}

void App::setupMR() {
  // Create reslice image widgets
  QVTKOpenGLWidget *ppVTKOGLWidgets[4] = {
    this->ui->mrView0,
    this->ui->mrView1,
    this->ui->mrView2,
    this->ui->mrView3D
  };

  for (size_t i = 0; i < 3; i++) {
    m_riw[i] = vtkSmartPointer<vtkResliceImageViewer>::New();
    vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
    m_riw[i]->SetRenderWindow(renderWindow);
    // TODO: Figure out how to replace RIW's InputConnection with
    // a vtkImageBlend of this and an overlay
  }
  for (size_t i = 0; i < 3; i++) {
    ppVTKOGLWidgets[i]->SetRenderWindow(m_riw[i]->GetRenderWindow());
    m_riw[i]->SetupInteractor(
      ppVTKOGLWidgets[i]->GetRenderWindow()->GetInteractor());

    // Disable interactor until data are present
    ppVTKOGLWidgets[i]->GetInteractor()->Disable();
  }

  // Disable interactor
  ppVTKOGLWidgets[3]->GetInteractor()->Disable();

  // Initialize dummy data for reslice image widgets
  this->m_dummy = vtkSmartPointer<vtkImageData>::New();


  // Setup cursors and orientation of reslice image widgets
  for (int i = 0; i < 3; i++) {
    vtkResliceCursorLineRepresentation *rep =
      vtkResliceCursorLineRepresentation::SafeDownCast(
        m_riw[i]->GetResliceCursorWidget()->GetRepresentation());

    if (i==2) {
      // Must set vtkResliceCursorActor on vtkResliceCursorLineRepresentation

      // Was uncommented
      // rep->Highlight(1);//GetImageActor()->SetVisibility(0);

      // rep->VisibilityOff(); // Works as off
      // Consider calling InitPathTraversal + and traverse props
      // rep->GetResliceCursorActor()->SetVisibility(0);
    }

    // Make all reslice image viewers share the same reslice cursor object.
    m_riw[i]->SetResliceCursor(m_riw[0]->GetResliceCursor());

    rep->GetResliceCursorActor()->
    GetCursorAlgorithm()->SetReslicePlaneNormal(i);

    m_riw[i]->SetSliceOrientation(i);
    m_riw[i]->SetResliceModeToAxisAligned();
    //m_riw[i]->SetResliceModeToOblique();
    // Set empty data - otherwise we cannot enable widgets

    // Could this be an input connection????
    m_riw[i]->SetInputData(this->m_dummy);
  }

  // Create 3D viewer
  vtkSmartPointer<vtkCellPicker> picker =
    vtkSmartPointer<vtkCellPicker>::New();
  picker->SetTolerance(0.005);

  vtkSmartPointer<vtkProperty> ipwProp =
    vtkSmartPointer<vtkProperty>::New();


  vtkSmartPointer<vtkRenderer> ren =
    vtkSmartPointer<vtkRenderer>::New();

  vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;

  this->ui->mrView3D->SetRenderWindow(renderWindow);

  // Why both a GL and conventional -> edges are CPU rendered
  this->ui->mrView3D->GetRenderWindow()->AddRenderer(ren);

  vtkRenderWindowInteractor *iren = this->ui->mrView3D->GetInteractor();

  for (int i = 0; i < 3; i++) {
    m_planeWidget[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();

    // No need to add as an vtkActor to the renderer - it is a widget
    m_planeWidget[i]->SetInteractor(iren);
    m_planeWidget[i]->SetPicker(picker); // PICKER
    m_planeWidget[i]->RestrictPlaneToVolumeOn();
    double color[3] = { 0.0, 0.0, 0.0 };
    color[i] = 1.0;

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
  }

  // Establish callbacks
  cbk =
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
      m_riw[i]->GetResliceCursorWidget()->
      GetResliceCursorRepresentation()->GetColorMap());

    // Buffers are updated when resizing.
    // Otherwise uninitialized memory is shown.
    m_riw[i]->GetInteractor()->Enable();
  }

  this->ui->mrView0->show();
  this->ui->mrView1->show();
  this->ui->mrView2->show();

  this->Render();
}

void App::setupUS() {
  // Create reslice image widgets
  // TODO: usView2 is working

  QVTKOpenGLWidget *ppVTKOGLWidgets[4] = {
    this->ui->usView0,
    this->ui->usView1,
    this->ui->usView2,
  };

  for (size_t i = 0; i < 3; i++) {
    m_riw_us[i] = vtkSmartPointer<vtkResliceImageViewer>::New();
    vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
    m_riw_us[i]->SetRenderWindow(renderWindow);
  }
  for (size_t i = 0; i < 3; i++) {
    ppVTKOGLWidgets[i]->SetRenderWindow(m_riw_us[i]->GetRenderWindow());
    m_riw_us[i]->SetupInteractor(
      ppVTKOGLWidgets[i]->GetRenderWindow()->GetInteractor());

    // Disable interactor until data are present
    ppVTKOGLWidgets[i]->GetInteractor()->Disable();
  }

  // Initialize dummy data for reslice image widgets
  this->m_dummy1 = vtkSmartPointer<vtkImageData>::New();

  // Setup cursors and orientation of reslice image widgets
  for (int i = 0; i < 3; i++) {
    vtkResliceCursorLineRepresentation *rep =
      vtkResliceCursorLineRepresentation::SafeDownCast(
        m_riw_us[i]->GetResliceCursorWidget()->GetRepresentation());

    // Make all reslice image viewers share the same reslice cursor object.
    m_riw_us[i]->SetResliceCursor(m_riw_us[0]->GetResliceCursor());

    rep->GetResliceCursorActor()->
    GetCursorAlgorithm()->SetReslicePlaneNormal(i);

    m_riw_us[i]->SetSliceOrientation(i);
    m_riw_us[i]->SetResliceModeToAxisAligned();
    //m_riw_us[i]->SetResliceModeToOblique();

    // Set empty data - otherwise we cannot enable widgets
    m_riw_us[i]->SetInputData(this->m_dummy1);
  }

  for (int i = 0; i < 3; i++) {
    double color[3] = { 0, 0, 0 };
    color[i] = 1;

    color[0] /= 4.0;
    color[1] /= 4.0;
    color[2] /= 4.0;
    m_riw_us[i]->GetRenderer()->SetBackground(color);
  }

  // Establish callbacks - needs to impact MR as well
  if (!cbk) {
    cbk = vtkSmartPointer<vtkResliceCursorCallback>::New();
    qDebug() << "should never happen";
  }

  for (int i = 0; i < 3; i++) {
    // Can we have multiple callbacks????
    //cbk->IPW[i] = nullptr;//m_planeWidget[i];
    cbk->USRCW[i] = m_riw_us[i]->GetResliceCursorWidget();
    m_riw_us[i]->GetResliceCursorWidget()->AddObserver(
      vtkResliceCursorWidget::ResliceAxesChangedEvent, cbk);
    m_riw_us[i]->GetResliceCursorWidget()->AddObserver(
      vtkResliceCursorWidget::WindowLevelEvent, cbk);
    m_riw_us[i]->GetResliceCursorWidget()->AddObserver(
      vtkResliceCursorWidget::ResliceThicknessChangedEvent, cbk);
    m_riw_us[i]->GetResliceCursorWidget()->AddObserver(
      vtkResliceCursorWidget::ResetCursorEvent, cbk);
    m_riw_us[i]->GetInteractorStyle()->AddObserver(
      vtkCommand::WindowLevelEvent, cbk);

    // Add obserser for zoom

    // Make them all share the same color map.
    m_riw_us[i]->SetLookupTable(m_riw_us[0]->GetLookupTable());


    // Buffers are updated when resizing.
    // Otherwise uninitialized memory is shown.
    m_riw_us[i]->GetInteractor()->Enable();
  }

  this->ui->usView0->show();
  this->ui->usView1->show();
  this->ui->usView2->show();

  this->Render();
}

void App::PopulateMenus() {
  connect(this->ui->actionExit, SIGNAL(triggered()),
          this, SLOT(slotExit()));
  connect(this->ui->actionOpenMR, SIGNAL(triggered()),
          this, SLOT(onLoadMRClicked()));
  connect(this->ui->actionOpenUS, SIGNAL(triggered()),
          this, SLOT(onLoadUSClicked()));
  connect(this->ui->actionOpenVessels, SIGNAL(triggered()),
          this, SLOT(onLoadVesselsClicked()));
  connect(ui->btnReg, &QPushButton::clicked,
          this, &App::onRegClick);
  connect(ui->sliderZoom, &QSlider::valueChanged,
          this, &App::setZoom);
  connect(ui->btnOne, &QPushButton::clicked,
          this, &App::dumpImages);
  connect(ui->btnTwo, &QPushButton::clicked,
          this, &App::dumpImageOffscreen);

  connect(ui->btnSeg, &QPushButton::clicked,
          this, &App::onSegClick);

  connect(ui->btnSurf, &QPushButton::clicked,
          this, &App::onSurfClick);

  connect(ui->btnReset, &QPushButton::clicked,
          this, &App::ResetViews);

  connect(ui->btnPreset, &QPushButton::clicked,
          this, &App::onApplyPresetClick);
  connect(ui->btnThree, &QPushButton::clicked,
          this, &App::onThreeClicked);
  connect(ui->horizontalSlider, &QSlider::valueChanged, this, &App::segmSliderChanged);

  connect(ui->btnAddSeeds, SIGNAL(pressed()), this, SLOT(AddSeedsToView1()));
  connect(ui->btnClearSeeds, SIGNAL(pressed()), this, SLOT(ClearSeedsInView1()));

  connect(this->thresholdsSlider, SIGNAL(lowerValueChanged(int)), this, SLOT(SliderLow(int)));

  connect(this->thresholdsSlider, SIGNAL(upperValueChanged(int)), this, SLOT(SliderHigh(int)));

  connect(ui->cboxPlane, SIGNAL(currentIndexChanged(int)),
      SLOT(TransformationUpdated(int)));
  // void updateSurface();
  
}

void App::SliderLow(int value) {
  data["low"] = value;
  this->m_sliderLblLow->setText(QString::number(value));
}

void App::SliderHigh(int value) {
  data["high"] = value;
  this->m_sliderLblHigh->setText(QString::number(value));
}

void App::AddSeedsToView1() {
  return this->AddSeedsToView(1);
}

void App::setZoom(int zoom) {
  double dZoom = double(zoom)/100.0;
  for (int i = 0; i < 3; i++) {
    m_riw[i]->GetRenderer()->ResetCamera();
    m_riw[i]->GetRenderer()->GetActiveCamera()->Zoom(dZoom);
    m_riw[i]->Render();
  }
  for (int i = 0; i < 3; i++) {
    m_riw_us[i]->GetRenderer()->ResetCamera();
    m_riw_us[i]->GetRenderer()->GetActiveCamera()->Zoom(dZoom);
    m_riw_us[i]->Render();
  }
}

void App::onLoadMRClicked() {
  const QString DEFAULT_DIR_KEY("RegistrationDefaultDir");
  QSettings MySettings;// = QSettings(;

  QString selectedDirectory;

  FileDialog w;
  qDebug() << "default_dir:" << MySettings.value(DEFAULT_DIR_KEY).toString();
  w.setDirectory(MySettings.value(DEFAULT_DIR_KEY).toString());

  int nMode = w.exec();
  QStringList fnames = w.selectedFiles();

  if (nMode != 0 && fnames.size() != 0) {
    qDebug() << fnames;

    this->FileLoad(fnames[0]);

    int i = fnames.size();
    QString selectedFileOrDirectory = fnames.at(i-1);

    QFileInfo fi = QFileInfo(selectedFileOrDirectory);

    if (fi.isDir()) {
      QDir currentDir(selectedFileOrDirectory);
      currentDir.cdUp(); // Maybe necessary
      MySettings.setValue(DEFAULT_DIR_KEY,
                          currentDir.absolutePath());
      qDebug() << currentDir.absolutePath();
    } else {
      QDir currentDir = QFileInfo(selectedFileOrDirectory).absoluteDir();
      qDebug() << currentDir.absolutePath();
      MySettings.setValue(DEFAULT_DIR_KEY,
                          currentDir.absolutePath());
    }
  }
}

void App::onLoadVesselsClicked() {

  const QString DEFAULT_DIR_KEY("RegistrationDefaultDir");
  QSettings MySettings;

  QString selectedDirectory;

  FileDialog w;
  w.setDirectory(MySettings.value(DEFAULT_DIR_KEY).toString());

  int nMode = w.exec();
  QStringList fnames = w.selectedFiles();

  if (nMode != 0 && fnames.size() != 0) {
    QString files = fnames[0];
    QDir directory = QDir(files);


    if (directory.exists()) {
      return;
    } else {
      QFileInfo info(files);
      if (info.completeSuffix() == QLatin1String("vtp")) {
        vtkSmartPointer<vtkXMLPolyDataReader> reader =
            vtkSmartPointer<vtkXMLPolyDataReader>::New();

        reader->SetFileName(files.toUtf8().constData());
        reader->Update();

        vtkSmartPointer<vtkPolyDataMapper> mapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(reader->GetOutputPort());

        if (m_vessels) {
          m_planeWidget[0]->GetDefaultRenderer()->RemoveActor(m_vessels);
          m_vessels = nullptr;
        }

        m_vessels =
            vtkSmartPointer<vtkActor>::New();

        m_vessels->SetMapper(mapper);
        auto prop = m_vessels->GetProperty();
        vtkSmartPointer<vtkNamedColors> namedColors =
            vtkSmartPointer<vtkNamedColors>::New();

        prop->SetColor(namedColors->GetColor3d("Red").GetData());

        m_planeWidget[0]->GetDefaultRenderer()->AddActor(m_vessels);
        this->ui->mrView3D->GetRenderWindow()->Render();
      } else if (info.completeSuffix() == QLatin1String("vtk")) {
#if 0
          vtkSmartPointer<vtkGenericDataObjectReader> reader =
              vtkSmartPointer<vtkGenericDataObjectReader>::New();
          reader->SetFileName(files.toUtf8().constData());
          reader->Update();

          vtkSmartPointer<vtkPolyDataMapper> mapper =
              vtkSmartPointer<vtkPolyDataMapper>::New();
          mapper->SetInputConnection(reader->GetOutputPort());

          if (m_vessels) {
              m_planeWidget[0]->GetDefaultRenderer()->RemoveActor(m_vessels);
              m_vessels = nullptr;
          }

          m_vessels =
              vtkSmartPointer<vtkActor>::New();

          m_vessels->SetMapper(mapper);
          auto prop = m_vessels->GetProperty();
          vtkSmartPointer<vtkNamedColors> namedColors =
              vtkSmartPointer<vtkNamedColors>::New();

          prop->SetColor(namedColors->GetColor3d("Red").GetData());

          m_planeWidget[0]->GetDefaultRenderer()->AddActor(m_vessels);
          this->ui->mrView3D->GetRenderWindow()->Render();
#endif
      } else {
        return;
      }
    }
  }
}

void App::onLoadUSClicked() {
  const QString DEFAULT_DIR_KEY("RegistrationDefaultDir");
  QSettings MySettings;

  QString selectedDirectory;

  FileDialog w;
  qDebug() << "default_dir:" << MySettings.value(DEFAULT_DIR_KEY).toString();
  w.setDirectory(MySettings.value(DEFAULT_DIR_KEY).toString());

  int nMode = w.exec();
  QStringList fnames = w.selectedFiles();

  if (nMode != 0 && fnames.size() != 0) {
    qDebug() << fnames;

    this->FileLoad(fnames[0], 1);

    int i = fnames.size();
    QString selectedFileOrDirectory = fnames.at(i-1);

    QFileInfo fi = QFileInfo(selectedFileOrDirectory);

    if (fi.isDir()) {
      QDir currentDir(selectedFileOrDirectory);
      currentDir.cdUp(); // Maybe necessary
      MySettings.setValue(DEFAULT_DIR_KEY,
                          currentDir.absolutePath());
      qDebug() << currentDir.absolutePath();
    } else {
      QDir currentDir = QFileInfo(selectedFileOrDirectory).absoluteDir();
      qDebug() << currentDir.absolutePath();
      MySettings.setValue(DEFAULT_DIR_KEY,
                          currentDir.absolutePath());
    }
  }
}


App::App(int argc, char* argv[]) {
  regrunner = nullptr;
  m_riw[0] = m_riw[1] = m_riw[2] = nullptr;
  m_planeWidget[0] = m_planeWidget[1] = m_planeWidget[2] = nullptr;

  m_riw_us[0] = m_riw_us[1] = m_riw_us[2] = nullptr;

  m_seeds[0] = m_seeds[1] = m_seeds[2] = nullptr;

#ifdef INTEGRATED_SURFACING
  m_segmentation = vtkSmartPointer<vtkImageData>::New();
  m_polydata = vtkSmartPointer<vtkPolyData>::New();
#endif

  m_vsum = 0.0f;
  m_vsum2 = 0.0f;
  //  m_vessels = nullptr;

  this->Connections = vtkSmartPointer<vtkEventQtSlotConnect>::New();

  this->ConnectionsTest = vtkSmartPointer<vtkEventQtSlotConnect>::New();

  this->SetupUI();
  this->setupMR();

  this->setupUS(); // This cause invalid extent

  this->PopulateMenus();

  regDelegate = &App::onRegStartClick;

  segDelegate = &App::onSegStartClick;

  surfDelegate = &App::onSurfStartClick;


}

void App::slotExit() {
  qApp->exit();
}

void App::resizeEvent(QResizeEvent* event) {
  //qDebug() << "resizeEvent";
  QMainWindow::resizeEvent(event);
}

void App::Render() {
  for (int i = 0; i < 3; i++) {
    m_riw[i]->Render();
  }

  for (int i = 0; i < 3; i++) {
    if (m_riw_us[i]) {
      m_riw_us[i]->Render();
    }
  }

  this->ui->mrView3D->GetRenderWindow()->Render();
}

void App::FileLoad(const QString &files, int type) {
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
  if (type == 0) {
    data["finput"] = QString(files.toUtf8().constData());
    return FileLoadMR(reader);
  }
  else {
    return FileLoadUS(reader);
  }
}

void App::FileLoadMR(const vtkSmartPointer<vtkImageReader2>& reader) {
  int imageDims[3];
  reader->GetOutput()->GetDimensions(imageDims);

  // Display dimensions
  std::cout << "Dimensions: " << imageDims[0] << " "
      << imageDims[1] << " "
      << imageDims[2] << std::endl;
  double spacing[3];
  reader->GetOutput()->GetSpacing(spacing);
  std::cout << "Spacing: " << spacing[0] << " "
      << spacing[1] << " "
      << spacing[2] << std::endl;

  double origin[3];
  reader->GetOutput()->GetOrigin(origin);
  std::cout << "Origin: " << origin[0] << " "
      << origin[1] << " "
      << origin[2] << std::endl;


  QVTKOpenGLWidget *ppVTKOGLWidgets[4] = {
    this->ui->mrView0,
    this->ui->mrView1,
    this->ui->mrView2,
    this->ui->mrView3D
  };

  // Disable renderers
  for (int i = 0; i < 3; i++) {
    m_riw[i]->GetInteractor()->EnableRenderOff();
  }
  this->ui->mrView3D->GetInteractor()->EnableRenderOff();

  for (int i = 0; i < 3; i++) {
    m_planeWidget[i]->Off();
  }

  for (int i = 0; i < 3; i++) {
    vtkResliceCursorLineRepresentation *rep =
      vtkResliceCursorLineRepresentation::SafeDownCast(
        m_riw[i]->GetResliceCursorWidget()->GetRepresentation());

    // Make all reslice image viewers share the same reslice cursor object.
    m_riw[i]->SetResliceCursor(m_riw[0]->GetResliceCursor());

    // Set normal for reslice planes
    rep->GetResliceCursorActor()->
    GetCursorAlgorithm()->SetReslicePlaneNormal(i);

    // Assign data and orientation
    m_riw[i]->SetInputData(reader->GetOutput());
    //m_riw[i]->SetInputConnection(reader->GetOutputPort());
    //m_riw[i]->Update();

    m_riw[i]->SetSliceOrientation(i);
    m_riw[i]->SetResliceModeToAxisAligned();
    //m_riw[i]->SetResliceModeToOblique();

  }

  vtkRenderWindowInteractor *iren = this->ui->mrView3D->GetInteractor();

  for (int i = 0; i < 3; i++) {
    m_planeWidget[i]->SetInteractor( iren );
    m_planeWidget[i]->RestrictPlaneToVolumeOn();  // Default

    m_planeWidget[i]->TextureInterpolateOn();
    m_planeWidget[i]->SetResliceInterpolateToLinear();
    m_planeWidget[i]->SetInputConnection(reader->GetOutputPort());
    m_planeWidget[i]->SetPlaneOrientation(i);
    m_planeWidget[i]->SetSliceIndex(imageDims[i]/2);
    m_planeWidget[i]->DisplayTextOn();

    // TODO: Call SetWindowLevel() using statistics from data
    m_planeWidget[i]->UpdatePlacement();
    m_planeWidget[i]->GetInteractor()->Enable();  // Important
    m_planeWidget[i]->On();
    m_planeWidget[i]->InteractionOn();
  }

  for (int i = 0; i < 3; i++) {
    m_riw[i]->GetRenderer()->ResetCamera();
    m_riw[i]->GetInteractor()->EnableRenderOn(); // calls this->RenderWindow->Render()
  }

  for (size_t i = 0; i < 3; i++) {
    ppVTKOGLWidgets[i]->GetInteractor()->Enable();
  }

  ppVTKOGLWidgets[3]->GetInteractor()->EnableRenderOn();

  // Reset camera for the renderer - otherwise it is set using dummy data
  m_planeWidget[0]->GetDefaultRenderer()->ResetCamera();

  // Interactor for other views are enabled through reslice image widgets
  this->ui->mrView3D->GetInteractor()->EnableRenderOn();

  ui->btnAddSeeds->setEnabled(true);

  // view0 - view3 are already visible

  this->ResetViews();  // renders everything

  this->resliceMode(1);  // Only renders again the 3 reslice image planes
}

void App::FileLoadUS(const vtkSmartPointer<vtkImageReader2>& reader) {

  int imageDims[3];
  reader->GetOutput()->GetDimensions(imageDims);

  QVTKOpenGLWidget *ppVTKOGLWidgets[4] = {
    this->ui->usView0,
    this->ui->usView1,
    this->ui->usView2,
  };

  // Disable renderers
  for (int i = 0; i < 3; i++) {
    m_riw_us[i]->GetInteractor()->EnableRenderOff();
  }

  for (int i = 0; i < 3; i++) {
    vtkResliceCursorLineRepresentation *rep =
      vtkResliceCursorLineRepresentation::SafeDownCast(
        m_riw_us[i]->GetResliceCursorWidget()->GetRepresentation());

    // Make all reslice image viewers share the same reslice cursor object.
    m_riw_us[i]->SetResliceCursor(m_riw_us[0]->GetResliceCursor());

    // Set normal for reslice planes
    rep->GetResliceCursorActor()->
    GetCursorAlgorithm()->SetReslicePlaneNormal(i);

    // Assign data and orientation
    m_riw_us[i]->SetInputData(reader->GetOutput());
    m_riw_us[i]->SetSliceOrientation(i);
    m_riw_us[i]->SetResliceModeToAxisAligned();
    //m_riw_us[i]->SetResliceModeToOblique();

  }

  for (int i = 0; i < 3; i++) {
    m_riw_us[i]->GetRenderer()->ResetCamera();
    m_riw_us[i]->GetInteractor()->EnableRenderOn(); // calls this->RenderWindow->Render()
  }

  for (size_t i = 0; i < 3; i++) {
    ppVTKOGLWidgets[i]->GetInteractor()->Enable();
  }

  this->ResetViews();  // renders everything

  this->resliceMode(1);  // Only renders again the 3 reslice image planes


}

void App::resliceMode(int mode) {
  for (int i = 0; i < 3; i++) {
    if (m_riw[i]) {
      m_riw[i]->SetResliceMode(mode ? 1 : 0);
      m_riw[i]->GetRenderer()->ResetCamera();
      m_riw[i]->Render();
    }
  }

  for (int i = 0; i < 3; i++) {
    if (m_riw_us[i]) {
      m_riw_us[i]->SetResliceMode(mode ? 1 : 0);
      m_riw_us[i]->GetRenderer()->ResetCamera();
      m_riw_us[i]->Render();
    }
  }
}


void App::ResetViews() {
  // Reset the reslice image views
  for (int i = 0; i < 3; i++) {
    m_riw[i]->Reset();
  }

  for (int i = 0; i < 3; i++) {
    if (m_riw_us[i]) {
      m_riw_us[i]->Reset();
    }
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

void App::ClearSeedsInView1() {
  ClearSeedsInView(1);
  this->ui->btnSeg->setEnabled(false);
}
void App::ClearSeedsInView(int i) {
  if (this->m_seeds[i]) {
    this->m_seeds[i]->SetEnabled(0);
    this->m_seeds[i] = nullptr;
  }
}

void App::AddSeedsToView(int i) {
  // remove existing widgets.
  if (this->m_seeds[i]) {
    this->m_seeds[i]->SetEnabled(0);
    this->m_seeds[i] = nullptr;
  }

  // Add a seed widget
  this->m_seeds[i] = vtkSmartPointer<vtkSeedWidget>::New();
  this->m_seeds[i]->SetInteractor(
                                  this->m_riw[i]->GetResliceCursorWidget()->GetInteractor());

  // Set priority higher than reslice cursor widget
  this->m_seeds[i]->SetPriority(
                                this->m_riw[i]->GetResliceCursorWidget()->GetPriority() + 0.01);

  // Create the representation
  auto handle =
    vtkSmartPointer<vtkPointHandleRepresentation3D>::New();
  if (i == 0) {
    handle->GetProperty()->SetColor(1, 0, 0);
  } else if (i == 1) {
    handle->GetProperty()->SetColor(0, 1, 0);
  } else {
    handle->GetProperty()->SetColor(0, 0, 1);
  }
  double sz = handle->GetHandleSize();
  handle->SetHandleSize(2.0*sz);

  auto rep =
    vtkSmartPointer<vtkSeedRepresentation>::New();
  rep->SetHandleRepresentation(handle);

  m_seeds[i]->SetRepresentation(rep);


  // Seed callback (TODO)
  auto seedCallback =
    vtkSmartPointer<vtkSeedImageCallback>::New();
  seedCallback->SetRepresentation(rep);
  seedCallback->SetWidget(this->m_seeds[i]);

  // TODO: Catch also move point event
  // LIFO behavior: This is called after seedCallback (registered below)
#if 1
  this->Connections->Connect(this->m_seeds[i],
                             vtkCommand::PlacePointEvent,
                             this,
                             SLOT(SeedsUpdated(vtkObject*, unsigned long, void*, void*)),
                             nullptr /*  void* clientData */,
                             0.0f /*  float priority */,
                             Qt::AutoConnection /*  connection type */);

#else
  this->Connections->Connect(this->m_seeds[i],
                             vtkCommand::PlacePointEvent || vtkCommand::DeletePointEvent || vtkCommand::InteractionEvent ||
                             vtkCommand::EndInteractionEvent,
                             this,
                             SLOT(SeedsUpdateData(vtkObject*, unsigned long, void*, void*)),
                             nullptr /*  void* clientData */,
                             0.0f /*  float priority */,
                             Qt::AutoConnection /*  connection type */);
#endif

  this->m_seeds[i]->AddObserver(vtkCommand::PlacePointEvent, seedCallback);
  this->m_seeds[i]->AddObserver(vtkCommand::InteractionEvent, seedCallback);


  this->m_riw[i]->GetMeasurements()->AddItem(this->m_seeds[i]);

  this->m_seeds[i]->CreateDefaultRepresentation();
  this->m_seeds[i]->EnabledOn();


  // EventQtSlotConnect..
}

// TODO: Adding points update statistics, moving or deleting -> redo statistics
void App::SeedsUpdateData(vtkObject* obj, unsigned long event, void* calldata, void* clientData) {
  vtkSeedWidget* seedWidget =
      vtkSeedWidget::SafeDownCast(obj);

  static bool activeInteraction = false;
  static bool justPlacedPoint = false;
  if (seedWidget) {
    // std::cout << "SeedsUpdateData: " << event << std::endl;
    auto rep = seedWidget->GetSeedRepresentation();

    double origin[3];
    double spacing[3];
    int imageDims[3];

    vtkImageData* pImage = m_riw[1]->GetInput();

    pImage->GetSpacing(spacing);
    pImage->GetOrigin(origin);
    pImage->GetDimensions(imageDims);

    int nx = imageDims[0];
    int ny = imageDims[1];
    int nz = imageDims[2];

    if (event == vtkCommand::PlacePointEvent) {
      // New point - update vsum and vsum2
      std::cout << "PlacePointEvent" << std::endl;
      justPlacedPoint = true;
    } else if (event == vtkCommand::InteractionEvent) {
      // No action needed
      std::cout << "InteractionEvent" << std::endl;
      if (!justPlacedPoint) {
        activeInteraction = true;
      }
      justPlacedPoint = false;
    } else if (event == vtkCommand::EndInteractionEvent) {
      std::cout << "EndInteractionEvent" << std::endl;
      // Done moving point
      if (activeInteraction) {
        // Update vsum and vsum2 using last position or redo everything


        activeInteraction = false;
      }
      justPlacedPoint = false;
    }
  }
}
void App::TransformationUpdated(int index) {
  if (m_riw[index]) {
    vtkResliceCursorLineRepresentation *rep = dynamic_cast<
        vtkResliceCursorLineRepresentation * >(
        m_riw[index]->GetResliceCursorWidget()->GetRepresentation());
    if (rep) {
      rep->GetResliceCursorActor()->GetCursorAlgorithm()->GetResliceCursor();

      vtkSmartPointer<vtkMatrix4x4> resliceAxes =
          vtkSmartPointer<vtkMatrix4x4>::New();

      resliceAxes->DeepCopy(rep->GetResliceAxes()->GetData());

      double trans[4][4];
      memcpy(&trans[0][0], resliceAxes->GetData(), 16 * sizeof(double));

      for (size_t i = 0; i < 4; i++) {
        for (size_t j = 0; j < 4; j++) {
            QModelIndex _index = transModel->index(i, j);
            this->transModel->setData(_index, trans[i][j], Qt::EditRole);
        }
      }
    }
    auto ps = m_riw[index]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetResliceCursor();
    if (ps) {
      double* pTmp = ps->GetCenter();
      for (size_t i = 0 ; i < 3 ; i++) {
        QModelIndex _index = transModel->index(4, i);
        this->transModel->setData(_index, pTmp[i], Qt::EditRole);
      }
    }
  }
}
void App::SeedsUpdated(vtkObject* obj, unsigned long event, void* calldata, void* clientData)
{
  // TODO: Subscribe to more than one event type and maintain a list

  vtkSeedWidget* seedWidget =
      vtkSeedWidget::SafeDownCast(obj);

  if (seedWidget) {
    auto rep = seedWidget->GetSeedRepresentation();

    double origin[3];
    double spacing[3];
    int imageDims[3];

    vtkImageData* pImage = m_riw[1]->GetInput();

    pImage->GetSpacing(spacing);
    pImage->GetOrigin(origin);
    pImage->GetDimensions(imageDims);

    int nx = imageDims[0];
    int ny = imageDims[1];
    int nz = imageDims[2];

    double vsum = 0.0;
    double vsqsum = 0.0;

    // Quick hack
    printf("ScalarType: %d\n", pImage->GetScalarType());
    // std::cout << "scalar size: " << pImage->GetScalarSize() << std::endl;
    assert(pImage->GetScalarType() == 4 || pImage->GetScalarType() == 5);

    // TODO: Template this
    int iType = pImage->GetScalarType();

    short* pVoxels = static_cast<short*>( pImage->GetScalarPointer());
    unsigned short* pVoxels0 = static_cast<unsigned short*>( pImage->GetScalarPointer());

    int nPixels = 0;
    double v;
    double v2;

    // TODO: Update last result instead of go throuh all
    // TODO: Support more than one seed

    for (unsigned int seedId = 0; static_cast<int>(seedId) <
             rep->GetNumberOfSeeds();
         seedId++) {
      double pos[3];
      rep->GetSeedWorldPosition(seedId, pos);
      int ix = (pos[0] - origin[0]) / spacing[0];
      int iy = (pos[1] - origin[1]) / spacing[1];
      int iz = (pos[2] - origin[2]) / spacing[2];

      if (seedId == 0) {
        data["seedX"] = ix;
        data["seedY"] = iy;
        data["seedZ"] = iz;

        if (iType == 4) {
          signed short iValue = pVoxels[iz*ny*nx + iy*nx + ix];
          data["value"] = iValue;
          std::cout << "Voxel value: " << iValue << std::endl;
        } else {
          unsigned short iValue = pVoxels0[iz*ny*nx + iy*nx + ix];
          data["value"] = iValue;
          std::cout << "Voxel value: " << iValue << std::endl;
        }
      }
      // Hack to loop over 3x3 neighborhood
      for (int x = ix - 1 ; x < ix + 2 ; x++) {
        for (int y = iy - 1 ; y < iy + 2 ; y++) {
          for (int z = iz - 1 ; z < iz + 2 ; z++) {
            if ((0 <= x) && (x < nx) &&
                (0 <= y) && (y < ny) &&
                (0 <= z) && (z < nz)) {
              if (iType == 4) {
                v = double(pVoxels[z*ny*nx + y*nx + x]);
              } else {
                v = double(pVoxels0[z*ny*nx + y*nx + x]);
              }
              // std::cout << "v: " << v << std::endl;
              v2 = v*v;
              vsum = vsum + v;
              vsqsum = vsqsum + v2;
              nPixels++;
            }
          }
        }
      }
    }

    if (nPixels > 1) {
      double mean = vsum / nPixels;
      std::cout << "mean: " << mean << std::endl;
      double s2 = (vsqsum / double(nPixels) - SQUARE(vsum / double(nPixels))) * double(nPixels) / (double(nPixels) - 1.0);
      double std = sqrt(s2);
      std::cout << "std: " << std << std::endl;
      // HERE
      data["std"] = std;
      data["mean"]  = mean;
      // Enabled segmentation button
      this->ui->btnSeg->setEnabled(true);

      int iLow = int(mean - 2.5*std);
      int iHigh = int(mean + 2.5*std);
      this->thresholdsSlider->setLowerValue(iLow);
      this->thresholdsSlider->setUpperValue(iHigh);

      data["low"] = iLow;
      data["high"] = iHigh;
      this->m_sliderLblLow->setText(QString::number(iLow));
      this->m_sliderLblHigh->setText(QString::number(iHigh));

    }

    // (n vsqsum - vsum**2) / (n*(n-1))

    // itk::ImageToVTKImageFilter
  }
}


#if 0
// Use vtkPointWidget or vtkSeedWidget
void QtVTKRenderWindows::AddDistanceMeasurementToView(int i) {
  // remove existing widgets.
  if (this->DistanceWidget[i]) {
    this->DistanceWidget[i]->SetEnabled(0);
    this->DistanceWidget[i] = nullptr;
  }

  // add new widget
  this->DistanceWidget[i] = vtkSmartPointer< vtkDistanceWidget >::New();
  this->DistanceWidget[i]->SetInteractor(
    this->riw[i]->GetResliceCursorWidget()->GetInteractor());

  // Set a priority higher than our reslice cursor widget
  this->DistanceWidget[i]->SetPriority(
    this->riw[i]->GetResliceCursorWidget()->GetPriority() + 0.01);

  vtkSmartPointer< vtkPointHandleRepresentation2D > handleRep =
    vtkSmartPointer< vtkPointHandleRepresentation2D >::New();
  vtkSmartPointer< vtkDistanceRepresentation2D > distanceRep =
    vtkSmartPointer< vtkDistanceRepresentation2D >::New();
  distanceRep->SetHandleRepresentation(handleRep);
  this->DistanceWidget[i]->SetRepresentation(distanceRep);
  distanceRep->InstantiateHandleRepresentation();
  distanceRep->GetPoint1Representation()->SetPointPlacer(riw[i]->GetPointPlacer());
  distanceRep->GetPoint2Representation()->SetPointPlacer(riw[i]->GetPointPlacer());

  // Add the distance to the list of widgets whose visibility is managed based
  // on the reslice plane by the ResliceImageViewerMeasurements class
  this->riw[i]->GetMeasurements()->AddItem(this->DistanceWidget[i]);

  this->DistanceWidget[i]->CreateDefaultRepresentation();
  this->DistanceWidget[i]->EnabledOn();
}

#endif


/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */

