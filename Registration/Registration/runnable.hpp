#pragma once

#include <cstdint>

#include <QRunnable>
#include <QProgressBar>

#include <QString>
#include <QVariant>
#include <QMap>

class RegRunner : public QRunnable {
 public:
  RegRunner(QWidget *receiver,
            QMap<QString, QVariant> data,
            const uint8_t* pData0,
            const uint8_t* pData1,
            int* retval,
            volatile bool *stopped);
  void run();
 private:
  QWidget* receiver;
  const uint8_t* pData0;
  const uint8_t* pData1;
  int* retval;
  volatile bool* stopped;
  QMap<QString, QVariant> data;
};
