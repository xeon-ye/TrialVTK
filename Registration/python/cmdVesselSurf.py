# Number of algorithms: 7
# vtkImageThreshold
# vtkGaussianSmooth
# vtkMarchingCubes
# vtkWindowedSincPolyDataFilter
# vtkPolyDataNormals
# vtkPolyDataConnectivityFilter
# vtkPolyDataMapper

import vtk

fileName = './seg.mhd'
reader = vtk.vtkMetaImageReader()
reader.SetFileName(fileName)
reader.Update()

selectTissue = vtk.vtkImageThreshold()
selectTissue.ThresholdBetween(1, 2)
selectTissue.SetInValue(255)
selectTissue.SetOutValue(0)
selectTissue.SetInputConnection(reader.GetOutputPort())

from timeit import default_timer as timer

start = timer()
gaussianRadius = 1
gaussianStandardDeviation = 2.0
gaussian = vtk.vtkImageGaussianSmooth()
gaussian.SetStandardDeviations(gaussianStandardDeviation, gaussianStandardDeviation, gaussianStandardDeviation)
gaussian.SetRadiusFactors(gaussianRadius, gaussianRadius, gaussianRadius)
gaussian.SetInputConnection(selectTissue.GetOutputPort())

#isoValue = 20.5#127.5 # Was 127.5
isoValue = 127.5 # Was 127.5
mcubes = vtk.vtkMarchingCubes()
mcubes.SetInputConnection(gaussian.GetOutputPort())
mcubes.ComputeScalarsOff()
mcubes.ComputeGradientsOff()
mcubes.ComputeNormalsOff()
mcubes.SetValue(0, isoValue)

smoothingIterations = 5 # was 3
passBand = 0.001
featureAngle = 60.0
smoother = vtk.vtkWindowedSincPolyDataFilter()
smoother.SetInputConnection(mcubes.GetOutputPort())
smoother.SetNumberOfIterations(smoothingIterations)
smoother.BoundarySmoothingOff() #smoother.BoundarySmoothingOn()
smoother.FeatureEdgeSmoothingOff() # Turn off smoothing along sharp interior edges
smoother.SetFeatureAngle(featureAngle) # Angle to distinguish a sharp edge
smoother.SetPassBand(passBand)
smoother.NonManifoldSmoothingOn()
smoother.NormalizeCoordinatesOn()
smoother.Update()

end = timer()
print('\nSmoothing: Seconds elapsed: %f' % (end - start))

normals = vtk.vtkPolyDataNormals()
normals.SetInputConnection(smoother.GetOutputPort())
normals.SetFeatureAngle(featureAngle)

stripper = vtk.vtkStripper()
stripper.SetInputConnection(normals.GetOutputPort())

# Connectivity filter

connectFilter = vtk.vtkPolyDataConnectivityFilter()
connectFilter.SetInputConnection(stripper.GetOutputPort())
connectFilter.SetExtractionModeToSpecifiedRegions()
connectFilter.AddSpecifiedRegion(0)
connectFilter.Update();

#cleaner = vtk.vtkCleanPolyData()
#cleaner.SetInputConnection(connectFilter.GetOutputPort())
cleaner = connectFilter
cleaner = stripper

writer = vtk.vtkXMLPolyDataWriter()
writer.SetFileName("/home/jmh/bkmedical/data/CT/A.vtp")
writer.SetInputConnection(cleaner.GetOutputPort())
writer.Write()

# Convert from RAS to LPS
