#include "datamanager.h"
#include <SlicerWidget/ui_datamanager.h>

#include "QVTKOpenGLWidget.h"

#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleRubberBandZoom.h>
#include <vtkGenericOpenGLRenderWindow.h>

#include <vtkResliceImageViewer.h>
#include <vtkResliceCursorLineRepresentation.h>
#include <vtkResliceCursorWidget.h>
#include <vtkResliceCursorActor.h>
#include <vtkResliceCursorPolyDataAlgorithm.h>

DataManager::DataManager(QWidget *parent) :
  QWidget(parent), ui(new Ui::DataManager),
  m_riw({nullptr, nullptr, nullptr}),
m_planeWidget({nullptr, nullptr, nullptr}) {
  ui->setupUi(this);

  // Create reslice image widgets
  QVTKOpenGLWidget* ppVTKOGLWidgets[4] = {
    this->ui->view0,
    this->ui->view1,
    this->ui->view2,
    this->ui->view3
  };

  for (size_t i = 0; i < 3 ; i++) {
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

    rep->GetResliceCursorActor()->
    GetCursorAlgorithm()->SetReslicePlaneNormal(i);
    m_riw[i]->SetSliceOrientation(i);
    m_riw[i]->SetResliceModeToAxisAligned();
  }

  // Create 3D viewer




}

DataManager::~DataManager() {
  delete ui;
}
