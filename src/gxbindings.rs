use cxx_qt_lib::QString;
use libgxbuild::core::session::Session;
use std::pin::Pin;
use std::sync::{Arc, Mutex};

#[cxx_qt::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
    }

    #[auto_cxx_name]
    unsafe extern "RustQt" {
        #[qobject]
        #[qml_element]
        #[qproperty(QString, build_type)]
        #[qproperty(QString, console_type)]
        #[qproperty(QString, output_path)]
        #[qproperty(QString, status)]
        type GxBuildSession = super::GxBuildSessionStruct;

        #[qinvokable]
        fn run_build(self: Pin<&mut GxBuildSession>) -> QString;

        #[qinvokable]
        fn set_ini_dir(self: Pin<&mut GxBuildSession>, path: QString);

        #[qinvokable]
        fn set_data_dir(self: Pin<&mut GxBuildSession>, path: QString);
    }
}

pub struct GxBuildSessionStruct {
    build_type: QString,
    console_type: QString,
    output_path: QString,
    status: QString,

    pub(crate) session: Arc<Mutex<Session>>,
}

impl Default for GxBuildSessionStruct {
    fn default() -> Self {
        Self {
            build_type: QString::from("4RGH"),
            console_type: QString::from("Jasper"),
            output_path: QString::from("updflash.bin"),
            status: QString::from("Idle"),
            session: Arc::new(Mutex::new(Session::new())),
        }
    }
}

impl ffi::GxBuildSession {
    pub fn run_build(mut self: Pin<&mut Self>) -> QString {
        self.as_mut().set_status(QString::from("Running..."));

        let build_type = self.build_type().to_string();
        let console = self.console_type().to_string();
        let output = self.output_path().to_string();

        let session_arc = self.session.clone();
        if let Ok(mut session) = session_arc.lock() {
            session.set_build_type(build_type);
            session.set_console(console);
            session.set_output(std::path::PathBuf::from(output));

            session.session_run();

            if let Some(err) = &session.last_error {
                let err_qs = QString::from(err);
                self.as_mut()
                    .set_status(QString::from(format!("Error: {}", err)));
                return err_qs;
            }
        }

        self.as_mut().set_status(QString::from("Finished"));
        QString::default()
    }

    pub fn set_ini_dir(self: Pin<&mut Self>, path: QString) {
        if let Ok(mut session) = self.session.lock() {
            session.set_ini_dir(std::path::PathBuf::from(path.to_string()));
        }
    }

    pub fn set_data_dir(self: Pin<&mut Self>, path: QString) {
        if let Ok(mut session) = self.session.lock() {
            session.set_data_dir(std::path::PathBuf::from(path.to_string()));
        }
    }
}
