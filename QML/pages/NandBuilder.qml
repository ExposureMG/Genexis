import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import "../components"

Kirigami.Page {
    id: root

    title: qsTr("NAND Builder")

    NandBuilder {
        anchors.fill: parent
    }
}
