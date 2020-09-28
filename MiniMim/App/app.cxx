#include <App/ui_app.h>

#include <QSettings>
#include <QDebug>

#include <cstdlib>
#include <iostream>

#ifdef _WIN32
# define _USE_MATH_DEFINES 1
#endif
#include <cmath>

#include <App/app.hpp>
#include <App/filedialog.hpp>

#include "vtkDistanceWidget.h"
#include "vtkPointHandleRepresentation2D.h"
#include "vtkDistanceRepresentation2D.h"
#include "vtkBoundedPlanePointPlacer.h"
#include "vtkResliceImageViewerMeasurements.h"
#include "vtkOrientedGlyphContourRepresentation.h"
#include "vtkImageActorPointPlacer.h"
#include "vtkLine.h"
#include "vtkProperty.h"
#include "vtkResliceCursorLineRepresentation.h"
#include "vtkProperty2D.h"

#include "QVTKOpenGLWidget.h"

// Ugly
#include <SlicerWidget/ui_datamanager.h>


App::~App() {}

void App::SetupUI() {
  this->ui = new Ui_App;
  this->ui->setupUi(this);

  datamanager = new DataManager(this);
  horizontalLayout = new QHBoxLayout(this->ui->widget);
  horizontalLayout->setObjectName(QStringLiteral("horizontalLayoutVTK"));
  horizontalLayout->addWidget(this->datamanager);

  DistanceWidget[0] = DistanceWidget[1] = DistanceWidget[2] = nullptr;
}

void App::PopulateMenus() {
  connect(this->ui->actionExit, SIGNAL(triggered()),
          this, SLOT(slotExit()));
  connect(this->ui->actionOpen, SIGNAL(triggered()),
          this, SLOT(onLoadClicked()));
  connect(this->ui->cbViewMeasurement, SIGNAL(activated(int)),
      this, SLOT(referenceViewChanged(int)));
  connect(this->ui->resetButton, SIGNAL(pressed()), this, SLOT(ResetViews()));
  connect(this->ui->btnAddDistance, SIGNAL(pressed()), this,
          SLOT(AddDistanceMeasurementToView()));
  connect(this->ui->btnAddContour, SIGNAL(pressed()), this,
          SLOT(AddContourWidgetToView()));
  connect(this->ui->resliceModeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(resliceMode(int)));

  connect(this->ui->btnClearDistance, SIGNAL(pressed()), this,
          SLOT(ClearDistanceView()));
  connect(this->ui->btnClearContour, SIGNAL(pressed()), this,
          SLOT(ClearContour()));
}

void App::resliceMode(int i) {
  this->datamanager->resliceMode(i);
}

void App::referenceViewChanged(int index) {
  std::cout << "reference view is now: " << index << std::endl;
  this->datamanager->SetReferenceSlice(index);
}

void App::onLoadClicked() {
  const QString DEFAULT_DIR_KEY("SlicerWidgetDefaultDir");
  QSettings MySettings;

  QString selectedDirectory;

  FileDialog w;
  qDebug() << "default_dir:" << MySettings.value(DEFAULT_DIR_KEY).toString();
  w.setDirectory(MySettings.value(DEFAULT_DIR_KEY).toString());

  int nMode = w.exec();
  QStringList fnames = w.selectedFiles();

  if (nMode != 0 && fnames.size() != 0) {
    qDebug() << fnames;

    datamanager->FileLoad(fnames[0]);

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
void App::ResetViews() {
  this->datamanager->ResetViews();
}

void App::AddDistanceMeasurementToView()
{
  int index = this->ui->cbViewMeasurement->currentIndex();
  this->AddDistanceMeasurementToView(index);
}

void App::ClearDistanceView(int i) {
  if (this->DistanceWidget[i]) {
    this->DistanceWidget[i]->SetEnabled(0);
    this->DistanceWidget[i] = nullptr;
  }
  this->datamanager->Render();
}
void App::ClearDistanceView() {
  int index = this->ui->cbViewMeasurement->currentIndex();
  this->ClearDistanceView(index);
}


void App::ClearContour(int i) {
  if (this->ContourWidget[i]) {
    this->ContourWidget[i]->SetEnabled(0);
    this->ContourWidget[i] = nullptr;
  }
  datamanager->Render();
}

void App::ClearContour() {
  int index = this->ui->cbViewContour->currentIndex();
  this->ClearContour(index);
}

void App::AddDistanceMeasurementToView(int i)
{
  // remove existing widgets.
  if (this->DistanceWidget[i])
  {
    this->DistanceWidget[i]->SetEnabled(0);
    this->DistanceWidget[i] = nullptr;
  }

    // remove existing widgets.
  if (this->ContourWidget[i]) {
    this->ContourWidget[i]->SetEnabled(0);
    this->ContourWidget[i] = nullptr;
  }

  // add new widget
  this->DistanceWidget[i] = vtkSmartPointer< vtkDistanceWidget >::New();
  this->DistanceWidget[i]->SetInteractor(
    this->datamanager->m_riw[i]->GetResliceCursorWidget()->GetInteractor());

  // Set a priority higher than our reslice cursor widget
  this->DistanceWidget[i]->SetPriority(
    this->datamanager->m_riw[i]->GetResliceCursorWidget()->GetPriority() + 0.01);

  vtkSmartPointer< vtkPointHandleRepresentation2D > handleRep =
    vtkSmartPointer< vtkPointHandleRepresentation2D >::New();
  vtkSmartPointer< vtkDistanceRepresentation2D > distanceRep =
    vtkSmartPointer< vtkDistanceRepresentation2D >::New();
  // distanceRep->GetAxisProperty()
  distanceRep->SetHandleRepresentation(handleRep);
  this->DistanceWidget[i]->SetRepresentation(distanceRep);
  distanceRep->InstantiateHandleRepresentation();
  distanceRep->GetPoint1Representation()->SetPointPlacer(this->datamanager->m_riw[i]->GetPointPlacer());
  distanceRep->GetPoint2Representation()->SetPointPlacer(this->datamanager->m_riw[i]->GetPointPlacer());

  // Add the distance to the list of widgets whose visibility is managed based
  // on the reslice plane by the ResliceImageViewerMeasurements class
  this->datamanager->m_riw[i]->GetMeasurements()->AddItem(this->DistanceWidget[i]);

  this->DistanceWidget[i]->CreateDefaultRepresentation();
  this->DistanceWidget[i]->EnabledOn();
  this->datamanager->Render();
}

void App::AddContourWidgetToView(int index) {
  if (this->ContourWidget[index]) {
    this->ContourWidget[index]->SetEnabled(0);
    this->ContourWidget[index] = nullptr;
  }
  // remove existing widgets.
  if (this->DistanceWidget[index]) {
    this->DistanceWidget[index]->SetEnabled(0);
    this->DistanceWidget[index] = nullptr;
  }


  this->ContourWidget[index] = vtkSmartPointer<vtkContourWidget>::New();

  vtkSmartPointer<vtkOrientedGlyphContourRepresentation> rep =
    vtkOrientedGlyphContourRepresentation::New();
  rep->GetLinesProperty()->SetColor(1, 0, 0);
  rep->GetLinesProperty()->SetLineWidth(2.0);

  this->ContourWidget[index]->SetRepresentation(rep);

  vtkSmartPointer<vtkImageActorPointPlacer> imageActorPointPlacer =
    vtkImageActorPointPlacer::New();
  imageActorPointPlacer->SetImageActor(
    this->datamanager->m_riw[index]->GetImageActor());
  rep->SetPointPlacer(imageActorPointPlacer);

  // this->ContourWidget[index]->SetRepresentation(rep);

  imageActorPointPlacer->Delete();
  rep->Delete();

  // Test
  QVTKOpenGLWidget* ppVTKOGLWidgets[4] = {
    this->datamanager->ui->view0, this->datamanager->ui->view1,
    this->datamanager->ui->view2, this->datamanager->ui->view3};

  this->ContourWidget[index]->SetInteractor(
    ppVTKOGLWidgets[index]->GetRenderWindow()->GetInteractor());
  // this->ContourWidget[index]->SetInteractor(this->datamanager->m_riw[index]->GetInteractor());

  this->ContourWidget[index]->SetPriority(
    this->datamanager->m_riw[index]->GetResliceCursorWidget()->GetPriority() +
    0.01);

  this->ContourWidget[index]->SetEnabled(true);
  this->ContourWidget[index]->On();

  vtkSmartPointer<vtkPoints> points = vtkPoints::New();
  vtkSmartPointer<vtkCellArray> lines = vtkCellArray::New();

  double radius = 20.0;

  double origin[3] = {0.0, 0.0, 0.0};
  double* pTmp = this->datamanager->m_riw[index]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetPlaneSource()->GetCenter();

  memcpy(origin, pTmp, 3*sizeof(double));

  for (int i = 0 ; i < 11 ; i++) {
    int ii = i % 10;
    double angle = 2.0 * M_PI * i / 10.0;
    if (index == 2) {
      points->InsertNextPoint(origin[0] + radius * cos(angle),
                              origin[1] + radius * sin(angle),
                              origin[2]);
    } else if (index == 1) {
        points->InsertNextPoint(origin[0] + radius * cos(angle),
            origin[1] ,
            origin[2] + radius * sin(angle));
    } else {
        points->InsertNextPoint(origin[0] ,
            origin[1] + radius * cos(angle),
            origin[2] + radius * sin(angle));
    }
    auto line = vtkLine::New();
    line->GetPointIds()->SetId(0,ii);
    line->GetPointIds()->SetId(1,ii+1);
    lines->InsertNextCell(line);
  }

  vtkSmartPointer<vtkPolyData> pd = vtkPolyData::New();
  pd->SetPoints(points);
  pd->SetLines(lines);

  this->ContourWidget[index]->Initialize(pd, 1);
  this->ContourWidget[index]->Render();
  this->datamanager->Render();
}
void App::AddContourWidgetToView() {
  int index = this->ui->cbViewContour->currentIndex();
  this->AddContourWidgetToView(index);
}

App::App(int argc, char* argv[]) {
  this->SetupUI();
  this->PopulateMenus();
}

void App::slotExit() {
  qApp->exit();
}

void App::resizeEvent(QResizeEvent* event) {
  //qDebug() << "resizeEvent";
  QMainWindow::resizeEvent(event);
  datamanager->Render();// Your code here.
}

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
