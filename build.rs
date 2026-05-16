use cxx_qt_build::{CxxQtBuilder, QmlModule};

fn main() {
    CxxQtBuilder::new_qml_module(
        QmlModule::new("org.gxgx.genexis")
            .qml_file("src/qml/Main.qml")
            .qml_file("src/qml/pages/Home.qml")
            .qml_file("src/qml/pages/GxBuild.qml")
            .qml_file("src/qml/pages/panels/NandInfo.qml"),
    )
    .files(["src/gxbindings.rs"])
    .build();
}
