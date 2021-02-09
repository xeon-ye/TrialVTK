// TODO: Synchronize cursor widget
// TODO: Synchronize zoom

#pragma once

#include <vtkCommand.h>
#include <vtkImageMapToColors.h>
#include <vtkImagePlaneWidget.h>
#include <vtkInteractorStyleImage.h>
#include <vtkPlaneSource.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkResliceCursor.h>
#include <vtkResliceCursorActor.h>
#include <vtkResliceCursorLineRepresentation.h>
#include <vtkResliceCursorPolyDataAlgorithm.h>

#include <vtkResliceCursorWidget.h>

#include <vtkPlane.h>

#include <QDebug>

//vtkSeedRepresentation* SeedRepresentation = nullptr;
//vtkSeedWidget* SeedWidget = nullptr;

class vtkResliceCursorCallback : public vtkCommand {
 public:
  static vtkResliceCursorCallback *New() {
    return new vtkResliceCursorCallback;
  }

  void Execute(vtkObject *caller, unsigned long ev,
               void *callData ) override {

    if (ev == vtkResliceCursorWidget::ResliceAxesChangedEvent) {
      // std::cout << "axes changed2" << std::endl;
    }


    if (ev == vtkResliceCursorWidget::WindowLevelEvent ||
        ev == vtkCommand::WindowLevelEvent ||
        ev == vtkResliceCursorWidget::ResliceThicknessChangedEvent) {

      // Render reslice cursor widgets
      for (int i = 0; i < 3; i++) {
        if (this->RCW[i]) {
          this->RCW[i]->Render();
        }
        if (this->USRCW[i]) {
          this->USRCW[i]->Render();
        }
      }
      // Render image-plane widget 3D
      if (this->IPW[0]) {
        this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
      }
      return;
    }

    vtkImagePlaneWidget* ipw =
      dynamic_cast< vtkImagePlaneWidget* >( caller );
    if (ipw) {
      // Synchronize window level
      double* wl = static_cast<double*>( callData );
      if ( ipw == this->IPW[0] ) {
        if (this->IPW[1]) {
          this->IPW[1]->SetWindowLevel(wl[0],wl[1],1);
        }
        if (this->IPW[2]) {
          this->IPW[2]->SetWindowLevel(wl[0],wl[1],1);
        }
      } else if( ipw == this->IPW[1] ) {
        if (this->IPW[0]) {
          this->IPW[0]->SetWindowLevel(wl[0],wl[1],1);
        }
        if (this->IPW[2]) {
          this->IPW[2]->SetWindowLevel(wl[0],wl[1],1);
        }
      } else if (ipw == this->IPW[2]) {
        if (this->IPW[0]) {
          this->IPW[0]->SetWindowLevel(wl[0],wl[1],1);
        }
        if (this->IPW[1]) {
          this->IPW[1]->SetWindowLevel(wl[0],wl[1],1);
        }
      }
    }

    vtkResliceCursorWidget *rcw =
      dynamic_cast<vtkResliceCursorWidget*>(caller);
    if (rcw) {
      vtkResliceCursorLineRepresentation *rep = dynamic_cast<
          vtkResliceCursorLineRepresentation * >(rcw->GetRepresentation());

      // Although the return value is not used, we keep the get calls
      // in case they had side-effects
      auto cursor = rep->GetResliceCursorActor()->GetCursorAlgorithm()->GetResliceCursor();

      auto src = this->RCW[0]->GetResliceCursorRepresentation()->GetResliceCursor();
      auto dest  = this->USRCW[0]->GetResliceCursorRepresentation()->GetResliceCursor();
      for (int i = 0; i < 3 ; i++) {
        auto normal = src->GetPlane(i)->GetNormal();
        dest->GetPlane(i)->SetNormal(normal);
        auto origin = src->GetPlane(i)->GetOrigin();
        dest->GetPlane(i)->SetOrigin(origin);
      }

      // Update 3D widget
      for (int i = 0; i < 3; i++) {
        if (this->IPW[i]) {
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

          // Check if this causes Invalid extent
          if (true) {

            // Updata other modality - only cursor is updated
            if (rep == this->RCW[i]->GetResliceCursorRepresentation()) {
              // MR is resliced, update US

              // Works - update cursor of ultrasound using CT
              vtkResliceCursor* target =
                this->USRCW[i]->GetResliceCursorRepresentation()->
                GetResliceCursor();

              // TODO: Consider using SetHoleWithInPixels(pixxels) so it is easier to see pixel at cursor
              target->SetCenter(cursor->GetCenter());




              // rep->InitializeReslicePlane();

              // std::cout << "update plane" << std::endl;
              // TEST
              // GetResliceAxes();
              // this->USRCW[i]->SetResliceCursor

              // ResliceAxesChangedEvent
              // SetManipulationMode(vtkResliceCursorRepresentation::RotateBothAxes)
              // CTRL - rotates both

              // TODO: Figure out to copy plane also
            }
          }
        }
      }
    }

    // Render everything
    for (int i = 0; i < 3; i++) {
      if (this->RCW[i]) {
        this->RCW[i]->Render();
      }
      if (this->USRCW[i]) {
        this->USRCW[i]->Render();
      }
    }

    // Any of the in-plane widgets will do
    if (this->IPW[0]) {
      this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
    }
  }

  vtkResliceCursorCallback() {
    IPW[0] = IPW[1] = IPW[2] = nullptr;
    RCW[0] = RCW[1] = RCW[2] = nullptr;
  }
  vtkImagePlaneWidget* IPW[3];
  vtkResliceCursorWidget *RCW[3];
  vtkResliceCursorWidget *USRCW[3];
};

