#include <cstdlib>


#include <itkConnectedThresholdImageFilter.h>
#include <itkImage.h>
#include <itkCastImageFilter.h>
#include "itkCurvatureFlowImageFilter.h"

#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>

#include <itkMacro.h>

template <typename T>
void generate_segmentation(int argc, char* argv[]) {
  //void generate_segmentation(itk::ImageFileReader<ikt::Image<T> > reader) {

  const char* fOutput = "./seg.mhd";
  const char* fInput = "/home/jmh/bkmedical/data/CT/CT-Abdomen.mhd";

  constexpr unsigned int Dimension = 3;
  using InternalVoxelType = T;// signed short;
  using InternalImageType = itk::Image<InternalVoxelType, Dimension>;

  using InputVoxelType = T;//signed short;
  using InputImageType = itk::Image<InputVoxelType, Dimension>;

  using OutputVoxelType = unsigned char;
  using OutputImageType = itk::Image<OutputVoxelType, Dimension>;
  using CastingFilterType = itk::CastImageFilter<InternalImageType, OutputImageType>;
  typename CastingFilterType::Pointer caster = CastingFilterType::New();


  using ReaderType = itk::ImageFileReader<InputImageType>;
  using WriterType = itk::ImageFileWriter<OutputImageType>;

  using ConnectedThresholdImageFilterType = itk::ConnectedThresholdImageFilter<InternalImageType, InternalImageType>;

  InternalVoxelType lowerThreshold = 168;
  InternalVoxelType upperThreshold = 576;

  typename InternalImageType::IndexType seed = {185,217,407};

  typename ReaderType::Pointer reader = ReaderType::New();
  typename WriterType::Pointer writer = WriterType::New();

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
    if (argc > 6) {
      if (argc > 7) {
        lowerThreshold = atoi(argv[6]);
        upperThreshold = atoi(argv[7]);
      }
      else {
        std::cerr << "Usage: " << argv[0];
        std::cerr << " inputImage outputImage seedX seedY seedZ lowerThreshold upperThreshold" << std::endl;
      }
    }
  }

  std::cout << "Seed is: (" << seed[0] << ", " << seed[1] << ", " << seed[2] << ")" << std::endl;
  std::cout << "Thresholds are: [" << lowerThreshold << ", " << upperThreshold << "]" << std::endl;

  reader->SetFileName(fInput);
  writer->SetFileName(fOutput);

  using ConnectedFilterType =
      itk::ConnectedThresholdImageFilter<InternalImageType, InternalImageType>;


  typename ConnectedFilterType::Pointer connectedThreshold = ConnectedFilterType::New();

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

  // Orient filter
#if 0
  typedef itk::OrientedImage<dcmAPI::PixelType, 3> OrientedVolumeType;
  itk::OrientImageFilter<VolumeType,VolumeType>::Pointer orienter = itk::OrientImageFilter<VolumeType,VolumeType>::New();
  orienter->UseImageDirectionOn();
  orienter->SetDesiredCoordinateOrientation(
      itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LPS);
  orienter->SetInput(volume);
  orienter->Update();
#endif

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
}
#if 0
template <typename T>
void generate_segmentation2(itk::ImageFileReader<itk::Image<T> >::Pointer reader) {
  //void generate_segmentation(itk::ImageFileReader<ikt::Image<T> > reader) {

  const char* fOutput = "./seg.mhd";

  constexpr unsigned int Dimension = 3;
  using InternalVoxelType = T;// signed short;
  using InternalImageType = itk::Image<InternalVoxelType, Dimension>;

  using InputVoxelType = T;//signed short;
  using InputImageType = itk::Image<InputVoxelType, Dimension>;

  using OutputVoxelType = unsigned char;
  using OutputImageType = itk::Image<OutputVoxelType, Dimension>;
  using CastingFilterType = itk::CastImageFilter<InternalImageType, OutputImageType>;
  typename CastingFilterType::Pointer caster = CastingFilterType::New();


  using ReaderType = itk::ImageFileReader<InputImageType>;
  using WriterType = itk::ImageFileWriter<OutputImageType>;

  using ConnectedThresholdImageFilterType = itk::ConnectedThresholdImageFilter<InternalImageType, InternalImageType>;

  InternalVoxelType lowerThreshold = 168;
  InternalVoxelType upperThreshold = 576;

  typename InternalImageType::IndexType seed = {185,217,407};

  //  typename ReaderType::Pointer reader = ReaderType::New();
  typename WriterType::Pointer writer = WriterType::New();

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
    if (argc > 6) {
      if (argc > 7) {
        lowerThreshold = atoi(argv[6]);
        upperThreshold = atoi(argv[7]);
      }
      else {
        std::cerr << "Usage: " << argv[0];
        std::cerr << " inputImage outputImage seedX seedY seedZ lowerThreshold upperThreshold" << std::endl;
      }
    }
  }

  std::cout << "Seed is: (" << seed[0] << ", " << seed[1] << ", " << seed[2] << ")" << std::endl;
  std::cout << "Thresholds are: [" << lowerThreshold << ", " << upperThreshold << "]" << std::endl;

  reader->SetFileName(fInput);
  writer->SetFileName(fOutput);

  using ConnectedFilterType =
      itk::ConnectedThresholdImageFilter<InternalImageType, InternalImageType>;


  typename ConnectedFilterType::Pointer connectedThreshold = ConnectedFilterType::New();

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

  // Orient filter
#if 0
  typedef itk::OrientedImage<dcmAPI::PixelType, 3> OrientedVolumeType;
  itk::OrientImageFilter<VolumeType,VolumeType>::Pointer orienter = itk::OrientImageFilter<VolumeType,VolumeType>::New();
  orienter->UseImageDirectionOn();
  orienter->SetDesiredCoordinateOrientation(
      itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LPS);
  orienter->SetInput(volume);
  orienter->Update();
#endif

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
}
#endif

int main(int argc, char* argv[]) {
  //int iSeed[3] = {185,217,407};
  //signed short limits[2] = {168, 576};

  generate_segmentation<signed short>(argc, argv);

  return EXIT_SUCCESS;
}
