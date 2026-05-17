use cxx_qt_build::{CxxQtBuilder, QmlModule};

fn main() {
    CxxQtBuilder::new_qml_module(
        QmlModule::new("org.gxgx.genexis")
            .qml_file("src/qml/Main.qml")
            .qml_file("src/qml/pages/Home.qml")
            .qml_file("src/qml/pages/Build.qml")
            .qml_file("src/qml/pages/NandFlash.qml")
            .qml_file("src/qml/pages/Settings.qml")
            .qml_file("src/qml/pages/TimingFlash.qml")
            .qml_file("src/qml/pages/panels/NandInfo.qml")
            .qml_file("src/qml/pages/panels/FlasherInfo.qml")
            .qml_file("src/qml/pages/panels/GxBuildPanel.qml")
            .qml_file("src/qml/pages/panels/Blockmap.qml")
            .qml_file("src/qml/pages/panels/NandSelector.qml")
            .qml_file("src/qml/pages/panels/TimingSelector.qml"),
    )
    .files(["src/gxbindings.rs"])
    .build();
}
