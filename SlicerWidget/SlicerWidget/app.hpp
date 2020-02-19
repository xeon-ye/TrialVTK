#pragma once

#include <QMainWindow>
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
 private Q_SLOTS:
  void onLoadClicked();

 private:
  void SetupUI();
  void PopulateMenus();

  // Designer form
  Ui_App *ui;

  DataManager* datamanager;
  QHBoxLayout* horizontalLayout;
};
