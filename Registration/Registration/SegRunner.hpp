#pragma once

#include <cstdint>

#include <QRunnable>
#include <QProgressBar>

#include <QString>
#include <QVariant>
#include <QMap>

class SegRunner : public QRunnable {
 public:
  SegRunner(QWidget *receiver,
            QMap<QString, QVariant> data,
            int* retval,
            volatile bool *stopped);
  void run();
 private:
  QWidget* receiver;
  int* retval;
  volatile bool* stopped;
  QMap<QString, QVariant> data;
};
