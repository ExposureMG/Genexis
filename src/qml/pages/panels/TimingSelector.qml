import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as Controls
import org.kde.kirigami as Kirigami

ColumnLayout {
    id: timingSelector
    spacing: Kirigami.Units.largeSpacing

    Kirigami.Heading {
        text: qsTr("Placeholder for timings")
        level: 2
    }

    Kirigami.FormLayout {
        Layout.fillWidth: true
        wideMode: true

        Controls.ComboBox {
            Kirigami.FormData.label: qsTr("Type:")
            model: ["RGH1", "RGH1.2", "RJTOP", "SRGH", "EXT_CLK"]
        }
        
        Controls.ComboBox {
            Kirigami.FormData.label: qsTr("Timing:")
            model: ["1", "2", "3"]
        }
    }

    Controls.Button {
        text: qsTr("Load Into Flasher")
        icon.name: "system-run"
        Layout.fillWidth: true
    }

}
