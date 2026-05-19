//! FTDI2SPI FFI bindings for xFlasher.dll
//!
//! These bindings provide low-level access to the xFlasher SPI/eMMC
//! flash programming interface via FTDI devices.

use std::ffi::{c_char, c_int, CStr};
use std::path::Path;

/// Error codes returned by xFlasher operations
pub mod error {
    /// Operation completed successfully
    pub const SUCCESS: i32 = 0;
    /// Operation was cancelled by user
    pub const CANCELLED: i32 = -1;
    /// Device not initialized
    pub const DEVICE_NOT_INITIALIZED: i32 = -2;
    /// Console not found
    pub const CONSOLE_NOT_FOUND: i32 = -3;
    /// Unknown NAND type
    pub const UNKNOWN_NAND: i32 = -4;
    /// Could not open file
    pub const FILE_OPEN_ERROR: i32 = -11;
}

/// SPI operation modes
pub mod spi_mode {
    /// Read flash config
    pub const GET_CONFIG: i32 = 0;
    /// Read NAND
    pub const READ: i32 = 1;
    /// Write NAND (standard)
    pub const WRITE: i32 = 3;
    /// Write XeLL (raw ECC)
    pub const WRITE_XELL: i32 = 4;
    /// Erase NAND
    pub const ERASE: i32 = 5;
}

/// eMMC block constants
pub const EMMC_BLOCK_COUNT_FULL: i32 = 98304;
pub const EMMC_BLOCK_COUNT_XELL: i32 = 2560;

/// Common flash config values
pub mod flash_config {
    pub const JASPER_TRINITY_16MB: &str = "00023010";
    pub const CORONA_16MB: &str = "00043000";
    pub const JASPER_TRINITY_256MB: &str = "008A3020";
    pub const JASPER_TRINITY_512MB: &str = "00AA3020";
    pub const CORONA_256MB: &str = "008C3020";
    pub const CORONA_512MB: &str = "00AC3020";
    pub const XENON_ZEPHYR_FALCON_16MB: &str = "01198010";
    pub const XENON_ZEPHYR_FALCON_64MB: &str = "01198030";
    pub const CORONA_4GB_EMMC: &str = "C0462002";
}

// FFI bindings to xFlasher.dll
#[link(name = "xFlasher")]
extern "C" {
    /// Perform SPI operation on NAND flash
    ///
    /// # Arguments
    /// * `mode` - Operation mode (see `spi_mode` constants)
    /// * `size` - Size in MB for read/write operations
    /// * `file` - Path to file for read/write
    /// * `startblock` - Starting block number (optional, default 0)
    /// * `length` - Length in blocks (optional, default 0)
    ///
    /// # Returns
    /// * 0 on success
    /// * -1 if cancelled
    /// * -2 if device not initialized
    /// * -3 if console not found
    /// * -4 if unknown NAND
    /// * -11 if file couldn't be opened
    pub fn spi(mode: c_int, size: c_int, file: *const c_char, startblock: c_int, length: c_int) -> c_int;

    /// Get current block count during SPI operation
    ///
    /// # Returns
    /// * Number of blocks processed (>= 0)
    /// * Negative value if still initializing
    pub fn spiGetBlocks() -> c_int;

    /// Get flash configuration from detected NAND
    ///
    /// # Returns
    /// * Flash config value as 32-bit integer (format as hex for display)
    pub fn spiGetConfig() -> c_int;

    /// Stop current SPI operation
    pub fn spiStop();

    /// Read from eMMC storage
    ///
    /// # Arguments
    /// * `file` - Output file path
    /// * `startBlock` - Starting block number
    /// * `blockNum` - Number of blocks to read (default: 98304)
    ///
    /// # Returns
    /// * 0 on success
    /// * Negative values for errors (see error codes)
    pub fn emmc_read(file: *const c_char, startBlock: c_int, blockNum: c_int) -> c_int;

    /// Write to eMMC storage
    ///
    /// # Arguments
    /// * `file` - Input file path
    /// * `startBlock` - Starting block number
    ///
    /// # Returns
    /// * 0 on success
    /// * Negative values for errors (see error codes)
    pub fn emmc_write(file: *const c_char, startBlock: c_int) -> c_int;

    /// Get current block count during eMMC operation
    ///
    /// # Returns
    /// * Number of blocks processed (>= 0)
    /// * Negative value if still initializing
    pub fn emmcGetBlocks() -> c_int;
}

/// Safe wrapper for SPI read operation
///
/// # Arguments
/// * `size_mb` - Size in MB
/// * `filepath` - Output file path
/// * `start_block` - Starting block (default: 0)
/// * `length` - Length in blocks (default: 0 = all)
pub fn spi_read<P: AsRef<Path>>(
    size_mb: i32,
    filepath: P,
    start_block: i32,
    length: i32,
) -> Result<i32, std::ffi::NulError> {
    let path_cstring = std::ffi::CString::new(filepath.as_ref().to_string_lossy().as_ref())?;
    let result = unsafe { spi(spi_mode::READ, size_mb, path_cstring.as_ptr(), start_block, length) };
    Ok(result)
}

/// Safe wrapper for SPI write operation
///
/// # Arguments
/// * `size_mb` - Size in MB
/// * `filepath` - Input file path
/// * `start_block` - Starting block (default: 0)
/// * `length` - Length in blocks (default: 0 = all)
/// * `xell_mode` - Use XeLL write mode if true
pub fn spi_write<P: AsRef<Path>>(
    size_mb: i32,
    filepath: P,
    start_block: i32,
    length: i32,
    xell_mode: bool,
) -> Result<i32, std::ffi::NulError> {
    let path_cstring = std::ffi::CString::new(filepath.as_ref().to_string_lossy().as_ref())?;
    let mode = if xell_mode {
        spi_mode::WRITE_XELL
    } else {
        spi_mode::WRITE
    };
    let result = unsafe { spi(mode, size_mb, path_cstring.as_ptr(), start_block, length) };
    Ok(result)
}

/// Safe wrapper for SPI erase operation
///
/// # Arguments
/// * `size_mb` - Size in MB
/// * `start_block` - Starting block (default: 0)
/// * `length` - Length in blocks (default: 0 = all)
pub fn spi_erase(
    size_mb: i32,
    start_block: i32,
    length: i32,
) -> i32 {
    unsafe { spi(spi_mode::ERASE, size_mb, std::ptr::null(), start_block, length) }
}

/// Safe wrapper for getting flash config
///
/// # Returns
/// * Flash config as i32 (use format!("{:08X}", config) for hex string)
pub fn get_flash_config() -> i32 {
    unsafe { spiGetConfig() }
}

/// Safe wrapper for getting flash config as hex string
///
/// # Returns
/// * 8-character uppercase hex string of the flash config
pub fn get_flash_config_hex() -> String {
    format!("{:08X}", get_flash_config())
}

/// Safe wrapper for eMMC read operation
///
/// # Arguments
/// * `filepath` - Output file path
/// * `start_block` - Starting block
/// * `block_count` - Number of blocks (default: 98304 for full)
pub fn emmc_read_safe<P: AsRef<Path>>(
    filepath: P,
    start_block: i32,
    block_count: i32,
) -> Result<i32, std::ffi::NulError> {
    let path_cstring = std::ffi::CString::new(filepath.as_ref().to_string_lossy().as_ref())?;
    let result = unsafe { emmc_read(path_cstring.as_ptr(), start_block, block_count) };
    Ok(result)
}

/// Safe wrapper for eMMC write operation
///
/// # Arguments
/// * `filepath` - Input file path
/// * `start_block` - Starting block
pub fn emmc_write_safe<P: AsRef<Path>>(
    filepath: P,
    start_block: i32,
) -> Result<i32, std::ffi::NulError> {
    let path_cstring = std::ffi::CString::new(filepath.as_ref().to_string_lossy().as_ref())?;
    let result = unsafe { emmc_write(path_cstring.as_ptr(), start_block) };
    Ok(result)
}

/// Stop any ongoing operation
pub fn stop_operation() {
    unsafe { spiStop() };
}

/// Get current progress in blocks (SPI operation)
///
/// # Returns
/// * Some(blocks) if progress available
/// * None if still initializing
pub fn get_spi_progress() -> Option<i32> {
    let blocks = unsafe { spiGetBlocks() };
    if blocks >= 0 {
        Some(blocks)
    } else {
        None
    }
}

/// Get current progress in blocks (eMMC operation)
///
/// # Returns
/// * Some(blocks) if progress available
/// * None if still initializing
pub fn get_emmc_progress() -> Option<i32> {
    let blocks = unsafe { emmcGetBlocks() };
    if blocks >= 0 {
        Some(blocks)
    } else {
        None
    }
}

/// Calculate percentage progress
///
/// # Arguments
/// * `current` - Current block count
/// * `start` - Starting block
/// * `total` - Total length in blocks
///
/// # Returns
/// * Percentage (0-100)
pub fn calculate_progress(current: i32, start: i32, total: i32) -> i32 {
    if total <= 0 {
        return 0;
    }
    let progress = ((current - start) * 100) / total;
    progress.clamp(0, 100)
}

/// Convert error code to human-readable message
pub fn error_message(code: i32) -> &'static str {
    match code {
        error::SUCCESS => "Success",
        error::CANCELLED => "Operation cancelled",
        error::DEVICE_NOT_INITIALIZED => "Device not initialized",
        error::CONSOLE_NOT_FOUND => "Console not found",
        error::UNKNOWN_NAND => "Unknown NAND type",
        error::FILE_OPEN_ERROR => "Could not open file",
        _ => "Unknown error",
    }
}

/// Check if flash config indicates Corona 4GB (eMMC)
pub fn is_corona_4gb(flash_config_hex: &str) -> bool {
    flash_config_hex == flash_config::CORONA_4GB_EMMC
}

/// Get NAND size description from flash config
///
/// # Arguments
/// * `flash_config_hex` - 8-character hex string
///
/// # Returns
/// * Description string (e.g., "Corona: 16MB")
pub fn get_nand_description(flash_config_hex: &str) -> &'static str {
    match flash_config_hex {
        flash_config::JASPER_TRINITY_16MB => "Jasper, Trinity: 16MB",
        flash_config::CORONA_16MB => "Corona: 16MB",
        flash_config::JASPER_TRINITY_256MB => "Jasper, Trinity: 256MB",
        flash_config::JASPER_TRINITY_512MB => "Jasper, Trinity: 512MB",
        flash_config::CORONA_256MB => "Corona: 256MB",
        flash_config::CORONA_512MB => "Corona: 512MB",
        flash_config::XENON_ZEPHYR_FALCON_16MB => "Xenon, Zephyr, Falcon: 16MB",
        flash_config::XENON_ZEPHYR_FALCON_64MB => "Xenon, Zephyr, Falcon: 64MB",
        flash_config::CORONA_4GB_EMMC => "Corona: 4GB",
        _ => "Unrecognized Flash Config",
    }
}

/// Auto-detect NAND size from flash config
///
/// # Arguments
/// * `flash_config_hex` - 8-character hex string
///
/// # Returns
/// * Some(size_mb) if known, None for eMMC or unrecognized
pub fn auto_detect_size(flash_config_hex: &str) -> Option<i32> {
    match flash_config_hex {
        flash_config::JASPER_TRINITY_16MB |
        flash_config::CORONA_16MB |
        flash_config::XENON_ZEPHYR_FALCON_16MB => Some(16),
        flash_config::XENON_ZEPHYR_FALCON_64MB => Some(64),
        flash_config::JASPER_TRINITY_256MB |
        flash_config::CORONA_256MB => Some(256),
        flash_config::JASPER_TRINITY_512MB |
        flash_config::CORONA_512MB => Some(512),
        flash_config::CORONA_4GB_EMMC => None, // eMMC, use EMMC functions
        _ => None,
    }
}

/// Validate flash config is valid (not zero or all F's)
pub fn is_valid_flash_config(flash_config_hex: &str) -> bool {
    flash_config_hex != "00000000" && flash_config_hex != "FFFFFFFF"
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_flash_config_constants() {
        assert_eq!(flash_config::CORONA_16MB, "00043000");
        assert_eq!(flash_config::CORONA_4GB_EMMC, "C0462002");
    }

    #[test]
    fn test_error_codes() {
        assert_eq!(error::SUCCESS, 0);
        assert_eq!(error::CANCELLED, -1);
        assert_eq!(error::DEVICE_NOT_INITIALIZED, -2);
    }

    #[test]
    fn test_nand_descriptions() {
        assert_eq!(get_nand_description("00043000"), "Corona: 16MB");
        assert_eq!(get_nand_description("C0462002"), "Corona: 4GB");
        assert_eq!(get_nand_description("FFFFFFFF"), "Unrecognized Flash Config");
    }

    #[test]
    fn test_auto_detect_size() {
        assert_eq!(auto_detect_size("00043000"), Some(16));
        assert_eq!(auto_detect_size("00AC3020"), Some(512));
        assert_eq!(auto_detect_size("C0462002"), None); // eMMC
    }

    #[test]
    fn test_is_corona_4gb() {
        assert!(is_corona_4gb("C0462002"));
        assert!(!is_corona_4gb("00043000"));
    }

    #[test]
    fn test_valid_flash_config() {
        assert!(!is_valid_flash_config("00000000"));
        assert!(!is_valid_flash_config("FFFFFFFF"));
        assert!(is_valid_flash_config("00043000"));
    }

    #[test]
    fn test_progress_calculation() {
        assert_eq!(calculate_progress(50, 0, 100), 50);
        assert_eq!(calculate_progress(25, 0, 100), 25);
        assert_eq!(calculate_progress(100, 0, 100), 100);
        assert_eq!(calculate_progress(0, 0, 0), 0);
    }

    #[test]
    fn test_error_messages() {
        assert_eq!(error_message(0), "Success");
        assert_eq!(error_message(-1), "Operation cancelled");
        assert_eq!(error_message(-2), "Device not initialized");
        assert_eq!(error_message(999), "Unknown error");
    }
}
