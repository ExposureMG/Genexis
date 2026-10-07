import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.AbstractCard {
    id: root

    property string heading
    property string summary

    signal detailsRequested

    contentItem: Item {
        implicitWidth: cardLayout.implicitWidth
        implicitHeight: cardLayout.implicitHeight

        ColumnLayout {
            id: cardLayout
            anchors {
                left: parent.left
                top: parent.top
                right: parent.right
            }
            spacing: Kirigami.Units.mediumSpacing

            Kirigami.Heading {
                level: 2
                text: root.heading
            }
            Kirigami.Separator {
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.fillWidth: true
                QQC2.Label {
                    text: root.summary
                    font.bold: true
                    Layout.fillWidth: true
                }
                QQC2.Button {
                    text: qsTr("Details…")
                    icon.name: "dialog-information"
                    onClicked: root.detailsRequested()
                }
            }
        }
    }
}
