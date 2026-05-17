import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as Controls
import org.kde.kirigami as Kirigami
import org.gxgx.genexis
import "panels" as Panels

Kirigami.Page {
    title: "Timing"

    RowLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        // Left Column: Timing Selector Panel
        Kirigami.Card {
            Layout.preferredWidth: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignTop
            
            header: Kirigami.Heading {
                text: qsTr("Timing Configuration")
                level: 3
            }
            
            contentItem: Panels.TimingSelector {
                id: timingSelector
            }
        }

        // Right Column: Flasher Information Panel
        Kirigami.Card {
            Layout.preferredWidth: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignTop
            
            header: Kirigami.Heading {
                text: qsTr("Flasher Information")
                level: 3
            }
            
            contentItem: Panels.FlasherInfo {
                id: flasherInfo
            }
        }
    }
}
