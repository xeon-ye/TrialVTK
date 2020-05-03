import sys
import os
import numpy as np

# sitk.LabelOverlay
# sitk.ConnectedThreshold
# sitk.BinaryMorphologicalClosing
# sitk.ConfidenceConnected


from matplotlib import pyplot as plt

plt.ion()
plt.close('all')

import SimpleITK as sitk

from timeit import default_timer as timer

if os.name == 'nt':
  sys.path.append('e:/github/SimpleITK-Notebooks/Utilities')
else:
  sys.path.append('/home/jmh/github/ITK/SimpleITK-Notebooks/Utilities')

from myshow import myshow, myshow3d

if os.name == 'nt':
  img16 = sitk.ReadImage('e:/datavtk/CT/CT-Abdomen.mhd')
else:
  img16 = sitk.ReadImage('/home/jmh/bkmedical/data/CT/CT-Abdomen.mhd')

# itk::IntensityWindowingImageFilter

# SetWindowLevel(50,145)
#img255 = sitk.Cast(sitk.RescaleIntensity(img),sitk.sitkUInt8)

level = 145
window = 55 # 50

imax = level + 0.5*window
imin = level - 0.5*window

ifilter = sitk.IntensityWindowingImageFilter()
ifilter.SetWindowMaximum(imax)
ifilter.SetWindowMinimum(imin)
img = ifilter.Execute(img16)

img255 = sitk.Cast(sitk.RescaleIntensity(img), sitk.sitkUInt8)

#img255 = sitk.Cast(img, sitk.sitkUInt16)

size = img16.GetSize()

#myshow3d(img255, xslices=range(50, size[0] - 50, 30), title='CT')

seed = (185, 217, 407)

seg = sitk.Image(img16.GetSize(), sitk.sitkUInt8)
seg.CopyInformation(img)
seg[seed] = 1

seg = sitk.BinaryDilate(seg, 3)

# sitk.LabelOverlay(img1, sitk.LabelContour(img1_seg), 1.0)

# ImageToVTKImageFilter[itk.Image[itk.UC,3]].New()

if 1:
  myshow3d(sitk.LabelOverlay(img255, seg, colormap=[255,0,0]),
           xslices=range(seed[0], seed[0]+1), yslices=range(seed[1], seed[1]+1),
           zslices=range(seed[2], seed[2]+1), title="Initial Seed")

  # using FilterType = itk::BinaryMask3DMeshSource<ImageType, MeshType>;
  # FilterType::Pointer filter = FilterType::New();
  # filter->SetInput(threshold->GetOutput());
  # filter->SetObjectValue(255);
  #
  # using WriterType = itk::MeshFileWriter<MeshType>;
  # WriterType::Pointer writer = WriterType::New();
  # writer->SetFileName(outputFileName);
  # writer->SetInput(filter->GetOutput());

# 181 - 576 (MITK)
# 168 - 576

if 1:
  start = timer()
#  seg_con = sitk.ConnectedThreshold(img, seedList=[seed],
#                                    lower=150, upper=255)

  seg_con = sitk.ConnectedThreshold(img16, seedList=[seed],
                                    lower=168, upper=576)
  #seg_con255 = sitk.Cast(sitk.RescaleIntensity(seg_con), sitk.sitkUInt8)
  seg_con255 = sitk.Cast(seg_con, sitk.sitkUInt8)
  end = timer()
  print('\nConnected Threshold: Seconds elapsed: %f' % (end - start))

  if 1:
    myshow3d(sitk.LabelOverlay(img255, seg_con255, colormap = [255,0,0]),
             xslices=range(seed[0], seed[0]+1), yslices=range(seed[1], seed[1]+1),
             zslices=range(seed[2], seed[2]+1), title="Connected Threshold")


  if 1:
    myshow3d(img16,
             xslices=range(seed[0], seed[0]+1), yslices=range(seed[1], seed[1]+1),
             zslices=range(seed[2], seed[2]+1), title="Connected Threshold")


  if 0:
    vectorRadius = (3, 3, 3)
    kernel = sitk.sitkBall
    #bum = sitk.Cast(seg_con, sitk.sitkUInt8)
    start = timer()
    seg_clean = sitk.BinaryMorphologicalClosing(seg_con255,
                                                vectorRadius,
                                                kernel)
    end = timer()
    print('\nMorphological closing: Seconds elapsed: %f' % (end - start))

    myshow3d(sitk.LabelOverlay(img255, seg_clean, colormap=[255,0,0]),
             xslices=range(seed[0], seed[0]+1), yslices=range(seed[1], seed[1]+1),
             zslices=range(seed[2], seed[2]+1), title="Cleaned up segmentation")


#sys.exit(0)
seg_clean = seg_con255

# TODO: Save and create mesh using VTK
sitk.WriteImage(seg_clean, 'seg.mhd')






  # TODO: Make surface and show


  # Unlike in ``ConnectedThreshold``, you need not select the bounds in
  # ``ConfidenceConnected`` filter. Bounds are implicitly specified as
  # :math:`\mu\pm c\sigma`, where :math:`\mu` is the mean intensity of the seed
  # points, :math:`\sigma` their standard deviation and :math:`c` a user specified
  # constant.
  #
  # This algorithm has some flexibility which you should familiarize yourself with:
  #
  # * The ``multiplier`` parameter is the constant :math:`c` from the formula above.
  # * You can specify a region around each seed point ``initialNeighborhoodRadius``
  #   from which the statistics are estimated, see what happens when you set it to zero.
  # * The ``numberOfIterations`` allows you to rerun the algorithm. In the first
  #   run the bounds are defined by the seed voxels you specified, in the
  #   following iterations :math:`\mu` and :math:`\sigma` are estimated from
  #   the segmented points and the region growing is updated accordingly.

if 0:
  start = timer()
  seg_conf = sitk.ConfidenceConnected(img255, seedList=[seed],
                                      numberOfIterations=1,
                                      multiplier=2.5,
                                      initialNeighborhoodRadius=1,
                                      replaceValue=1)
  end = timer()
  print('\nConfidence connected: %f' % (end - start))

  if 0:
    vectorRadius = (1, 1, 1)
    kernel = sitk.sitkBall
    start = timer()
    seg_clean = sitk.BinaryMorphologicalClosing(seg_conf,
                                                vectorRadius,
                                                kernel)
    end = timer()
    print('\nMorphological Closing: %f' % (end - start))
  seg_clean = seg_conf
  myshow3d(sitk.LabelOverlay(img255, seg_clean),
           xslices=range(seed[0], seed[0]+1), yslices=range(seed[1], seed[1]+1),
           zslices=range(seed[2], seed[2]+1), title="Confidence Connected Threshold")


