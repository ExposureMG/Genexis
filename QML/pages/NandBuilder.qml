import QtQuick
import org.kde.kirigami as Kirigami
import "../components"

Kirigami.ScrollablePage {
    id: root

    title: qsTr("NAND Builder")

    NandBuilderTabs {
        anchors.fill: parent
    }
}
