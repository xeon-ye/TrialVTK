#include <QDebug>
#include <Registration/SurfRunner.hpp>
#include <Registration/whereami.h>

#include <vtkImageThreshold.h>
#include <vtkSmartPointer.h>
#include <vtkImageGaussianSmooth.h>
#include <vtkMarchingCubes.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkStripper.h>
#include <vtkPolyDataConnectivityFilter.h>
// #include <vtkXMLPolyDataWriter.h>

#include <iostream>
#include <sstream>
SurfRunner::SurfRunner(QWidget *receiver,
    QMap<QString, QVariant> data,
    vtkPolyData* pPolyData,
    vtkImageData* pData,
    int *retval,
    volatile bool *stopped) : QRunnable(), receiver(receiver),
                              retval(retval), stopped(stopped), 
    pPolyData(pPolyData), pData(pData), data(data) {
}

void SurfRunner::internalRun() {

  int imageDims[3];
  pData->GetDimensions(imageDims);

  // Display dimensions
  std::cout << "Segmentation dimensions: " << imageDims[0] << " "
            << imageDims[1] << " "
            << imageDims[2] << std::endl;

  auto selectTissue = vtkSmartPointer<vtkImageThreshold>::New();
  selectTissue->ThresholdBetween(1.0,2.0);
  selectTissue->SetInValue(255);
  selectTissue->SetOutValue(0);
  selectTissue->SetInputData(pData);
  // selectTissue->Update();

 
  double gaussianRadius = 1.0;
  double gaussianStandardDeviation = 2.0;

  auto gaussian = vtkSmartPointer<vtkImageGaussianSmooth>::New();
  // I anticipate if only one is specified, they will be the same for x,y and z.
  gaussian->SetStandardDeviations(gaussianStandardDeviation, gaussianStandardDeviation, gaussianStandardDeviation);
  gaussian->SetRadiusFactors(gaussianRadius, gaussianRadius, gaussianRadius);
  gaussian->SetInputConnection(selectTissue->GetOutputPort());

  double isoValue = 127.5;

  auto mcubes = vtkSmartPointer<vtkMarchingCubes>::New();

  mcubes->SetInputConnection(gaussian->GetOutputPort());
  mcubes->ComputeScalarsOff();
  mcubes->ComputeGradientsOff();
  mcubes->ComputeNormalsOff();
  mcubes->SetValue(0, isoValue);

  int smoothingIterations = 5; // was 3
  float passBand = 0.001f;
  float featureAngle = 60.0f;
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

  // Connectivity filter
  auto connectFilter = vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
  connectFilter->SetInputConnection(stripper->GetOutputPort());
  // connectFilter->SetExtractionModeToSpecifiedRegions();
  // connectFilter->AddSpecifiedRegion(0);
  connectFilter->SetExtractionModeToLargestRegion();
  connectFilter->Update();
  pPolyData = connectFilter->GetOutput();
  emit Done();
  // cleaner = vtk->vtkCleanPolyData();
  // cleaner->SetInputConnection(connectFilter->GetOutputPort());
  // cleaner = connectFilter; // TODO: Make this work all the time
  // cleaner = stripper

  //  auto writer = vtk->vtkXMLPolyDataWriter();
  //  writer->SetFileName(oFileName);
  //  writer->SetInputConnection(cleaner->GetOutputPort());
  //  writer->Write();


}

void SurfRunner::externalRun() {
#ifdef WIN32
    // Really really ugly
    std::stringstream ss;
    int dirnameLength = 0;
    int length = wai_getExecutablePath(NULL, 0, &dirnameLength);
    char *path = (char *)malloc(length + 1);

    wai_getModulePath(path, length, &dirnameLength);

    path[length] = '\0';

    if (dirnameLength + 2 < length) {
        path[dirnameLength + 1] = '\0';
    }

    ss << "C:/ProgramData/Anaconda2/Scripts/activate.bat && ";
    ss << "activate Python37 && ";
    ss << "C:/ProgramData/Anaconda2/envs/Python37/python.exe ";
    ss << path;
    ss << "/";
    ss << "cmdVesselSurf.py";
    std::cout << ss.str() << std::endl;
    system(ss.str().c_str());
#else
    system("/home/jmh/.virtualenv/bin/python cmdVesselSurf.py");
#endif
    std::cout << "Surfacing done" << std::endl;

    QMetaObject::invokeMethod(receiver, "updateSegProgressBar",
        Qt::QueuedConnection,
        Q_ARG(int, 100));

}

void SurfRunner::run() {
    return internalRun();
}

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */

