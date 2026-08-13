import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.Page {
    id: root

    title: qsTr("NAND Info")

    ListModel {
        id: subPageModel
        ListElement {
            name: "Overview"
            pageUrl: "nand/Overview.qml"
        }
        ListElement {
            name: "Components"
            pageUrl: "nand/Components.qml"
        }
        ListElement {
            name: "Keyvault"
            pageUrl: "nand/Keyvault.qml"
        }
    }

    header: QQC2.TabBar {
        id: tabBar
        currentIndex: 0

        Repeater {
            model: subPageModel
            QQC2.TabButton {
                required property string name
                text: name
            }
        }
    }

    Loader {
        id: pageLoader
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        source: subPageModel.get(tabBar.currentIndex).pageUrl

        Connections {
            target: pageLoader.item
            ignoreUnknownSignals: true
            function onKeyvaultRequested() {
                tabBar.currentIndex = 2;
            }
        }
    }
}
