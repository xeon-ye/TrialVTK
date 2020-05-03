#include <Registration/SurfRunner.hpp>
#include <iostream>
#include <QDebug>

SurfRunner::SurfRunner(QWidget *receiver,
                     QMap<QString, QVariant> data,
                     int* retval,
                     volatile bool *stopped) :
  QRunnable(), receiver(receiver),
  retval(retval), stopped(stopped), data(data) {
}

void SurfRunner::run() {
  //*stopped = true;
  // Signal complete (just in case)
  system("/home/jmh/.virtualenv/bin/python cmdVesselSurf.py");
  std::cout << "Surfacing done" << std::endl;

  QMetaObject::invokeMethod(receiver, "updateSegProgressBar",
                            Qt::QueuedConnection,
                            Q_ARG(int, 100));
}

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
