#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QWidget>

namespace Ui {
class DataManager;
}

class DataManager : public QWidget
{
    Q_OBJECT

public:
    explicit DataManager(QWidget *parent = 0);
    ~DataManager();

private:
    Ui::DataManager *ui;
};

#endif // DATAMANAGER_H