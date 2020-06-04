#include <sstream>

#include <vtkImageThreshold.h>
#include <vtkSmartPointer.h>
#include <vtkImageGaussianSmooth.h>
#include <vtkMarchingCubes.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkStripper.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkMetaImageReader.h>
#include <vtkCleanPolyData.h>

#include <Registration/whereami.h>


int main() {
  auto reader = vtkSmartPointer<vtkMetaImageReader>::New();
  const char* fileName = "seg.mhd";

  std::stringstream ss;
  int dirnameLength = 0;
  int length = wai_getExecutablePath(NULL, 0, &dirnameLength);
  char* path = (char*)malloc(length + 1);

  wai_getModulePath(path, length, &dirnameLength);

  path[length] = '\0';

  if (dirnameLength + 2 < length) {
      path[dirnameLength + 1] = '\0';
  }

  ss << path;
  ss << fileName;

  reader->SetFileName(ss.str().c_str());
  reader->Update();

  auto selectTissue = vtkSmartPointer<vtkImageThreshold>::New();
  selectTissue->ThresholdBetween(1.0,1.0);//2.0);
  selectTissue->SetInValue(255);
  selectTissue->SetOutValue(0);
  selectTissue->SetInputConnection(reader->GetOutputPort());

  int gaussianRadius = 1.0;
  double gaussianStandardDeviation = 2.0;

  auto gaussian = vtkSmartPointer<vtkImageGaussianSmooth>::New();
  // I anticipate if only one is specified, they will be the same for x,y and z.
  gaussian->SetStandardDeviations(gaussianStandardDeviation,
                                  gaussianStandardDeviation,
                                  gaussianStandardDeviation);
  gaussian->SetRadiusFactors(gaussianRadius,
                             gaussianRadius,
                             gaussianRadius);
  gaussian->SetInputConnection(selectTissue->GetOutputPort());

  double isoValue = 127.5;

  auto mcubes = vtkSmartPointer<vtkMarchingCubes>::New();

  mcubes->SetInputConnection(gaussian->GetOutputPort());
  mcubes->ComputeScalarsOff();
  mcubes->ComputeGradientsOff();
  mcubes->ComputeNormalsOff();
  mcubes->SetValue(0, isoValue);

  int smoothingIterations = 5; // was 3
  double passBand = 0.001;
  double featureAngle = 60.0;
  auto smoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
  smoother->SetInputConnection(mcubes->GetOutputPort());
  smoother->SetNumberOfIterations(smoothingIterations);
  smoother->BoundarySmoothingOff(); // smoother->BoundarySmoothingOn()
  smoother->FeatureEdgeSmoothingOff(); // Turn off smoothing along sharp interior edges
  smoother->SetFeatureAngle(featureAngle); // Angle to distinguish a sharp edge
  smoother->SetPassBand(passBand);
  smoother->NonManifoldSmoothingOn();
  smoother->NormalizeCoordinatesOn();
  smoother->Update();

  auto normals = vtkSmartPointer<vtkPolyDataNormals>::New();
  normals->SetInputConnection(smoother->GetOutputPort());
  normals->SetFeatureAngle(featureAngle);

  auto stripper = vtkSmartPointer<vtkStripper>::New();
  stripper->SetInputConnection(normals->GetOutputPort());

#if 1
  // Connectivity filter
  auto connectFilter = vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
  connectFilter->SetInputConnection(stripper->GetOutputPort());
  connectFilter->SetExtractionModeToLargestRegion();
  connectFilter->Update();
#endif

  auto cleaner = vtkSmartPointer<vtkCleanPolyData>::New();
  cleaner->SetInputConnection(connectFilter->GetOutputPort());
  //cleaner = connectFilter; // TODO: Make this work all the time
  // cleaner = stripper

  char* oFileName = "A.vtp";
  auto writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();

  ss.str("");
  ss.clear();
  ss << path;
  ss << oFileName;

  writer->SetFileName(ss.str().c_str());
  writer->SetInputConnection(stripper->GetOutputPort());
  writer->Write();
  return 0;
}
