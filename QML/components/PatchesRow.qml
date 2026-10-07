import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

RowLayout {
    id: root

    property var activePatches: []
    property bool patchesEnabled: true
    property bool showOptions: false

    signal patchesClicked
    signal optionsClicked

    Layout.fillWidth: true
    spacing: Kirigami.Units.smallSpacing

    QQC2.Button {
        enabled: root.patchesEnabled
        text: root.activePatches.length > 0 ? qsTr("Patches (%1 selected)").arg(root.activePatches.length) : qsTr("Patches")
        icon.name: "preferences-system-patches"
        onClicked: root.patchesClicked()
    }

    QQC2.Button {
        visible: root.showOptions
        text: qsTr("Options")
        icon.name: "configure"
        onClicked: root.optionsClicked()
    }

    QQC2.Label {
        visible: root.activePatches.length > 0
        text: root.activePatches.join(", ")
        color: Kirigami.Theme.disabledTextColor
        elide: Text.ElideRight
        Layout.fillWidth: true
    }
}
