import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("Hello QCustomPlot in QML")
    header: TabBar {
        id: tabBar
        currentIndex: 0
        TabButton {
            text: qsTr("Open File")
        }
        TabButton {
            text: qsTr("Info")
        }
    }
}
