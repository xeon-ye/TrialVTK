int main(int argc, char** argv) {
  vtkNew<vtkDICOMImageReader> reader;
  vtkNew<vtkResliceImageViewer> resliceImageViewer;
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  vtkNew<vtkInteractorStyleImage> style;
  vtkNew<vtkRenderWindow> renderWindow;
  vtkNew<vtkRenderer> renderer;
  vtkSmartPointer<vtkImageData> dcmImage;

  renderWindow->AddRenderer(renderer);
  renderer->SetBackground(0.0, 0.0, 0.0);
  renderWindowInteractor->SetInteractorStyle(style);

  //load dicom
  reader->SetDirectoryName("<path_to_dicom_series>");
  reader->Update();
  dcmImage = reader->GetOutput();

  resliceImageViewer->SetInputData(dcmImage);
  resliceImageViewer->SetRenderWindow(renderWindow);
  resliceImageViewer->SetupInteractor(renderWindowInteractor);
  resliceImageViewer->SetResliceModeToAxisAligned();
  resliceImageViewer->SetSlice(50);
  resliceImageViewer->SetColorLevel(-27);
  resliceImageViewer->SetColorWindow(1358);

  // Create mask volume
  vtkSmartPointer<vtkImageData> maskImage =
    vtkSmartPointer<vtkImageData>::New();
  int extent[6];

  dcmImage->GetExtent(extent);
  int xDim = extent[1] - extent[0] + 1;
  int yDim = extent[3] - extent[2] + 1;
  int zDim = extent[5] - extent[4] + 1;

  maskImage->SetSpacing(dcmImage->GetSpacing());
  maskImage->SetDimensions(xDim, yDim, zDim);
  maskImage->AllocateScalars(VTK_DOUBLE, 1);

  double scalarvalue = 0.0;

  // colour mask
  for (int y = extent[2]; y < extent[3]; y++) {
    for (int x = extent[0]; x < extent[1]; x++) {
      for (int z = extent[4]; z < extent[5]; z++) {
        double* pixel =
          static_cast<double*>(maskImage->GetScalarPointer(x, y, z));

        pixel[0] = scalarvalue;
        scalarvalue += 1.0;
      }
      scalarvalue = 0.0;
    }
    scalarvalue = 0.0;
  }

  // create lookup table
  vtkSmartPointer<vtkLookupTable> lookupTable =
    vtkSmartPointer<vtkLookupTable>::New();
  lookupTable->SetNumberOfTableValues(zDim);
  lookupTable->SetRange(0, zDim-1);
  lookupTable->Build();

  // get slice axes and set pass-through point
  vtkSmartPointer<vtkMatrix4x4> resliceAxes =
    vtkSmartPointer<vtkMatrix4x4>::New();

  resliceAxes->DeepCopy(resliceImageViewer->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetResliceAxes()->GetData());
  resliceAxes->SetElement(0, 3, 0);
  resliceAxes->SetElement(1, 3, 0);
  resliceAxes->SetElement(2, 3, 100);

  // extract slice
  vtkSmartPointer<vtkImageReslice> reslice =
    vtkSmartPointer<vtkImageReslice>::New();
  reslice->SetInputData(maskImage);
  reslice->SetOutputDimensionality(2);
  reslice->SetResliceAxes(resliceAxes);
  reslice->SetInterpolationModeToLinear();
  reslice->SetOutputSpacing(maskImage->GetSpacing());

  // map colours
  vtkSmartPointer<vtkImageMapToColors> mapTransparency2 =
    vtkSmartPointer<vtkImageMapToColors>::New();
  mapTransparency2->SetLookupTable(lookupTable);
  mapTransparency2->PassAlphaToOutputOn();
  mapTransparency2->SetInputConnection(reslice->GetOutputPort());

  // setup image actor for slice
  vtkSmartPointer<vtkImageActor> maskActor =
    vtkSmartPointer<vtkImageActor>::New();

  maskActor->GetMapper()->SetInputConnection(mapTransparency2->GetOutputPort());

  double pos[3];
  maskActor->GetPosition(pos);
  pos[2] = 55;
  maskActor->SetPosition(pos);
  maskActor->SetOpacity(0.5);

  resliceImageViewer->GetRenderer()->AddActor(maskActor);
  resliceImageViewer->GetRenderer()->ResetCamera();

  renderWindowInteractor->Start();

  return 0;

}
