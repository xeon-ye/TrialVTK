#include <QDebug>
#include <Registration/SegRunner.hpp>
#include <Registration/whereami.h>
#include <iostream>
#include <sstream>

SegRunner::SegRunner(QWidget* receiver,
    QMap<QString, QVariant> data,
    int* retval,
    volatile bool* stopped) : QRunnable(), receiver(receiver),
                              retval(retval), stopped(stopped), data(data) {
}

void SegRunner::run() {
    const char* fInput = nullptr;

    QString fileInput = data.value("finput").toString();
    //"/home/jmh/bkmedical/data/CT/CT-Abdomen.mhd";

    const char* fOutput = "seg.mhd";

    int low = data["low"].toInt();
    int high = data["high"].toInt();

    int seedX = data["seedX"].toInt();
    int seedY = data["seedY"].toInt();
    int seedZ = data["seedZ"].toInt();

    std::cout << "low: " << low << std::endl;
    std::cout << "high: " << high << std::endl;

    std::cout << "seed: (" << seedX << ", " << seedY << "," << seedZ << ")" << std::endl;

#if 0
  // hard-coded
  low = 168;
  high = 576;
  seedX = 185;
  seedY = 217;
  seedZ = 407;
#endif

    std::stringstream ss;
    int dirnameLength = 0;
    int length = wai_getExecutablePath(NULL, 0, &dirnameLength);
    char* path = (char*)malloc(length + 1);

    wai_getModulePath(path, length, &dirnameLength);

    path[length] = '\0';

    if (dirnameLength + 1 < length) {
        path[dirnameLength] = '\0';
    }
    ss << path;
#ifdef WIN32
    ss << "\\SegmGrow.exe ";
#else
    ss << "./SegmGrow ";
#endif
    ss << fileInput.toStdString();
    ss << " ";
    ss << path;
#ifdef WIN32
    ss << "\\";
#else
    ss << "/";
#endif
    ss << fOutput;
    ss << " ";
    ss << seedX << " " << seedY << " " << seedZ;
    ss << " ";
    ss << low << " " << high;
    std::cout << ss.str() << std::endl;
    system(ss.str().data());

    // Old works
    // system("./SegmGrow");
    std::cout << "Segmentation done" << std::endl;

    // *stopped = true; // only if cancelled

    QMetaObject::invokeMethod(receiver, "updateSegProgressBar",
        Qt::QueuedConnection,
        Q_ARG(int, 100));
}

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
