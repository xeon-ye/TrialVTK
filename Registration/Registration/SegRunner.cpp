#include <Registration/SegRunner.hpp>
#include <iostream>
#include <QDebug>

SegRunner::SegRunner(QWidget *receiver,
                     QMap<QString, QVariant> data,
                     int* retval,
                     volatile bool *stopped) :
  QRunnable(), receiver(receiver),
  retval(retval), stopped(stopped), data(data) {
}

void SegRunner::run() {
  //*stopped = true;
  // Signal complete (just in case)
  qDebug() << "convert";
  system("./SegmGrow");
  std::cout << "Segmentation done" << std::endl;

  std::cout << data["seedX"].toInt() << " " << data["seedY"].toInt() << " " << data["seedZ"].toInt() << std::endl;

  double low = data["mean"].toFloat() - 2.5 * data["std"].toFloat();
  double high = data["mean"].toFloat() + 2.5 * data["std"].toFloat();

  // hard-coded
  low = 168;
  high = 576;

  std::cout << "low: " << low << std::endl;
  std::cout << "high: " << high << std::endl;

  QMetaObject::invokeMethod(receiver, "updateSegProgressBar",
                            Qt::QueuedConnection,
                            Q_ARG(int, 100));
}

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
