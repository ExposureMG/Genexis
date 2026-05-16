import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: root

    minimumWidth: Kirigami.Units.gridUnit * 70
    minimumHeight: Kirigami.Units.gridUnit * 50
    width: minimumWidth
    height: minimumHeight

    title: "Genexis"

    // Combined Row for Centered Tabs and Right-Aligned Settings Action
    header: Rectangle {
        implicitHeight: Math.max(settingsAction.implicitHeight, bar.implicitHeight) + Kirigami.Units.smallSpacing * 2
        color: Kirigami.Theme.backgroundColor
        
        // Navigation Tabs centered in the window
        Controls.TabBar {
            id: bar
            anchors.centerIn: parent
            width: implicitWidth
            currentIndex: 0

            contentItem: ListView {
                model: bar.contentModel
                currentIndex: bar.currentIndex
                orientation: ListView.Horizontal
                spacing: Kirigami.Units.largeSpacing
                boundsBehavior: Flickable.StopAtBounds
                snapMode: ListView.SnapToItem
            }

            Controls.TabButton {
                text: qsTr("Home")
                width: implicitWidth
            }
            Controls.TabButton {
                text: qsTr("gxBuild")
                width: implicitWidth
            }
        }

        // Single Settings action button aligned to the right
        Kirigami.ActionToolBar {
            id: settingsAction
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: Kirigami.Units.smallSpacing
            
            actions: [
                Kirigami.Action {
                    text: qsTr("Settings")
                    icon.name: "settings-configure"
                    onTriggered: {
                        root.showPassiveNotification(qsTr("Settings clicked"))
                    }
                }
            ]
        }
    }

    pageStack.initialPage: Kirigami.Page {
        padding: 0
        
        StackLayout {
            anchors.fill: parent
            currentIndex: bar.currentIndex

            Loader {
                id: homeLoader
                active: true
                source: "pages/Home.qml"
            }
            Loader {
                id: gxBuildLoader
                active: true
                source: "pages/GxBuild.qml"
            }
        }
    }
}
