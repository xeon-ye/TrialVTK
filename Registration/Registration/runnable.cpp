#include <Registration/runnable.hpp>

RegRunner::RegRunner(QWidget *receiver,
                     QMap<QString, QVariant> data,
                     const uint8_t* pData0,
                     const uint8_t* pData1,
                     int* retval,
                     volatile bool *stopped) :
  QRunnable(), receiver(receiver),
  pData0(pData0),
  pData1(pData1),
  retval(retval), stopped(stopped), data(data) {
}

void RegRunner::run() {
  //*stopped = true;
  // Signal complete (just in case)
  QMetaObject::invokeMethod(receiver, "updateProgressBar",
                            Qt::QueuedConnection,
                            Q_ARG(int, 100));
}

/* Local variables: */
/* indent-tab-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
