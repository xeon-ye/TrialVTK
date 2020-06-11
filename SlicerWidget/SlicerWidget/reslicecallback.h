#pragma once

#include <vtkCommand.h>
#include <vtkImageMapToColors.h>
#include <vtkInteractorStyleImage.h>
#include <vtkPlaneSource.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkResliceCursorActor.h>
#include <vtkResliceCursorLineRepresentation.h>
#include <vtkResliceCursorPolyDataAlgorithm.h>

#include <vtkResliceCursorWidget.h>

#include <vtkMatrix4x4.h>

#include <QDebug>

class vtkResliceCursorCallback : public vtkCommand {
 public:
  static vtkResliceCursorCallback *New() {
    return new vtkResliceCursorCallback;
  }

  void Execute( vtkObject *caller, unsigned long ev,
                void *callData ) override {
    if (ev == vtkResliceCursorWidget::WindowLevelEvent ||
        ev == vtkCommand::WindowLevelEvent ||
        ev == vtkResliceCursorWidget::ResliceThicknessChangedEvent) {
      // Render reslice cursor widgets
      for (int i = 0; i < 3; i++) {
        this->RCW[i]->Render();
      }
      // Render image-plane widget
      this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
      return;
    }

    vtkImagePlaneWidget* ipw =
      dynamic_cast< vtkImagePlaneWidget* >( caller );
    if (ipw) {
      double* wl = static_cast<double*>( callData );
      if ( ipw == this->IPW[0] ) {
        this->IPW[1]->SetWindowLevel(wl[0],wl[1],1);
        this->IPW[2]->SetWindowLevel(wl[0],wl[1],1);
      } else if( ipw == this->IPW[1] ) {
        this->IPW[0]->SetWindowLevel(wl[0],wl[1],1);
        this->IPW[2]->SetWindowLevel(wl[0],wl[1],1);
      } else if (ipw == this->IPW[2]) {
        this->IPW[0]->SetWindowLevel(wl[0],wl[1],1);
        this->IPW[1]->SetWindowLevel(wl[0],wl[1],1);
      }
    }

    vtkResliceCursorWidget *rcw =
      dynamic_cast<vtkResliceCursorWidget*>(caller);
    if (rcw) {
      vtkResliceCursorLineRepresentation *rep = dynamic_cast<
          vtkResliceCursorLineRepresentation * >(rcw->GetRepresentation());

      // Although the return value is not used, we keep the get calls
      // in case they had side-effects
      rep->GetResliceCursorActor()->GetCursorAlgorithm()->GetResliceCursor();

      vtkSmartPointer<vtkMatrix4x4> resliceAxes =
        vtkSmartPointer<vtkMatrix4x4>::New();

      resliceAxes->DeepCopy(rep->GetResliceAxes()->GetData());

      double trans[4][4];
      memcpy(&trans[0][0], resliceAxes->GetData(), 16*sizeof(double));
      for (size_t i = 0 ; i < 4 ; i++) {
        for (size_t j = 0 ; j < 4 ; j++) {
          std::cout << trans[i][j] << " ";
        }
        std::cout << std::endl;
      }

      // Update 3D widget
      for (int i = 0; i < 3; i++) {
        vtkPlaneSource *ps = static_cast< vtkPlaneSource * >(
                               this->IPW[i]->GetPolyDataAlgorithm());
        double origin[3] = {};
        double* pTmp = this->RCW[i]->GetResliceCursorRepresentation()->
                       GetPlaneSource()->GetOrigin();
        memcpy(origin, pTmp, 3*sizeof(double));

        ps->SetOrigin(origin);
        ps->SetPoint1(this->RCW[i]->GetResliceCursorRepresentation()->
                      GetPlaneSource()->GetPoint1());
        ps->SetPoint2(this->RCW[i]->GetResliceCursorRepresentation()->
                      GetPlaneSource()->GetPoint2());

        // If the reslice plane has modified, update it on the 3D widget
        this->IPW[i]->UpdatePlacement();
      }
    }

    // Render everything
    for (int i = 0; i < 3; i++) {
      this->RCW[i]->Render();
    }
    // Any of the in-plane widgets will do
    this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
  }

  vtkResliceCursorCallback() {}
  vtkImagePlaneWidget* IPW[3];
  vtkResliceCursorWidget *RCW[3];
};
