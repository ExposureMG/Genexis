import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import org.gxgx.genexis

Kirigami.ApplicationWindow {
    id: root

    title: "Genexis"

    minimumWidth: Kirigami.Units.gridUnit * 30
    minimumHeight: Kirigami.Units.gridUnit * 50
    width: minimumWidth
    height: minimumHeight

    pageStack.initialPage: initPage

    Component {
        id: initPage

        Kirigami.Page {
            title: "gxBuild"

            GxBuild {
                id: gxbuild
            }


            ColumnLayout {
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                }
                
                RowLayout {
                    Controls.Label { text: "Console:" }
                    Controls.ComboBox {
                        model: ["Jasper", "Falcon", "Trinity", "Corona"]
                        onCurrentTextChanged: gxbuild.console_type = currentText
                    }
                }

                RowLayout {
                    Controls.Label { text: "Build Type:" }
                    Controls.ComboBox {
                        model: ["4RGH", "5JTAG", "4JTAG", "3RGH"]
                        onCurrentTextChanged: gxbuild.build_type = currentText
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    Controls.Button {
                        text: "Run Build"
                        onClicked: gxbuild.run_build()
                    }
                }

                Controls.Label {
                    id: statusLabel
                    text: "Status: " + gxbuild.status
                    Layout.fillWidth: true
                }
            }
        }
    }
}
