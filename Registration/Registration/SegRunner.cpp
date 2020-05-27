#include <QDebug>
#include <Registration/config.h>
#include <Registration/SegRunner.hpp>
#include <Registration/whereami.h>
#include <iostream>
#include <sstream>

#if INTEGRATED_SEGMENTATION
# include <itkVTKImageToImageFilter.h>
# include <itkCurvatureFlowImageFilter.h>
# include <itkCastImageFilter.h>
# include <itkConnectedThresholdImageFilter.h>
# include <itkImage.h>
# include <itkImageFileReader.h>
# include <itkImageFileWriter.h>
# include <itkMetaImageIO.h>
# include <itkMacro.h>
#endif

#ifdef INTEGRATED_SURFACING
# include <itkImageToVTKImageFilter.h>
#endif

#if INTEGRATED_SEGMENTATION

template <typename T>
void SegmentationConnectedThreshold(vtkImageData* pData,
                                    vtkImageData* pOutData,
                                    int inputSeed[3],
                                    int thresholds[2],
                                    const char* fOutput) {


  constexpr unsigned int Dimension = 3;

  using InternalVoxelType = T; // signed short;
  using InternalImageType = itk::Image<InternalVoxelType, Dimension>;

  using InputVoxelType = T; //signed short;
  using InputImageType = itk::Image<InputVoxelType, Dimension>;

  using VTK2ITKFilterType = itk::VTKImageToImageFilter<InputImageType>;

  using OutputVoxelType = unsigned char;
  using OutputImageType = itk::Image<OutputVoxelType, Dimension>;
  using CastingFilterType = itk::CastImageFilter<InternalImageType, OutputImageType>;

  using WriterType = itk::ImageFileWriter<OutputImageType>;

  using ConnectedThresholdImageFilterType = itk::ConnectedThresholdImageFilter<InternalImageType, InternalImageType>;
  
  using ConnectedFilterType =
      itk::ConnectedThresholdImageFilter<InternalImageType, InternalImageType>;

  typename CastingFilterType::Pointer caster = CastingFilterType::New();

#ifdef INTEGRATED_SURFACING
  // For converting from ITK to VTK
  using ITK2VTKFilterType = itk::ImageToVTKImageFilter<OutputImageType>;
  ITK2VTKFilterType::Pointer itk2vtkFilter = ITK2VTKFilterType::New();
#endif

  // Used for converting from VTK to ITK
  VTK2ITKFilterType::Pointer vtk2itkFilter = VTK2ITKFilterType::New();
  vtk2itkFilter->SetInput(pData);

  InternalVoxelType lowerThreshold = thresholds[0];
  InternalVoxelType upperThreshold = thresholds[1];

  typename InternalImageType::IndexType seed = { inputSeed[0],
                                                 inputSeed[1],
                                                 inputSeed[2] };

  typename ConnectedFilterType::Pointer connectedThreshold = ConnectedFilterType::New();

  connectedThreshold->SetInput(vtk2itkFilter->GetOutput());

  caster->SetInput(connectedThreshold->GetOutput());

  connectedThreshold->SetLower(lowerThreshold);
  connectedThreshold->SetUpper(upperThreshold);
  connectedThreshold->SetSeed(seed);

#ifndef INTEGRATED_SURFACING
  
  typename WriterType::Pointer writer = WriterType::New();

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
  ss << fOutput;

  writer->SetFileName(ss.str().c_str());
  writer->SetInput(caster->GetOutput());

  try {
    writer->Update();
  } catch (const itk::ExceptionObject& excep) {
    std::cerr << "Exception caught !" << std::endl;
    std::cerr << const_cast<itk::ExceptionObject&>(excep) << std::endl;
  }
#else
  itk2vtkFilter->SetInput(caster->GetOutput());

  try {
    itk2vtkFilter->Update();
  } catch (itk::ExceptionObject& excep) {
    std::cerr << "Exception caught !" << std::endl;
    std::cerr << const_cast<itk::ExceptionObject&>(excep) << std::endl;
  }

  pOutData = itk2vtkFilter->GetOutput();

  int imageDims[3];
  pOutData->GetDimensions(imageDims);

  // Display dimensions
  std::cout << "Segmentation dimensions: " << imageDims[0] << " "
      << imageDims[1] << " "
      << imageDims[2] << std::endl;
#endif

#endif

}

SegRunner::SegRunner(QWidget* receiver,
                     QMap<QString, QVariant> data,
                     vtkImageData* pData,
                     vtkImageData* pOutData,
                     int* retval,
                     volatile bool* stopped) : QRunnable(), receiver(receiver),
  pData(pData), pOutData(pOutData), retval(retval), stopped(stopped), data(data) {
}

void SegRunner::externalRun() {
    const char* fInput = nullptr;

    QString fileInput = data.value("finput").toString();

    const char* fOutput = "seg.mhd";

    int low = data["low"].toInt();
    int high = data["high"].toInt();

    int seedX = data["seedX"].toInt();
    int seedY = data["seedY"].toInt();
    int seedZ = data["seedZ"].toInt();

    std::cout << "low: " << low << std::endl;
    std::cout << "high: " << high << std::endl;

    std::cout << "seed: (" << seedX << ", " << seedY << "," << seedZ << ")" << std::endl;

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
#ifdef WIN32
    ss << "SegmGrow.exe ";
#else
    ss << "SegmGrow ";
#endif
    ss << fileInput.toStdString();
    ss << " ";
    ss << path;
    ss << fOutput;
    ss << " ";
    ss << seedX << " " << seedY << " " << seedZ;
    ss << " ";
    ss << low << " " << high;
    std::cout << ss.str() << std::endl;
    system(ss.str().data());
    free(path);
    std::cout << "Segmentation done" << std::endl;

    // *stopped = true; // Replace with atomic and set this to true when cancel is pressed

    QMetaObject::invokeMethod(receiver, "updateSegProgressBar",
        Qt::QueuedConnection,
        Q_ARG(int, 100));
}

void SegRunner::internalRun() {
    // Only works on signed/unsigned short
    assert(pData->GetScalarType() == 4 || pData->GetScalarType() == 5);

    int inputSeed[3];
    int thresholds[2];

    const char* fOutput = "seg.mhd";

    thresholds[0] = data["low"].toInt();
    thresholds[1] = data["high"].toInt();

    inputSeed[0] = data["seedX"].toInt();
    inputSeed[1] = data["seedY"].toInt();
    inputSeed[2] = data["seedZ"].toInt();


    if (pData->GetScalarType() == 4) {
        // Signed short
        SegmentationConnectedThreshold<signed short>(pData,
            pOutData,
            inputSeed,
            thresholds,
            fOutput);
    }  else if (pData->GetScalarType() == 5) {
        // Unsigned short
        SegmentationConnectedThreshold<unsigned short>(pData,
            pOutData,
            inputSeed,
            thresholds,
            fOutput);
    }

    std::cout << "Segmentation done" << std::endl;

    // *stopped = true; // Replace with atomic and set this to true when cancel is pressed

    QMetaObject::invokeMethod(receiver, "updateSegProgressBar",
        Qt::QueuedConnection,
        Q_ARG(int, 100));



}


void SegRunner::run() {
//    return this->externalRun();
    return this->internalRun();
}

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */

