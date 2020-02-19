#include "datamanager.h"
#include <SlicerWidget/ui_datamanager.h>

#include "QVTKOpenGLWidget.h"

DataManager::DataManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataManager) {
    ui->setupUi(this);
}

DataManager::~DataManager() {
    delete ui;
}
