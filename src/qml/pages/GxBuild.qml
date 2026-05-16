import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as Controls
import org.kde.kirigami as Kirigami
import org.gxgx.genexis
import "panels" as Panels

Kirigami.Page {
    title: "gxBuild"

    GxBuildSession {
        id: gxbuild
    }


    Kirigami.Dialog {
        id: loadImageDialog
        title: qsTr("Load Image")
        padding: Kirigami.Units.largeSpacing
        preferredWidth: Kirigami.Units.gridUnit * 20
        standardButtons: Kirigami.Dialog.NoButton
        customFooterActions: [
            Kirigami.Action {
                text: "Confirm"
                icon.name: "dialog-ok"
                onTriggered: {
                    root.showPassiveNotification(qsTr("Loading Image..."));
                    loadImageDialog.close();
                }
            },
            Kirigami.Action {
                text: "Browse"
                icon.name: "document-open"
                onTriggered: {
                    root.showPassiveNotification(qsTr("Browsing for files..."));
                }
            },
            Kirigami.Action {
                text: "Cancel"
                icon.name: "dialog-cancel"
                onTriggered: {
                    loadImageDialog.close();
                }
            }
        ]

        ColumnLayout {
            Controls.TextField {
                id: imagePathField
                Kirigami.FormData.label: qsTr("Image Path*:")
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        // Left Column: Build Configuration
        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: Kirigami.Units.largeSpacing

            // --- Source Loading Section ---
            RowLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                Controls.TextField {
                    id: sourcePathField
                    placeholderText: qsTr("Source NAND Image Path...")
                    Layout.fillWidth: true
                }

                Controls.Button {
                    text: qsTr("Load Source")
                    icon.name: "document-open"
                    onClicked: {
                        root.showPassiveNotification(qsTr("Loading source: %1").arg(sourcePathField.text))
                    }
                }
            }

            // --- Build Settings Section ---
            Kirigami.Heading {
                text: qsTr("Build Settings")
                level: 2
            }

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
                        onCurrentTextChanged: gxbuild.consoleType = currentText
                    }
                }

                Kirigami.FormLayout {
                    Layout.fillWidth: true
                    wideMode: true
                    Controls.ComboBox {
                        Kirigami.FormData.label: qsTr("Type:")
                        model: ["4RGH", "5JTAG", "4JTAG", "3RGH"]
                        onCurrentTextChanged: gxbuild.buildType = currentText
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
                        onTextChanged: gxbuild.output_path = text
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
                onClicked: gxbuild.runBuild()
            }

            Kirigami.InlineMessage {
                Layout.fillWidth: true
                text: gxbuild.status
                visible: gxbuild.status !== "Idle"
                type: gxbuild.status.indexOf("Error") !== -1 ? Kirigami.MessageType.Error : Kirigami.MessageType.Information
            }
        }

        // Right Column: NAND Info Panel
        Kirigami.Card {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignTop
            
            header: Kirigami.Heading {
                text: qsTr("NAND Information")
                level: 3
            }
            
            contentItem: Panels.NandInfo {
                id: nandInfo
            }
        }
    }
}