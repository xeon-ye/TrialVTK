#include <cstdlib>


#include <itkConnectedThresholdImageFilter.h>
#include <itkImage.h>
#include <itkCastImageFilter.h>
#include "itkCurvatureFlowImageFilter.h"

#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>

#include <itkMacro.h>

int main(int argc, char* argv[]) {
  const char* fInput = "/home/jmh/bkmedical/data/CT/CT-Abdomen.mhd";
  const char* fOutput = "./seg.mhd";
  //int iSeed[3] = {185,217,407};
  //signed short limits[2] = {168, 576};

  constexpr unsigned int Dimension = 3;
  using InternalVoxelType = signed short;
  using InternalImageType = itk::Image<InternalVoxelType, Dimension>;

  using InputVoxelType = signed short;
  using InputImageType = itk::Image<InputVoxelType, Dimension>;

  using OutputVoxelType = unsigned char;
  using OutputImageType = itk::Image<OutputVoxelType, Dimension>;
  using CastingFilterType = itk::CastImageFilter<InternalImageType, OutputImageType>;
  CastingFilterType::Pointer caster = CastingFilterType::New();


  using ReaderType = itk::ImageFileReader<InputImageType>;
  using WriterType = itk::ImageFileWriter<OutputImageType>;

  using ConnectedThresholdImageFilterType = itk::ConnectedThresholdImageFilter<InternalImageType, InternalImageType>;

  InternalVoxelType lowerThreshold = 168;
  InternalVoxelType upperThreshold = 576;

  InternalImageType::IndexType seed = {185,217,407};

  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();

  // TODO: Consider smoothing before applying connected threshold

  if (argc > 1) {
    fInput = argv[1];
  }
  if (argc > 2) {
    fOutput = argv[2];
  }
  if (argc > 3 && argc > 5) {
    seed[0] = atoi(argv[3]);
    seed[1] = atoi(argv[4]);
    seed[2] = atoi(argv[5]);
    if (argc > 6 && argc > 7) {
      lowerThreshold = atoi(argv[6]);
      upperThreshold = atoi(argv[7]);
    }
    else {
      std::cerr << "Usage: " << argv[0];
      std::cerr << " inputImage outputImage seedX seedY seedZ lowerThreshold upperThreshold" << std::endl;
    }
  }

  reader->SetFileName(fInput);
  writer->SetFileName(fOutput);

  using ConnectedFilterType =
      itk::ConnectedThresholdImageFilter<InternalImageType, InternalImageType>;


  ConnectedFilterType::Pointer connectedThreshold = ConnectedFilterType::New();

#if 0
  // Pre-smoothing
  using CurvatureFlowImageFilterType =
      itk::CurvatureFlowImageFilter<InternalImageType, InternalImageType>;
  CurvatureFlowImageFilterType::Pointer smoothing = CurvatureFlowImageFilterType::New();
  smoothing->SetInput(reader->GetOutput());
  connectedThreshold->SetInput(smoothing->GetOutput());
  smoothing->SetNumberOfIterations(5);
  smoothing->SetTimeStep(0.125);
#else
  connectedThreshold->SetInput(reader->GetOutput());
#endif
  caster->SetInput(connectedThreshold->GetOutput());
  writer->SetInput(caster->GetOutput());

  connectedThreshold->SetLower(lowerThreshold);
  connectedThreshold->SetUpper(upperThreshold);
  connectedThreshold->SetSeed(seed);
  // connectedThreshold->SetReplaceValue(255);
  try
  {
    writer->Update();
  }
  catch (const itk::ExceptionObject& excep)
  {
    std::cerr << "Exception caught !" << std::endl;
    std::cerr << const_cast<itk::ExceptionObject&>(excep) << std::endl;
  }

  return EXIT_SUCCESS;
}
