#pragma once

#include <QMainWindow>
#include <QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <SlicerWidget/datamanager.h>

class Ui_App;

class App : public QMainWindow {
  Q_OBJECT
 public:
  App(int argc, char *argv[]);
  ~App() override;
 public slots:
  virtual void slotExit();
  void resizeEvent(QResizeEvent* event);

 private Q_SLOTS:
  void onLoadClicked();
  void referenceViewChanged(int index);

 private:
  void SetupUI();
  void PopulateMenus();

 public:
  // Designer form
  Ui_App *ui;

  QHBoxLayout* horizontalLayout;
  DataManager* datamanager;
  QComboBox* cb;
  int iSlice = 0;
};
