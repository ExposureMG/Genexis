import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.Page {
    id: root

    title: qsTr("NAND Info")

    readonly property var pageUrls: [
        "../components/nand/Overview.qml",
        "../components/nand/Components.qml",
        "../components/nand/Keyvault.qml"
    ]

    header: QQC2.TabBar {
        id: tabBar
        currentIndex: 0

        QQC2.TabButton {
            text: qsTr("Overview")
        }
        QQC2.TabButton {
            text: qsTr("Components")
        }
        QQC2.TabButton {
            text: qsTr("Keyvault")
        }
    }

    Loader {
        id: pageLoader
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        source: root.pageUrls[tabBar.currentIndex]

        Connections {
            target: pageLoader.item
            ignoreUnknownSignals: true
            function onKeyvaultRequested() {
                tabBar.currentIndex = 2;
            }
        }
    }
}
