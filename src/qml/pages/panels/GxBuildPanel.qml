import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as Controls
import org.kde.kirigami as Kirigami
import org.gxgx.genexis

ColumnLayout {
    id: buildPanel
    spacing: Kirigami.Units.largeSpacing
    
    // The session should probably be passed in or managed globally, 
    // but for now we'll define it here if it's the primary owner.
    // However, if Build.qml owns it, we should use a property.
    property var gxbuild
    
    RowLayout {
        Layout.fillWidth: true
        spacing: Kirigami.Units.largeSpacing

        Kirigami.FormLayout {
            Layout.fillWidth: true
            wideMode: true
            Controls.ComboBox {
                Kirigami.FormData.label: qsTr("Kernel:")
                model: ["17559", "17526", "17489"]
            }
            Controls.ComboBox {
                Kirigami.FormData.label: qsTr("Platform:")
                model: ["Jasper", "Falcon", "Trinity", "Corona"]
                onCurrentTextChanged: if(gxbuild) gxbuild.consoleType = currentText
            }
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true
            wideMode: true
            Controls.ComboBox {
                Kirigami.FormData.label: qsTr("Type:")
                model: ["4RGH", "5JTAG", "4JTAG", "3RGH"]
                onCurrentTextChanged: if(gxbuild) gxbuild.buildType = currentText
            }
            Controls.ComboBox {
                Kirigami.FormData.label: qsTr("Ext:")
                model: ["None", "FCRT", "FSD"]
            }
        }
    }

    // --- Overrides Section ---
    Kirigami.Heading {
        text: qsTr("Overrides")
        level: 2
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: Kirigami.Units.largeSpacing

        Kirigami.FormLayout {
            Layout.fillWidth: true
            wideMode: true
            Controls.ComboBox {
                Kirigami.FormData.label: qsTr("CB:")
                model: ["Default", "CB_A", "CB_B"]
            }
            Controls.ComboBox {
                Kirigami.FormData.label: qsTr("Block:")
                model: ["Default", "64KB", "128KB"]
            }
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true
            wideMode: true
            Controls.ComboBox {
                Kirigami.FormData.label: qsTr("SMC:")
                model: ["Default", "Custom"]
            }
            Controls.ComboBox {
                Kirigami.FormData.label: qsTr("Size:")
                model: ["Default", "16MB", "256MB", "512MB"]
            }
        }
    }

    // --- Output & Actions ---
    Kirigami.FormLayout {
        Layout.fillWidth: true
        
        RowLayout {
            Kirigami.FormData.label: qsTr("Output Path:")
            Layout.fillWidth: true
            
            Controls.TextField {
                id: outputPathField
                placeholderText: "updflash.bin"
                Layout.fillWidth: true
                onTextChanged: if(gxbuild) gxbuild.output_path = text
            }
            
            Controls.Button {
                text: qsTr("Browse")
                icon.name: "document-open"
                onClicked: {
                    root.showPassiveNotification(qsTr("Browsing for output path..."))
                }
            }
        }
    }

    Controls.Button {
        text: qsTr("Run Build")
        icon.name: "system-run"
        Layout.fillWidth: true
        onClicked: if(gxbuild) gxbuild.runBuild()
    }

    Kirigami.InlineMessage {
        Layout.fillWidth: true
        text: gxbuild ? gxbuild.status : "Idle"
        visible: gxbuild && gxbuild.status !== "Idle"
        type: (gxbuild && gxbuild.status.indexOf("Error") !== -1) ? Kirigami.MessageType.Error : Kirigami.MessageType.Information
    }
}
