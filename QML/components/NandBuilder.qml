import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Item {
    id: root

    property bool isAdvanced: false

    ColumnLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.smallSpacing

        // Top toggle row with QQC2.Switch
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.topMargin: Kirigami.Units.smallSpacing
            spacing: Kirigami.Units.smallSpacing

            QQC2.Label {
                text: qsTr("Advanced Options")
                font.bold: true
                Layout.alignment: Qt.AlignVCenter
            }

            QQC2.Switch {
                id: advancedSwitch
                checked: root.isAdvanced
                onCheckedChanged: root.isAdvanced = checked
            }

            Item { Layout.fillWidth: true }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        // Dynamic view container switching between Simple and Advanced components
        Loader {
            id: builderLoader
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: root.isAdvanced ? advancedComponent : simpleComponent
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
}
