import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.Dialog {
    id: root

    title: qsTr("Configure Patches")
    preferredWidth: Kirigami.Units.gridUnit * 18
    preferredHeight: Kirigami.Units.gridUnit * 16
    standardButtons: Kirigami.Dialog.Save | Kirigami.Dialog.Cancel

    property var availablePatches: []
    property var selectedPatches: []
    property var _tempStates: ({})

    signal patchesAccepted(var patches)

    onAboutToShow: {
        var states = {};
        var list = root.availablePatches || [];
        for (var i = 0; i < list.length; i++) {
            var patchName = list[i];
            states[patchName] = root.selectedPatches.indexOf(patchName) !== -1;
        }
        _tempStates = states;
    }

    onAccepted: {
        var selected = [];
        var list = root.availablePatches || [];
        for (var i = 0; i < list.length; i++) {
            var patchName = list[i];
            if (root._tempStates[patchName]) {
                selected.push(patchName);
            }
        }
        root.patchesAccepted(selected);
    }

    ListView {
        id: patchesListView
        implicitHeight: contentHeight
        model: root.availablePatches
        delegate: QQC2.CheckDelegate {
            required property string modelData

            width: patchesListView.width
            text: modelData
            checked: root._tempStates[modelData] || false
            onToggled: {
                var updated = Object.assign({}, root._tempStates);
                updated[modelData] = checked;
                root._tempStates = updated;
            }
        }
    }
}
