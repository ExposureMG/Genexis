import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as Controls
import org.kde.kirigami as Kirigami

ColumnLayout {
    id: blockmapPanel
    spacing: Kirigami.Units.largeSpacing

    RowLayout {
        Layout.fillWidth: true
        Controls.Button {
            text: qsTr("Save Blockmap")
            icon.name: "document-import"
            Layout.fillWidth: true
        }
        Controls.Button {
            text: qsTr("Load Blockmap")
            icon.name: "document-export"
            Layout.fillWidth: true
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Heading {
            text: qsTr("Bad Blocks")
            level: 4
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: Kirigami.Units.gridUnit * 5
            color: "#0a0a0a"
            radius: Kirigami.Units.smallSpacing / 2
            border.color: "#333"

            Controls.ScrollView {
                anchors.fill: parent
                anchors.margins: Kirigami.Units.smallSpacing
                Controls.ScrollBar.vertical.policy: Controls.ScrollBar.AsNeeded

                Controls.TextArea {
                    id: badblockConsole
                    readOnly: true
                    text: "> Waiting for bad blocks..."
                    font.family: "Monospace"
                    font.pointSize: 10
                    color: "#00ff00" // Classic terminal green
                    wrapMode: Controls.TextArea.Wrap
                    background: null
                    selectByMouse: true
                }
            }
        }
    }
}
