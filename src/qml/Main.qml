import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: root

    minimumWidth: Kirigami.Units.gridUnit * 70
    minimumHeight: Kirigami.Units.gridUnit * 45
    width: minimumWidth
    height: minimumHeight

    title: "Genexis"

    pageStack.initialPage: Kirigami.Page {
        padding: 0
        
        Controls.SwipeView {
            id: swipeView
            anchors.fill: parent
            clip: true
            currentIndex: footerTabBar.currentIndex
            onCurrentIndexChanged: footerTabBar.currentIndex = currentIndex

            Loader {
                id: homeLoader
                active: true
                source: "pages/Home.qml"
            }
            Loader {
                id: buildLoader
                active: true
                source: "pages/Build.qml"
            }
            Loader {
                id: nandFlashLoader
                active: true
                source: "pages/NandFlash.qml"
            }
            Loader {
                id: timingFlashLoader
                active: true
                source: "pages/TimingFlash.qml"
            }
            Loader {
                id: settingsLoader
                active: true
                source: "pages/Settings.qml"
            }
        }

        footer: Kirigami.NavigationTabBar {
            id: footerTabBar
            actions: [
                Kirigami.Action {
                    icon.name: "go-home"
                    text: qsTr("Home")
                    checked: true
                    onTriggered: swipeView.currentIndex = 0
                },
                Kirigami.Action {
                    icon.name: "run-build"
                    text: qsTr("Build")
                    onTriggered: swipeView.currentIndex = 1
                },
                Kirigami.Action {
                    icon.name: "drive-harddisk"
                    text: qsTr("Flash")
                    onTriggered: swipeView.currentIndex = 2
                },
                Kirigami.Action {
                    icon.name: "chronometer"
                    text: qsTr("Timing")
                    onTriggered: swipeView.currentIndex = 3
                },
                Kirigami.Action {
                    icon.name: "settings-configure"
                    text: qsTr("Settings")
                    onTriggered: swipeView.currentIndex = 4
                }
            ]
        }
    }
}
