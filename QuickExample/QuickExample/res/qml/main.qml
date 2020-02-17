import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.0

Window {
    id: root
    visible: true
    height: 1280
    width: 1024
    SplitView {
        anchors.fill : parent
        orientation: Qt.Horizontal
        Rectangle {
            width : 200
            Layout.maximumWidth: 400
            color: "red"
            Text {
                text: "View 1"
                anchors.centerIn: parent
            }
        }
        Rectangle {
            id: centerItem
            Layout.minimumWidth: 50
            Layout.fillWidth: true
            color: "lightgray"
            Text {
                text: "View 2"
                anchors.centerIn: parent
            }
        }
        Rectangle {
            width: 200
            color: "lightgreen"
            Text {
                text: "View 3"
                anchors.centerIn: parent
            }
        }
    }
}
