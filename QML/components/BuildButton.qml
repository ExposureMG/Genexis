import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

QQC2.Button {
    text: qsTr("Build Image")
    icon.name: "system-run"
    highlighted: true
    Layout.fillWidth: true
    Layout.maximumWidth: Kirigami.Units.gridUnit * 16
    Layout.alignment: Qt.AlignHCenter
}
