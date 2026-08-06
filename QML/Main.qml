import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import "components"

Kirigami.ApplicationWindow {
    id: root

    title: qsTr("Genexis")
    width: 800
    height: 720
    minimumWidth: 380
    minimumHeight: 480
    maximumWidth: 1280

    // Suppress Kirigami's per-page header — our StatusBar replaces it
    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.None

    // Universal top status/title bar
    header: StatusBar {
        id: statusBar

        pageTitle: root.pageStack.currentItem ? root.pageStack.currentItem.title : qsTr("Genexis")

        onMenuRequested: root.globalDrawer.drawerOpen = !root.globalDrawer.drawerOpen
    }

    // Overlay navigation drawer — hamburger in StatusBar is the only trigger
    globalDrawer: Kirigami.GlobalDrawer {
        id: navDrawer

        title: qsTr("Genexis")
        modal: true
        width: Kirigami.Units.gridUnit * 14
        handleVisible: false

        actions: [
            Kirigami.Action {
                text: qsTr("Home")
                onTriggered: root.switchPage("pages/Home.qml")
            },
            Kirigami.Action {
                text: qsTr("NAND Info")
                onTriggered: root.switchPage("pages/Nand.qml")
            },
            Kirigami.Action {
                text: qsTr("NAND Builder")
                onTriggered: root.switchPage("pages/NandBuilder.qml")
            },
            Kirigami.Action {
                text: qsTr("BadUpdate")
                onTriggered: root.switchPage("pages/BadUpdate.qml")
            },
            Kirigami.Action {
                text: qsTr("Flasher")
                onTriggered: root.switchPage("pages/Flasher.qml")
            }
        ]

        footer: ColumnLayout {
            spacing: 0
            width: parent ? parent.width : undefined

            Kirigami.Separator {
                Layout.fillWidth: true
            }

            QQC2.ItemDelegate {
                text: qsTr("Settings")
                Layout.fillWidth: true
                onClicked: root.switchPage("pages/Settings.qml")
            }
        }
    }

    // Replace pageStack content and close the drawer
    function switchPage(url) {
        pageStack.clear();
        pageStack.push(Qt.resolvedUrl(url));
        navDrawer.drawerOpen = false;
    }

    Component.onCompleted: {
        pageStack.push(Qt.resolvedUrl("pages/Home.qml"));
    }
}
