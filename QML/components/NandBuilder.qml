import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Item {
    id: root

    property int currentTabIndex: 0

    ColumnLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.smallSpacing

        // Centred TabBar Navigation
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.smallSpacing

            Item { Layout.fillWidth: true }

            QQC2.TabBar {
                id: navTabBar
                currentIndex: root.currentTabIndex
                onCurrentIndexChanged: root.currentTabIndex = currentIndex

                QQC2.TabButton {
                    text: qsTr("Simple")
                }
                QQC2.TabButton {
                    text: qsTr("Advanced")
                }
                QQC2.TabButton {
                    text: qsTr("Donor")
                }
            }

            Item { Layout.fillWidth: true }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        // Dynamic view container switching between Simple, Advanced, and Donor components
        Loader {
            id: builderLoader
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: {
                if (root.currentTabIndex === 0) return simpleComponent;
                if (root.currentTabIndex === 1) return advancedComponent;
                if (root.currentTabIndex === 2) return donorComponent;
                return simpleComponent;
            }
        }
    }

    Component {
        id: simpleComponent
        NandBuilderSimple {
            onBuildRequested: function(config) {
                console.log("Simple Build requested:", JSON.stringify(config))
            }
        }
    }

    Component {
        id: advancedComponent
        NandBuilderAdvanced {
            onBuildRequested: function(config) {
                console.log("Advanced Build requested:", JSON.stringify(config))
            }
        }
    }

    Component {
        id: donorComponent
        NandBuilderDonor {}
    }
}
