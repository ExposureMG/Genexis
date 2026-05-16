import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

Kirigami.Page {
    title: "Home"

    ColumnLayout {
        anchors.centerIn: parent
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Heading {
            text: "Welcome to Genexis"
            Layout.alignment: Qt.AlignHCenter
        }

        Controls.Label {
            text: "Xbox 360 JTAG/RGH toolkit"
            Layout.alignment: Qt.AlignHCenter
        }
        
        Kirigami.Icon {
            source: "org.gxgx.genexis"
            Layout.preferredWidth: Kirigami.Units.gridUnit * 5
            Layout.preferredHeight: Kirigami.Units.gridUnit * 5
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
