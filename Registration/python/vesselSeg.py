import sys
import numpy as np

import SimpleITK as sitk

sys.path.append('/home/jmh/github/ITK/SimpleITK-Notebooks/Utilities')
from myshow import myshow, myshow3d

img = sitk.ReadImage('/home/jmh/bkmedical/data/CT/CT-Abdomen.mhd')

# itk::IntensityWindowingImageFilter

# SetWindowLevel(50,145)
#img255 = sitk.Cast(sitk.RescaleIntensity(img),sitk.sitkUInt8)

level = 145
window = 50

imax = level + 0.5*window
imin = level - 0.5*window
print(imax)
print(imin)

img255 = sitk.Cast(sitk.RescaleIntensity(img, imin, imax),sitk.sitkUInt8)

if 0:
  filter = sitk.MinimumMaximumImageFilter()
  filter.Execute(img255)
  ImageHighestIntensity = filter.GetMaximum()
  ImageLowestIntensity = filter.GetMinimum()
#filter.Dispose()

# RescaleIntensityImageFilter (used by function above)
# range = max - min
# level = 0.5*(max + min)

f2 = sitk.IntensityWindowingImageFilter()

f2.SetWindowMaximum(170)
f2.SetWindowMinimum(120)
f2.Execute(img)

size = img.GetSize()
myshow3d(img255, xslices=range(50, size[0] - 50, 30), title='CT')

#sys.exit(0)

# -164;164   -560:-152

# seed = np.r_[-95.1441, -622.891, -249.269]

seed = (132, 142, 340) # 96

seg = sitk.Image(img.GetSize(), sitk.sitkUInt8)
seg.CopyInformation(img)
seg[seed] = 1

seg = sitk.BinaryDilate(seg, 3)

myshow3d(sitk.LabelOverlay(img255, seg),
         xslices=range(132, 133), yslices=range(142, 143),
         zslices=range(340, 341), title="Initial Seed")


seg_con = sitk.ConnectedThreshold(img, seedList=[seed],
                                  lower=100, upper=190)

myshow3d(sitk.LabelOverlay(img255, seg_con),
         xslices=range(132, 133), yslices=range(142, 143),
         zslices=range(340, 341), title="Connected Threshold")
