import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

QQC2.ComboBox {
    id: root

    signal selected(string value)

    Layout.fillWidth: true
    onCurrentTextChanged: {
        if (currentText !== "") {
            root.selected(currentText);
        }
    }
}
