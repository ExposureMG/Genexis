import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.Dialog {
    id: root

    title: qsTr("Console Output")
    preferredWidth: Kirigami.Units.gridUnit * 30
    preferredHeight: Kirigami.Units.gridUnit * 20
    padding: Kirigami.Units.largeSpacing

    property alias logText: consoleText.text
    property bool showProgress: false
    property int progressValue: 0
    property string statusText: ""

    function appendLog(text) {
        consoleText.text += text + "\n";
    }

    function clearLog() {
        consoleText.text = "";
    }

    ColumnLayout {
        spacing: Kirigami.Units.mediumSpacing
        width: parent ? parent.width : undefined

        // Optional progress bar section
        ColumnLayout {
            Layout.fillWidth: true
            visible: root.showProgress
            spacing: Kirigami.Units.smallSpacing

            QQC2.Label {
                text: root.statusText !== "" ? root.statusText : qsTr("Processing… (%1%)").arg(root.progressValue)
                font.bold: true
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            QQC2.ProgressBar {
                id: progressBar
                from: 0
                to: 100
                value: root.progressValue
                Layout.fillWidth: true
            }
        }

        // Black output console frame (styled identically to Flasher.qml)
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            implicitHeight: Kirigami.Units.gridUnit * 12
            color: "#1e1e1e"
            radius: Kirigami.Units.smallSpacing
            border.color: Qt.rgba(1, 1, 1, 0.1)

            QQC2.ScrollView {
                anchors.fill: parent
                anchors.margins: Kirigami.Units.smallSpacing
                clip: true

                QQC2.TextArea {
                    id: consoleText
                    readOnly: true
                    font.family: "Monospace"
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                    color: "#00ff66"
                    background: null
                    selectByMouse: true
                    wrapMode: Text.Wrap
                    onTextChanged: {
                        cursorPosition = text.length;
                    }
                }
            }
        }
    }

    footer: QQC2.DialogButtonBox {
        alignment: Qt.AlignRight

        QQC2.Button {
            text: qsTr("Close")
            icon.name: "dialog-close"
            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.RejectRole
            onClicked: root.close()
        }
    }
}
