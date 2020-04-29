// TODO: Fix callback such that US doesn't rescale 3D
//       Possibility to follow MR

// Overload vtkInteractorStyleImage::OnMouseWheelForward and post events to
// other modality
// or imageStyle->AddObserver(vtkCommand::MouseMoveEvent, callback);


// renderer->ResetCamera();
// renderer->GetActiveCamera()->Zoom(1.5);

#include <Registration/ui_app.h>
#include <Registration/reslicecallback.h>
#include <Registration/seedcallback.h>
#include <Registration/utils.hpp>

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
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkImageReader2.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkNamedColors.h>
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

void App::segmSliderChanged(int value) {
  qDebug() << value;
}
void App::onApplyPresetClick() {
  qDebug() << "preset";
  double window = 200;//50.0; // 200
  double level = 100;//145.0; // 100

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
void App::onSegClick() {
  const int i = 1;

  double spacing[3];
  double origin[3];
  m_riw[1]->GetInput()->GetSpacing(spacing);
  m_riw[1]->GetInput()->GetOrigin(origin);

  // Or use Get
  //        double *spacing = signedDistInfo->Get(vtkDataObject::SPACING());
  // Get image from ResliceCursor, same center as input data

  if (m_seeds[i]) {
    auto rep = m_seeds[i]->GetSeedRepresentation();

    for (unsigned int seedId = 0; static_cast<int>(seedId) <
           rep->GetNumberOfSeeds();
         seedId++) {
      double pos[3];
      rep->GetSeedWorldPosition(seedId, pos);
      std::cout << "Seed " << seedId << " : (" << pos[0] << " " << pos[1]
                << " " << pos[2] << ")" << std::endl;

      int voxel[3];
      voxel[0] = (pos[0] - origin[0]) / spacing[0];
      voxel[1] = (pos[1] - origin[1]) / spacing[1];
      voxel[2] = (pos[2] - origin[2]) / spacing[2];

      std::cout << "Voxel " << seedId << " : (" << voxel[0] << " " << voxel[1]
          << " " << voxel[2] << ")" << std::endl;
    }
  }
}

void App::onRegClick() {
  (this->*regDelegate)();
}

void App::onCancelClick() {
  printf("Canceling\n");
  // Do something that interrupts registration
  regDelegate = &App::onRegClick;
}

void App::updateProgressBar(int progressPercent) {
  ui->progressBar->setValue(progressPercent);
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
  this->ui->segmVertLayout->insertWidget(0, this->thresholdsSlider);

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
      rep->Highlight(1);//GetImageActor()->SetVisibility(0);
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

  connect(ui->btnPreset, &QPushButton::clicked,
          this, &App::onApplyPresetClick);

  connect(ui->horizontalSlider, &QSlider::valueChanged, this, &App::segmSliderChanged);

  connect(ui->btnAddSeeds, SIGNAL(pressed()), this, SLOT(AddSeedsToView1()));
  connect(ui->btnClearSeeds, SIGNAL(pressed()), this, SLOT(ClearSeedsInView1()));
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

    vtkSmartPointer<vtkXMLPolyDataReader> reader =
        vtkSmartPointer<vtkXMLPolyDataReader>::New();

    if (directory.exists()) {
      return;
    } else {
      QFileInfo info(files);
      if (info.completeSuffix() == QLatin1String("vtp")) {
        reader->SetFileName(files.toUtf8().constData());
        reader->Update();

        vtkSmartPointer<vtkPolyDataMapper> mapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(reader->GetOutputPort());
        vtkSmartPointer<vtkActor> actor =
            vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        auto prop = actor->GetProperty();
        vtkSmartPointer<vtkNamedColors> namedColors =
            vtkSmartPointer<vtkNamedColors>::New();

        prop->SetColor(namedColors->GetColor3d("Red").GetData());

        m_planeWidget[0]->GetDefaultRenderer()->AddActor(actor);
        this->ui->mrView3D->GetRenderWindow()->Render();
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

  this->SetupUI();
  this->setupMR();

  this->setupUS(); // This cause invalid extent

  this->PopulateMenus();

  regDelegate = &App::onRegStartClick;

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
    m_riw[i]->SetResliceMode(mode ? 1 : 0);
    m_riw[i]->GetRenderer()->ResetCamera();
    m_riw[i]->Render();
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
  return ClearSeedsInView(1);
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
  auto rep =
    vtkSmartPointer<vtkSeedRepresentation>::New();
  rep->SetHandleRepresentation(handle);

  m_seeds[i]->SetRepresentation(rep);


  // Seed callback (TODO)
  auto seedCallback =
    vtkSmartPointer<vtkSeedImageCallback>::New();
  seedCallback->SetRepresentation(rep);
  seedCallback->SetWidget(this->m_seeds[i]);
  this->m_seeds[i]->AddObserver(vtkCommand::PlacePointEvent, seedCallback);
  this->m_seeds[i]->AddObserver(vtkCommand::InteractionEvent, seedCallback);

  this->m_riw[i]->GetMeasurements()->AddItem(this->m_seeds[i]);

  this->m_seeds[i]->CreateDefaultRepresentation();
  this->m_seeds[i]->EnabledOn();



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

