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
        #[qproperty(QString, ini_dir)]
        #[qproperty(QString, common_dir)]
        #[qproperty(QString, data_dir)]
        #[qproperty(QString, ini_ext)]
        #[qproperty(QString, bl_ext)]
        #[qproperty(bool, build_ini_loaded)]
        type GxBuildSession = super::GxBuildSessionStruct;

        #[qinvokable]
        fn run_build(self: Pin<&mut GxBuildSession>) -> QString;

        #[qinvokable]
        fn add_addon(self: Pin<&mut GxBuildSession>, addon: QString);

        #[qinvokable]
        fn clear_addons(self: Pin<&mut GxBuildSession>);

        #[qinvokable]
        fn set_cpukey(self: Pin<&mut GxBuildSession>, key: QString);

        #[qinvokable]
        fn set_option(self: Pin<&mut GxBuildSession>, key: QString, value: QString);

        #[qinvokable]
        fn load_options_ini_file(self: Pin<&mut GxBuildSession>, path: QString) -> bool;

        #[qinvokable]
        fn reset_build(self: Pin<&mut GxBuildSession>);

        #[qinvokable]
        fn prepare_build(self: Pin<&mut GxBuildSession>) -> QString;

        #[qinvokable]
        fn parse_patch(self: Pin<&mut GxBuildSession>, path: QString);

        #[qinvokable]
        fn apply_patch(self: Pin<&mut GxBuildSession>, path: QString, ptype: u8, target: u8);

        #[qinvokable]
        fn apply_smc_signature_batch(self: Pin<&mut GxBuildSession>, json_str: QString) -> i32;

        #[qinvokable]
        fn extract(self: Pin<&mut GxBuildSession>, id: QString, output_dir: QString);

        #[qinvokable]
        fn extract_all(self: Pin<&mut GxBuildSession>, output_dir: QString);

        #[qinvokable]
        fn extract_stfs(self: Pin<&mut GxBuildSession>, path: QString, target_dir: QString);

        #[qinvokable]
        fn replace(self: Pin<&mut GxBuildSession>, id: u8, path: QString);

        #[qinvokable]
        fn delete_asset(self: Pin<&mut GxBuildSession>, id: u8);

        #[qinvokable]
        fn parse_image(self: Pin<&mut GxBuildSession>, path: QString, key: QString);

        #[qinvokable]
        fn parse_flashfs(self: Pin<&mut GxBuildSession>, path: QString);

        #[qinvokable]
        fn swap_bootloader(self: Pin<&mut GxBuildSession>, bl_type: QString, path: QString, is_rebooter: bool);

        #[qinvokable]
        fn extract_options_from_nand(self: Pin<&mut GxBuildSession>);

        #[qinvokable]
        fn sync_options_to_nand(self: Pin<&mut GxBuildSession>) -> QString;

        #[qinvokable]
        fn session_clear(self: Pin<&mut GxBuildSession>);

        #[qinvokable]
        fn compress(self: Pin<&mut GxBuildSession>);
    }
}

pub struct GxBuildSessionStruct {
    build_type: QString,
    console_type: QString,
    output_path: QString,
    status: QString,
    ini_dir: QString,
    common_dir: QString,
    data_dir: QString,
    ini_ext: QString,
    bl_ext: QString,
    build_ini_loaded: bool,

    pub(crate) session: Arc<Mutex<Session>>,
}

impl Default for GxBuildSessionStruct {
    fn default() -> Self {
        Self {
            build_type: QString::from("4RGH"),
            console_type: QString::from("Jasper"),
            output_path: QString::from("updflash.bin"),
            status: QString::from("Idle"),
            ini_dir: QString::default(),
            common_dir: QString::default(),
            data_dir: QString::default(),
            ini_ext: QString::default(),
            bl_ext: QString::default(),
            build_ini_loaded: false,
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

    pub fn add_addon(self: Pin<&mut Self>, addon: QString) {
        if let Ok(mut session) = self.session.lock() {
            session.add_addon(addon.to_string());
        }
    }

    pub fn clear_addons(self: Pin<&mut Self>) {
        if let Ok(mut session) = self.session.lock() {
            session.clear_addons();
        }
    }

    pub fn set_cpukey(self: Pin<&mut Self>, key: QString) {
        if let Ok(mut session) = self.session.lock() {
            session.set_cpukey(key.to_string());
        }
    }

    pub fn set_option(self: Pin<&mut Self>, key: QString, value: QString) {
        if let Ok(mut session) = self.session.lock() {
            session.set_option(&key.to_string(), &value.to_string());
        }
    }

    pub fn load_options_ini_file(self: Pin<&mut Self>, path: QString) -> bool {
        if let Ok(mut session) = self.session.lock() {
            session.load_options_ini_file(path.to_string()).is_ok()
        } else {
            false
        }
    }

    pub fn reset_build(mut self: Pin<&mut Self>) {
        self.as_mut().set_build_ini_loaded(false);
        self.as_mut().set_status(QString::from("Idle"));
        if let Ok(mut session) = self.session.lock() {
            session.reset_build();
        }
    }

    pub fn prepare_build(mut self: Pin<&mut Self>) -> QString {
        self.as_mut().set_status(QString::from("Preparing..."));

        let ini_dir = self.ini_dir().to_string();
        let common_dir = self.common_dir().to_string();
        let data_dir = self.data_dir().to_string();
        let output_path = self.output_path().to_string();
        let ini_ext = self.ini_ext().to_string();
        let bl_ext = self.bl_ext().to_string();
        let build_type = self.build_type().to_string();
        let console = self.console_type().to_string();

        let session_arc = self.session.clone();
        let result = if let Ok(mut session) = session_arc.lock() {
            if !ini_dir.is_empty() {
                session.set_ini_dir(std::path::PathBuf::from(&ini_dir));
            }
            if !common_dir.is_empty() {
                session.set_common_dir(std::path::PathBuf::from(&common_dir));
            }
            if !data_dir.is_empty() {
                session.set_data_dir(std::path::PathBuf::from(&data_dir));
            }
            if !output_path.is_empty() {
                session.set_output(std::path::PathBuf::from(&output_path));
            }
            if !ini_ext.is_empty() {
                session.set_ini_ext(ini_ext);
            }
            if !bl_ext.is_empty() {
                session.set_bl_ext(bl_ext);
            }
            if !build_type.is_empty() {
                session.set_build_type(build_type);
            }
            if !console.is_empty() {
                session.set_console(console);
            }

            match session.prepare_build() {
                Ok(_) => {
                    self.as_mut().set_status(QString::from("Ready to build"));
                    QString::default()
                }
                Err(e) => {
                    let err_qs = QString::from(&e);
                    self.as_mut().set_status(QString::from(format!("Error: {}", e)));
                    err_qs
                }
            }
        } else {
            QString::from("Failed to acquire session lock")
        };
        result
    }

    pub fn parse_patch(self: Pin<&mut Self>, path: QString) {
        if let Ok(mut session) = self.session.lock() {
            session.parse_patch(std::path::PathBuf::from(path.to_string()));
        }
    }

    pub fn apply_patch(self: Pin<&mut Self>, path: QString, ptype: u8, target: u8) {
        if let Ok(mut session) = self.session.lock() {
            let target_opt = if target == 0 { None } else { Some(target) };
            session.apply_patch(std::path::PathBuf::from(path.to_string()), ptype, target_opt);
        }
    }

    pub fn apply_smc_signature_batch(self: Pin<&mut Self>, json_str: QString) -> i32 {
        if let Ok(mut session) = self.session.lock() {
            match session.apply_smc_signature_batch(&json_str.to_string()) {
                Ok(count) => count as i32,
                Err(_) => -1,
            }
        } else {
            -1
        }
    }

    pub fn extract(self: Pin<&mut Self>, id: QString, output_dir: QString) {
        if let Ok(mut session) = self.session.lock() {
            session.extract(id.to_string(), std::path::PathBuf::from(output_dir.to_string()));
        }
    }

    pub fn extract_all(self: Pin<&mut Self>, output_dir: QString) {
        if let Ok(mut session) = self.session.lock() {
            session.extract_all(std::path::PathBuf::from(output_dir.to_string()));
        }
    }

    pub fn extract_stfs(self: Pin<&mut Self>, path: QString, target_dir: QString) {
        if let Ok(mut session) = self.session.lock() {
            session.extract_stfs(std::path::PathBuf::from(path.to_string()), std::path::PathBuf::from(target_dir.to_string()));
        }
    }

    pub fn replace(self: Pin<&mut Self>, id: u8, path: QString) {
        if let Ok(mut session) = self.session.lock() {
            session.replace(id, std::path::PathBuf::from(path.to_string()));
        }
    }

    pub fn delete_asset(self: Pin<&mut Self>, id: u8) {
        if let Ok(mut session) = self.session.lock() {
            session.delete(id);
        }
    }

    pub fn parse_image(self: Pin<&mut Self>, path: QString, key: QString) {
        if let Ok(mut session) = self.session.lock() {
            let key_opt = if key.is_empty() {
                None
            } else if let Ok(bytes) = libgxbuild::builder::builder::hex_to_bytes(&key.to_string()) {
                bytes.try_into().ok()
            } else {
                None
            };
            session.parse_image(std::path::PathBuf::from(path.to_string()), key_opt);
        }
    }

    pub fn parse_flashfs(self: Pin<&mut Self>, path: QString) {
        if let Ok(mut session) = self.session.lock() {
            session.parse_flashfs(std::path::PathBuf::from(path.to_string()));
        }
    }

    pub fn swap_bootloader(self: Pin<&mut Self>, bl_type: QString, path: QString, is_rebooter: bool) {
        if let Ok(mut session) = self.session.lock() {
            session.swap_bootloader(bl_type.to_string(), std::path::PathBuf::from(path.to_string()), is_rebooter);
        }
    }

    pub fn extract_options_from_nand(self: Pin<&mut Self>) {
        if let Ok(mut session) = self.session.lock() {
            session.extract_options_from_nand();
        }
    }

    pub fn sync_options_to_nand(self: Pin<&mut Self>) -> QString {
        if let Ok(mut session) = self.session.lock() {
            match session.sync_options_to_nand() {
                Ok(_) => QString::default(),
                Err(e) => QString::from(&e),
            }
        } else {
            QString::from("Failed to acquire session lock")
        }
    }

    pub fn session_clear(self: Pin<&mut Self>) {
        if let Ok(mut session) = self.session.lock() {
            session.session_clear();
        }
    }

    pub fn compress(self: Pin<&mut Self>) {
        if let Ok(mut session) = self.session.lock() {
            session.compress();
        }
    }
}
