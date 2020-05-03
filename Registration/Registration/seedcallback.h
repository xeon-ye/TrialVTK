#pragma once

#include <vtkSeedWidget.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkPointHandleRepresentation3D.h>
#include <vtkSeedRepresentation.h>

class vtkSeedImageCallback : public vtkCommand {
 public:
  static vtkSeedImageCallback* New() {
    return new vtkSeedImageCallback;
  }

  vtkSeedImageCallback() = default;

  virtual void Execute(vtkObject*, unsigned long event, void* calldata) {
    if (event == vtkCommand::PlacePointEvent) {
      std::cout << "Placing point..." << std::endl;
      std::cout << "There are now "
                << this->SeedRepresentation->GetNumberOfSeeds() << " seeds."
                << std::endl;
      for (unsigned int seedId = 0; static_cast<int>(seedId) <
           this->SeedRepresentation->GetNumberOfSeeds();
           seedId++) {
        double pos[3];
        //        this->SeedRepresentation->GetSeedDisplayPosition(seedId, pos);
        this->SeedRepresentation->GetSeedWorldPosition(seedId, pos);
        std::cout << "Seed " << seedId << " : (" << pos[0] << " " << pos[1]
                  << " " << pos[2] << ")" << std::endl;
      }
      return;
    }
    if (event == vtkCommand::InteractionEvent) {
      // std::cout << "Interaction..." << std::endl;
      if (calldata) {
        double pos[3];
        this->SeedRepresentation->GetSeedDisplayPosition(0, pos);
        // std::cout << "Moved to (" << pos[0] << " " << pos[1] << " " << pos[2]
        //           << ")" << std::endl;
      }
      return;
    }
  }

  void SetRepresentation(vtkSmartPointer<vtkSeedRepresentation> rep) {
    this->SeedRepresentation = rep;
  }
  void SetWidget(vtkSmartPointer<vtkSeedWidget> widget) {
    this->SeedWidget = widget;
  }

 private:
  vtkSeedRepresentation * SeedRepresentation = nullptr;
  vtkSeedWidget* SeedWidget = nullptr;
};
