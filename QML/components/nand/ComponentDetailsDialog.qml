import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

QQC2.Dialog {
    id: root

    default property alias fields: form.data

    modal: true
    parent: QQC2.Overlay.overlay
    anchors.centerIn: parent
    padding: Kirigami.Units.largeSpacing
    implicitWidth: Kirigami.Units.gridUnit * 24

    contentItem: QQC2.ScrollView {
        clip: true
        implicitHeight: form.implicitHeight

        Kirigami.FormLayout {
            id: form
            wideMode: true
            width: parent.width
        }
    }

    footer: QQC2.DialogButtonBox {
        alignment: Qt.AlignRight
        QQC2.Button {
            text: qsTr("Close")
            onClicked: root.close()
        }
    }
}
