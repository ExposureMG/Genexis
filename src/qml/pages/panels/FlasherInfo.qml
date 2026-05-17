import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as Controls
import org.kde.kirigami as Kirigami

ColumnLayout {
    id: flasherInfoLayout
    spacing: Kirigami.Units.largeSpacing

    // Top: Image Box
    Rectangle {
        id: imageBox
        Layout.fillWidth: true
        Layout.preferredHeight: Kirigami.Units.gridUnit * 10
        color: Kirigami.Theme.alternateBackgroundColor
        border.color: Kirigami.Theme.focusColor
        border.width: 1
        radius: Kirigami.Units.smallSpacing / 2

        Kirigami.Icon {
            anchors.centerIn: parent
            width: parent.height * 0.8
            height: width
            source: "hw-info"
            opacity: 0.5
        }
        
        Controls.Label {
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottomMargin: Kirigami.Units.smallSpacing
            text: qsTr("Flasher Preview")
            font.pointSize: Kirigami.Theme.smallFont.pointSize
            opacity: 0.7
        }
    }

    // Middle section: Status on left half, Actions on right half
    RowLayout {
        Layout.fillWidth: true
        spacing: Kirigami.Units.smallSpacing
        
        // Left Column: Spans half width
        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            spacing: Kirigami.Units.smallSpacing
            
            Controls.TextField {
                readOnly: true
                text: "Disconnected"
                horizontalAlignment: Text.AlignHCenter
                palette.base: Kirigami.Theme.alternateBackgroundColor
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit * 2
            }

            Controls.TextField {
                readOnly: true
                text: "Idle"
                horizontalAlignment: Text.AlignHCenter
                palette.base: Kirigami.Theme.alternateBackgroundColor
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit * 2
            }
        }
        
        // Right Column: Spans half width
        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            spacing: Kirigami.Units.smallSpacing
            
            Controls.Button {
                text: qsTr("Read")
                icon.name: "document-import"
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit * 2
            }
            Controls.Button {
                text: qsTr("Write")
                icon.name: "document-export"
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit * 2
            }
        }
    }

    // Uneditable status/info field
    Controls.TextField {
        Layout.fillWidth: true
        readOnly: true
        placeholderText: qsTr("Source File")
        horizontalAlignment: Text.AlignHCenter
        palette.base: Kirigami.Theme.alternateBackgroundColor
    }

    // Bottom: Big Black Console
    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Heading {
            text: qsTr("Console")
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
                    id: flasherConsole
                    readOnly: true
                    text: "> Initializing flasher interface...\n> Ready."
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
