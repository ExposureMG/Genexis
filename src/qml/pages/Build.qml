import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as Controls
import org.kde.kirigami as Kirigami
import org.gxgx.genexis
import "panels" as Panels

Kirigami.Page {
    title: "Build"

    GxBuildSession {
        id: gxbuild
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        // Left Column: Split into NandSelector and GxBuildPanel
        ColumnLayout {
            Layout.preferredWidth: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Kirigami.Units.largeSpacing

            Kirigami.Card {
                // Top section takes more height
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredHeight: 0.3
                
                header: Kirigami.Heading {
                    text: qsTr("NAND Selection")
                    level: 3
                }
                
                contentItem: Panels.NandSelector {
                    id: nandSelector
                }
            }

            Kirigami.Card {
                // Bottom section takes less height to line up with consoles
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredHeight: 1
                
                header: Kirigami.Heading {
                    text: qsTr("Build Configuration")
                    level: 3
                }
                
                contentItem: Panels.GxBuildPanel {
                    gxbuild: gxbuild
                }
            }
        }

        // Right Column: NAND Info Panel in a Card
        Kirigami.Card {
            Layout.preferredWidth: 0
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
