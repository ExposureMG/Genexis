import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import org.gxgx.genexis

Kirigami.FormLayout {
    id: nandInfoLayout

    Kirigami.Separator {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: "Bootloaders"
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: Kirigami.Units.gridUnit * 2
        Layout.alignment: Qt.AlignHCenter

        // CB Column
        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Heading {
                text: "CB"
                level: 4
                Layout.alignment: Qt.AlignHCenter
            }

            Kirigami.FormLayout {
                Layout.fillWidth: true
                wideMode: true
                
                Controls.TextField {
                    Kirigami.FormData.label: "CB:"
                    id: cbField
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                }
                Controls.TextField {
                    Kirigami.FormData.label: "CB_B:"
                    id: cbbField
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                }
                Controls.TextField {
                    Kirigami.FormData.label: "CB_X:"
                    id: cbxField
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                    visible: false
                }
            }
        }

        // Vertical Separator
        Kirigami.Separator {
            Layout.fillHeight: true
            width: 1
        }

        // Kernel Column
        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Heading {
                text: "Kernel"
                level: 4
                Layout.alignment: Qt.AlignHCenter
            }

            Kirigami.FormLayout {
                Layout.fillWidth: true
                wideMode: true
                
                Controls.TextField {
                    Kirigami.FormData.label: "SC:"
                    id: scField
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                    visible: false
                }
                Controls.TextField {
                    Kirigami.FormData.label: "CD:"
                    id: cdField
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                }
                Controls.TextField {
                    Kirigami.FormData.label: "CE:"
                    id: ceField
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                }
            }
        }
    }

    Kirigami.Separator {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: "Patchslots"
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter
        spacing: Kirigami.Units.gridUnit * 2

        // Slot 0 Column
        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Heading {
                text: "Slot 0"
                level: 4
                Layout.alignment: Qt.AlignHCenter
            }

            Kirigami.FormLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                wideMode: true
                
                Controls.TextField {
                    Kirigami.FormData.label: "CF:"
                    id: cf0Field
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                }
                Controls.TextField {
                    Kirigami.FormData.label: "CG:"
                    id: cg0Field
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                }
                Controls.TextField {
                    Kirigami.FormData.label: "PD:"
                    id: pd0Field
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                }
                Controls.TextField {
                    Kirigami.FormData.label: "LDV:"
                    id: ldv0Field
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                }
            }
        }

        // Vertical Separator
        Kirigami.Separator {
            Layout.fillHeight: true
            width: 1
        }

        // Slot 1 Column
        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Heading {
                text: "Slot 1"
                level: 4
                Layout.alignment: Qt.AlignHCenter
            }

            Kirigami.FormLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                wideMode: true
                
                Controls.TextField {
                    Kirigami.FormData.label: "CF:"
                    id: cf1Field
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                }
                Controls.TextField {
                    Kirigami.FormData.label: "CG:"
                    id: cg1Field
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                }
                Controls.TextField {
                    Kirigami.FormData.label: "PD:"
                    id: pd1Field
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                }
                Controls.TextField {
                    Kirigami.FormData.label: "LDV:"
                    id: ldv1Field
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                }
            }
        }
    }

    Kirigami.Separator {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: "SMC & Flash"
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter
        spacing: Kirigami.Units.gridUnit * 2

        // SMC Column
        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Heading {
                text: "SMC"
                level: 4
                Layout.alignment: Qt.AlignHCenter
            }

            Kirigami.FormLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                wideMode: true
                
                Controls.TextField {
                    Kirigami.FormData.label: "Ver:"
                    id: smcVerField
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                }
                Controls.TextField {
                    Kirigami.FormData.label: "LDV:"
                    id: smcLdvField
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                }
                Controls.TextField {
                    Kirigami.FormData.label: "PD:"
                    id: smcPdField
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                }
            }
        }

        // Vertical Separator
        Kirigami.Separator {
            Layout.fillHeight: true
            width: 1
        }

        // Flash Column
        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Heading {
                text: "Flash"
                level: 4
                Layout.alignment: Qt.AlignHCenter
            }

            Kirigami.FormLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                wideMode: true
                
                Controls.TextField {
                    Kirigami.FormData.label: "Type:"
                    id: flashTypeField
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                }
                Controls.TextField {
                    Kirigami.FormData.label: "Console:"
                    id: flashConsoleField
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                }
                Controls.TextField {
                    Kirigami.FormData.label: "Block:"
                    id: flashBlockField
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                }
            }
        }
    }
}