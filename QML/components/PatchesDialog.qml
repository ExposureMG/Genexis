import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

QQC2.Dialog {
    id: root

    property var availablePatches: []
    property var activePatches: []
    property var tempStates: ({})

    signal patchesSaved(var patches)

    title: qsTr("Configure Patches")
    modal: true
    parent: QQC2.Overlay.overlay
    anchors.centerIn: parent
    implicitWidth: Kirigami.Units.gridUnit * 18
    implicitHeight: Kirigami.Units.gridUnit * 16

    onAboutToShow: {
        var states = {};
        for (var i = 0; i < root.availablePatches.length; i++) {
            var patchName = root.availablePatches[i];
            states[patchName] = root.activePatches.indexOf(patchName) !== -1;
        }
        tempStates = states;
    }

    contentItem: QQC2.ScrollView {
        clip: true
        ListView {
            id: patchesListView
            model: root.availablePatches
            delegate: QQC2.CheckDelegate {
                required property string modelData

                width: patchesListView.width
                text: modelData
                checked: root.tempStates[modelData] || false
                onCheckedChanged: {
                    var updated = Object.assign({}, root.tempStates);
                    updated[modelData] = checked;
                    root.tempStates = updated;
                }
            }
        }
    }

    footer: QQC2.DialogButtonBox {
        alignment: Qt.AlignRight

        QQC2.Button {
            text: qsTr("Close")
            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.RejectRole
            onClicked: root.reject()
        }

        QQC2.Button {
            text: qsTr("Save")
            highlighted: true
            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
            onClicked: {
                var selected = [];
                for (var i = 0; i < root.availablePatches.length; i++) {
                    var patchName = root.availablePatches[i];
                    if (root.tempStates[patchName]) {
                        selected.push(patchName);
                    }
                }
                root.patchesSaved(selected);
                root.accept();
            }
        }
    }
}
