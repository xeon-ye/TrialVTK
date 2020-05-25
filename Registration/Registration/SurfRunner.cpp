#include <QDebug>
#include <Registration/SurfRunner.hpp>
#include <Registration/whereami.h>
#include <iostream>
#include <sstream>
SurfRunner::SurfRunner(QWidget *receiver,
    QMap<QString, QVariant> data,
    int *retval,
    volatile bool *stopped) : QRunnable(), receiver(receiver),
                              retval(retval), stopped(stopped), data(data) {
}

void SurfRunner::run() {
    //*stopped = true;
    // Signal complete (just in case)

#ifdef WIN32
    // Really really ugly
    std::stringstream ss;
    int dirnameLength = 0;
    int length = wai_getExecutablePath(NULL, 0, &dirnameLength);
    char *path = (char *)malloc(length + 1);

    wai_getModulePath(path, length, &dirnameLength);

    path[length] = '\0';

    if (dirnameLength + 2 < length) {
        path[dirnameLength + 1] = '\0';
    }

    ss << "C:/ProgramData/Anaconda2/Scripts/activate.bat && ";
    ss << "activate Python37 && ";
    ss << "C:/ProgramData/Anaconda2/envs/Python37/python.exe ";
    ss << path;
    ss << "/";
    ss << "cmdVesselSurf.py";
    std::cout << ss.str() << std::endl;
    system(ss.str().c_str());
#else
    system("/home/jmh/.virtualenv/bin/python cmdVesselSurf.py");
#endif
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
