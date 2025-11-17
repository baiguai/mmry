use eframe::egui;
use global_hotkey::{GlobalHotKeyEvent, GlobalHotKeyManager, HotKeyState};
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::Duration;
use std::fs;
use std::path::PathBuf;
use arboard::Clipboard;
use chrono::{DateTime, Utc};
use serde::{Deserialize, Serialize};
use aes_gcm::{Aes256Gcm, Key, Nonce, KeyInit};
use aes_gcm::aead::{Aead, OsRng};
use base64::{Engine as _, engine::general_purpose};
use rand::RngCore;
use sha2::{Sha256, Digest};
use auto_launch;

fn main() -> Result<(), eframe::Error> {
    // Load config first to get hotkey settings
    let config = load_config();
    
    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default()
            .with_inner_size([600.0, 400.0])
            .with_inner_size([600.0, 400.0])
            .with_min_inner_size([400.0, 200.0])
            .with_always_on_top()
            .with_decorations(false)
            .with_transparent(true),
        ..Default::default()
    };

    let visible = Arc::new(Mutex::new(false));
    let visible_clone = visible.clone();

    // Set up global hotkey
    let hotkey_config = config.hotkey.clone();
    thread::spawn(move || {
        let hotkey_manager = GlobalHotKeyManager::new().unwrap();
        
        // Parse and register hotkey from config
        match parse_hotkey_config(&hotkey_config) {
            Ok(hotkey) => {
                if let Err(e) = hotkey_manager.register(hotkey) {
                    eprintln!("Failed to register global hotkey: {}", e);
                    // Fallback to default Ctrl+Alt+C
                    let fallback_hotkey = global_hotkey::hotkey::HotKey::new(
                        Some(global_hotkey::hotkey::Modifiers::CONTROL | global_hotkey::hotkey::Modifiers::ALT),
                        global_hotkey::hotkey::Code::KeyC,
                    );
                    let _ = hotkey_manager.register(fallback_hotkey);
                }
            }
            Err(e) => {
                eprintln!("Failed to parse hotkey config: {}", e);
                // Fallback to default Ctrl+Alt+C
                let fallback_hotkey = global_hotkey::hotkey::HotKey::new(
                    Some(global_hotkey::hotkey::Modifiers::CONTROL | global_hotkey::hotkey::Modifiers::ALT),
                    global_hotkey::hotkey::Code::KeyC,
                );
                let _ = hotkey_manager.register(fallback_hotkey);
            }
        }
        
        let rx = GlobalHotKeyEvent::receiver();
        while let Ok(event) = rx.recv() {
            if event.state == HotKeyState::Released {
                let mut vis = visible_clone.lock().unwrap();
                *vis = !*vis;
            }
        }
    });

    eframe::run_native(
        "MMRY",
        options,
        Box::new(|cc| {
            setup_custom_fonts(&cc.egui_ctx);
            Ok(Box::new(MyApp::new(visible, config)))
        }),
    )
}

#[derive(Debug, Clone, Serialize, Deserialize)]
struct ClipboardItem {
    content: String,
    timestamp: DateTime<Utc>,
    id: usize,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
struct HotkeyConfig {
    ctrl: bool,
    alt: bool,
    shift: bool,
    key: String,
}

impl Default for HotkeyConfig {
    fn default() -> Self {
        Self {
            ctrl: true,
            alt: true,
            shift: false,
            key: "c".to_string(),
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
struct Config {
    verbose: bool,
    theme: String,
    max_items: usize,
    hotkey: HotkeyConfig,
    encryption_enabled: bool,
    encryption_key: Option<String>,
    autostart_enabled: bool,
}

impl Default for Config {
    fn default() -> Self {
        Self { 
            verbose: false,
            theme: "dark".to_string(),
            max_items: 500,
            hotkey: HotkeyConfig::default(),
            encryption_enabled: true,
            encryption_key: None,
            autostart_enabled: false,
        }
    }
}

#[derive(Debug, Serialize, Deserialize)]
struct BookmarkGroup {
    name: String,
    created_at: DateTime<Utc>,
    clips: Vec<ClipboardItem>,
}

#[derive(Debug, Serialize, Deserialize)]
struct Theme {
    background_color: String,
    text_color: String,
    selected_background_color: String,
    selected_text_color: String,
    selected_border_color: String,
    border_color: String,
    alternate_row_color: String,
}

impl Default for Theme {
    fn default() -> Self {
        Self {
            background_color: "#000000".to_string(),
            text_color: "#FFFFFF".to_string(),
            selected_background_color: "#46465A".to_string(),
            selected_text_color: "#C8C8FF".to_string(),
            selected_border_color: "#6496FF".to_string(),
            border_color: "#3C3C3C".to_string(),
            alternate_row_color: "#282828".to_string(),
        }
    }
}

impl ClipboardItem {
    fn new(content: String, id: usize) -> Self {
        Self {
            content,
            timestamp: Utc::now(),
            id,
        }
    }
    
    fn preview(&self) -> String {
        let preview = self.content.lines().next().unwrap_or(&self.content);
        if preview.len() > 50 {
            format!("{}...", &preview[..47])
        } else {
            preview.to_string()
        }
    }
}

fn load_config() -> Config {
    if let Ok(config_path) = get_config_path() {
        // Create config file if it doesn't exist
        if !config_path.exists() {
            let default_config = Config::default();
            if let Ok(config_json) = serde_json::to_string_pretty(&default_config) {
                let _ = fs::write(&config_path, config_json);
            }
            return default_config;
        }
        
        // Load existing config
        if let Ok(config_content) = fs::read_to_string(&config_path) {
            if let Ok(config) = serde_json::from_str::<Config>(&config_content) {
                return config;
            }
        }
    }
    Config::default()
}

fn get_config_path() -> Result<PathBuf, Box<dyn std::error::Error>> {
    let mut path = dirs::config_dir().ok_or("No config directory found")?;
    path.push("mmry");
    fs::create_dir_all(&path)?;
    path.push("config.json");
    Ok(path)
}

fn get_clipboard_data_path() -> Result<PathBuf, Box<dyn std::error::Error>> {
    let mut path = dirs::config_dir().ok_or("No config directory found")?;
    path.push("mmry");
    fs::create_dir_all(&path)?;
    path.push("clipboard.json");
    Ok(path)
}

fn get_themes_dir() -> Result<PathBuf, Box<dyn std::error::Error>> {
    let mut path = dirs::config_dir().ok_or("No config directory found")?;
    path.push("mmry");
    path.push("themes");
    fs::create_dir_all(&path)?;
    Ok(path)
}

fn get_theme_path(theme_name: &str) -> Result<PathBuf, Box<dyn std::error::Error>> {
    let mut path = get_themes_dir()?;
    path.push(format!("{}.json", theme_name));
    Ok(path)
}

fn get_bookmarks_data_path() -> Result<PathBuf, Box<dyn std::error::Error>> {
    let mut path = dirs::config_dir().ok_or("No config directory found")?;
    path.push("mmry");
    fs::create_dir_all(&path)?;
    path.push("bookmarks.json");
    Ok(path)
}

fn load_theme(theme_name: &str) -> Theme {
    if let Ok(theme_path) = get_theme_path(theme_name) {
        if theme_path.exists() {
            if let Ok(content) = fs::read_to_string(&theme_path) {
                if let Ok(theme) = serde_json::from_str::<Theme>(&content) {
                    return theme;
                }
            }
        }
    }
    
    // Create default dark theme if it doesn't exist
    let default_theme = Theme::default();
    if theme_name == "dark" {
        // Ensure themes directory exists
        let _ = get_themes_dir();
        if let Ok(theme_path) = get_theme_path("dark") {
            if let Ok(theme_json) = serde_json::to_string_pretty(&default_theme) {
                let _ = fs::write(theme_path, theme_json);
            }
        }
    }
    
    default_theme
}

fn get_or_create_encryption_key(config: &Config) -> Result<[u8; 32], Box<dyn std::error::Error>> {
    if let Some(key_str) = &config.encryption_key {
        // Use provided key (hash it to ensure 32 bytes)
        let mut hasher = Sha256::new();
        hasher.update(key_str.as_bytes());
        let hash = hasher.finalize();
        let mut key = [0u8; 32];
        key.copy_from_slice(&hash);
        Ok(key)
    } else {
        // Generate a new random key and save it to config
        let mut key = [0u8; 32];
        OsRng.fill_bytes(&mut key);
        let key_b64 = general_purpose::STANDARD.encode(&key);
        
        // Update config with new key
        let mut updated_config = config.clone();
        updated_config.encryption_key = Some(key_b64);
        save_config(&updated_config)?;
        
        Ok(key)
    }
}

fn encrypt_data(data: &str, key: &[u8; 32]) -> Result<String, Box<dyn std::error::Error>> {
    let cipher = Aes256Gcm::new(Key::<Aes256Gcm>::from_slice(key));
    let mut nonce_bytes = [0u8; 12];
    OsRng.fill_bytes(&mut nonce_bytes);
    let nonce = Nonce::from_slice(&nonce_bytes);
    
    let ciphertext = cipher.encrypt(nonce, data.as_bytes())
        .map_err(|e| format!("Encryption failed: {:?}", e))?;
    
    // Combine nonce and ciphertext, then encode as base64
    let mut combined = nonce_bytes.to_vec();
    combined.extend_from_slice(&ciphertext);
    Ok(general_purpose::STANDARD.encode(&combined))
}

fn decrypt_data(encrypted_data: &str, key: &[u8; 32]) -> Result<String, Box<dyn std::error::Error>> {
    let combined = general_purpose::STANDARD.decode(encrypted_data)?;
    
    if combined.len() < 12 {
        return Err("Invalid encrypted data".into());
    }
    
    let (nonce_bytes, ciphertext) = combined.split_at(12);
    let nonce = Nonce::from_slice(nonce_bytes);
    
    let cipher = Aes256Gcm::new(Key::<Aes256Gcm>::from_slice(key));
    let plaintext = cipher.decrypt(nonce, ciphertext)
        .map_err(|e| format!("Decryption failed: {:?}", e))?;
    
    Ok(String::from_utf8(plaintext)?)
}

fn save_config(config: &Config) -> Result<(), Box<dyn std::error::Error>> {
    let path = get_config_path()?;
    let json = serde_json::to_string_pretty(config)?;
    fs::write(path, json)?;
    Ok(())
}

fn enable_autostart() -> Result<(), Box<dyn std::error::Error>> {
    let exe_path = std::env::current_exe()?;
    let auto = auto_launch::AutoLaunchBuilder::new()
        .set_app_name("mmry")
        .set_app_path(&exe_path.to_string_lossy())
        .build()?;
    
    auto.enable()?;
    Ok(())
}

fn disable_autostart() -> Result<(), Box<dyn std::error::Error>> {
    let exe_path = std::env::current_exe()?;
    let auto = auto_launch::AutoLaunchBuilder::new()
        .set_app_name("mmry")
        .set_app_path(&exe_path.to_string_lossy())
        .build()?;
    
    auto.disable()?;
    Ok(())
}

fn is_autostart_enabled() -> Result<bool, Box<dyn std::error::Error>> {
    let exe_path = std::env::current_exe()?;
    let auto = auto_launch::AutoLaunchBuilder::new()
        .set_app_name("mmry")
        .set_app_path(&exe_path.to_string_lossy())
        .build()?;
    
    Ok(auto.is_enabled()?)
}

fn hex_to_color(hex: &str) -> egui::Color32 {
    let hex = hex.trim_start_matches('#');
    if hex.len() == 6 {
        let r = u8::from_str_radix(&hex[0..2], 16).unwrap_or(0);
        let g = u8::from_str_radix(&hex[2..4], 16).unwrap_or(0);
        let b = u8::from_str_radix(&hex[4..6], 16).unwrap_or(0);
        egui::Color32::from_rgb(r, g, b)
    } else {
        egui::Color32::WHITE
    }
}

fn save_clipboard_items(items: &[ClipboardItem], config: &Config) -> Result<(), Box<dyn std::error::Error>> {
    let path = get_clipboard_data_path()?;
    let json = serde_json::to_string_pretty(items)?;
    
    if config.encryption_enabled {
        let key = get_or_create_encryption_key(config)?;
        let encrypted = encrypt_data(&json, &key)?;
        fs::write(&path, encrypted)?;
    } else {
        fs::write(&path, json)?;
    }
    Ok(())
}

fn load_clipboard_items(config: &Config) -> Vec<ClipboardItem> {
    if let Ok(path) = get_clipboard_data_path() {
        if path.exists() {
            if let Ok(content) = fs::read_to_string(&path) {
                // Try to parse as JSON first
                if let Ok(items) = serde_json::from_str::<Vec<ClipboardItem>>(&content) {
                    return items;
                }
                
                // If parsing fails and encryption is enabled, try to decrypt
                if config.encryption_enabled {
                    if let Ok(key) = get_or_create_encryption_key(config) {
                        if let Ok(decrypted) = decrypt_data(&content, &key) {
                            if let Ok(items) = serde_json::from_str::<Vec<ClipboardItem>>(&decrypted) {
                                return items;
                            }
                        }
                    }
                }
            }
        }
    }
    Vec::new()
}

fn save_bookmark_groups(groups: &[BookmarkGroup], config: &Config) -> Result<(), Box<dyn std::error::Error>> {
    let path = get_bookmarks_data_path()?;
    let json = serde_json::to_string_pretty(groups)?;
    
    if config.encryption_enabled {
        let key = get_or_create_encryption_key(config)?;
        let encrypted = encrypt_data(&json, &key)?;
        fs::write(path, encrypted)?;
    } else {
        fs::write(path, json)?;
    }
    Ok(())
}

fn load_bookmark_groups(config: &Config) -> Vec<BookmarkGroup> {
    if let Ok(path) = get_bookmarks_data_path() {
        if path.exists() {
            if let Ok(content) = fs::read_to_string(&path) {
                // Try to parse as new format first
                if let Ok(groups) = serde_json::from_str::<Vec<BookmarkGroup>>(&content) {
                    return groups;
                } else {
                    // Try to parse as old format for backward compatibility
                    if let Ok(groups) = parse_old_bookmark_format(&content) {
                        return groups;
                    }
                }
                
                // If parsing fails and encryption is enabled, try to decrypt
                if config.encryption_enabled {
                    if let Ok(key) = get_or_create_encryption_key(config) {
                        if let Ok(decrypted) = decrypt_data(&content, &key) {
                            // Try to parse decrypted content
                            if let Ok(groups) = serde_json::from_str::<Vec<BookmarkGroup>>(&decrypted) {
                                return groups;
                            } else {
                                // Try to parse as old format for backward compatibility
                                if let Ok(groups) = parse_old_bookmark_format(&decrypted) {
                                    return groups;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    Vec::new()
}

fn parse_old_bookmark_format(content: &str) -> Result<Vec<BookmarkGroup>, Box<dyn std::error::Error>> {
    #[derive(Debug, Serialize, Deserialize)]
    struct OldBookmarkGroup {
        name: String,
        created_at: DateTime<Utc>,
    }
    
    if let Ok(old_groups) = serde_json::from_str::<Vec<OldBookmarkGroup>>(content) {
        // Convert old format to new format
        Ok(old_groups.into_iter().map(|old_group| BookmarkGroup {
            name: old_group.name,
            created_at: old_group.created_at,
            clips: Vec::new(),
        }).collect())
    } else {
        Err("Failed to parse old bookmark format".into())
    }
}

fn parse_hotkey_config(config: &HotkeyConfig) -> Result<global_hotkey::hotkey::HotKey, Box<dyn std::error::Error>> {
    let mut modifiers = global_hotkey::hotkey::Modifiers::empty();
    
    if config.ctrl {
        modifiers |= global_hotkey::hotkey::Modifiers::CONTROL;
    }
    if config.alt {
        modifiers |= global_hotkey::hotkey::Modifiers::ALT;
    }
    if config.shift {
        modifiers |= global_hotkey::hotkey::Modifiers::SHIFT;
    }
    
    let code = match config.key.to_lowercase().as_str() {
        "a" => global_hotkey::hotkey::Code::KeyA,
        "b" => global_hotkey::hotkey::Code::KeyB,
        "c" => global_hotkey::hotkey::Code::KeyC,
        "d" => global_hotkey::hotkey::Code::KeyD,
        "e" => global_hotkey::hotkey::Code::KeyE,
        "f" => global_hotkey::hotkey::Code::KeyF,
        "g" => global_hotkey::hotkey::Code::KeyG,
        "h" => global_hotkey::hotkey::Code::KeyH,
        "i" => global_hotkey::hotkey::Code::KeyI,
        "j" => global_hotkey::hotkey::Code::KeyJ,
        "k" => global_hotkey::hotkey::Code::KeyK,
        "l" => global_hotkey::hotkey::Code::KeyL,
        "m" => global_hotkey::hotkey::Code::KeyM,
        "n" => global_hotkey::hotkey::Code::KeyN,
        "o" => global_hotkey::hotkey::Code::KeyO,
        "p" => global_hotkey::hotkey::Code::KeyP,
        "q" => global_hotkey::hotkey::Code::KeyQ,
        "r" => global_hotkey::hotkey::Code::KeyR,
        "s" => global_hotkey::hotkey::Code::KeyS,
        "t" => global_hotkey::hotkey::Code::KeyT,
        "u" => global_hotkey::hotkey::Code::KeyU,
        "v" => global_hotkey::hotkey::Code::KeyV,
        "w" => global_hotkey::hotkey::Code::KeyW,
        "x" => global_hotkey::hotkey::Code::KeyX,
        "y" => global_hotkey::hotkey::Code::KeyY,
        "z" => global_hotkey::hotkey::Code::KeyZ,
        "0" => global_hotkey::hotkey::Code::Digit0,
        "1" => global_hotkey::hotkey::Code::Digit1,
        "2" => global_hotkey::hotkey::Code::Digit2,
        "3" => global_hotkey::hotkey::Code::Digit3,
        "4" => global_hotkey::hotkey::Code::Digit4,
        "5" => global_hotkey::hotkey::Code::Digit5,
        "6" => global_hotkey::hotkey::Code::Digit6,
        "7" => global_hotkey::hotkey::Code::Digit7,
        "8" => global_hotkey::hotkey::Code::Digit8,
        "9" => global_hotkey::hotkey::Code::Digit9,
        "space" => global_hotkey::hotkey::Code::Space,
        "enter" => global_hotkey::hotkey::Code::Enter,
        "escape" => global_hotkey::hotkey::Code::Escape,
        "tab" => global_hotkey::hotkey::Code::Tab,
        "backspace" => global_hotkey::hotkey::Code::Backspace,
        "delete" => global_hotkey::hotkey::Code::Delete,
        "insert" => global_hotkey::hotkey::Code::Insert,
        "home" => global_hotkey::hotkey::Code::Home,
        "end" => global_hotkey::hotkey::Code::End,
        "pageup" => global_hotkey::hotkey::Code::PageUp,
        "pagedown" => global_hotkey::hotkey::Code::PageDown,
        "arrowup" => global_hotkey::hotkey::Code::ArrowUp,
        "arrowdown" => global_hotkey::hotkey::Code::ArrowDown,
        "arrowleft" => global_hotkey::hotkey::Code::ArrowLeft,
        "arrowright" => global_hotkey::hotkey::Code::ArrowRight,
        "f1" => global_hotkey::hotkey::Code::F1,
        "f2" => global_hotkey::hotkey::Code::F2,
        "f3" => global_hotkey::hotkey::Code::F3,
        "f4" => global_hotkey::hotkey::Code::F4,
        "f5" => global_hotkey::hotkey::Code::F5,
        "f6" => global_hotkey::hotkey::Code::F6,
        "f7" => global_hotkey::hotkey::Code::F7,
        "f8" => global_hotkey::hotkey::Code::F8,
        "f9" => global_hotkey::hotkey::Code::F9,
        "f10" => global_hotkey::hotkey::Code::F10,
        "f11" => global_hotkey::hotkey::Code::F11,
        "f12" => global_hotkey::hotkey::Code::F12,
        _ => return Err(format!("Unsupported key: {}", config.key).into()),
    };
    
    let hotkey = if modifiers.is_empty() {
        global_hotkey::hotkey::HotKey::new(None, code)
    } else {
        global_hotkey::hotkey::HotKey::new(Some(modifiers), code)
    };
    
    Ok(hotkey)
}

fn setup_custom_fonts(ctx: &egui::Context) {
    // Use default fonts with monospace family
    let mut fonts = egui::FontDefinitions::default();
    
    // Ensure monospace font family exists
    fonts.families.entry(egui::FontFamily::Monospace).or_default();
    
    ctx.set_fonts(fonts);
}

#[derive(Debug, Clone, PartialEq)]
enum FilterMode {
    None,
    Generous,  // '/' - contains match
}

#[derive(Debug, Clone, PartialEq)]
enum Context {
    Help,             // Help dialog
    Always,           // Always available
    ClipsList,        // When viewing clips list (not filtering)
    Filtering,        // When filtering clips
    BookmarkGroups,   // When viewing bookmark groups
    BookmarkClips,    // When viewing clips in a bookmark group
}

#[derive(Debug, Clone)]
struct KeyBinding {
    key: String,
    description: String,
    context: Context,
}

impl KeyBinding {
    fn new(key: &str, description: &str, context: Context) -> Self {
        Self {
            key: key.to_string(),
            description: description.to_string(),
            context,
        }
    }
}

struct MyApp {
    visible: Arc<Mutex<bool>>,
    should_quit: bool,
    clipboard_items: Arc<Mutex<Vec<ClipboardItem>>>,
    #[allow(dead_code)]
    last_clipboard_content: Arc<Mutex<Option<String>>>,
    #[allow(dead_code)]
    next_id: Arc<Mutex<usize>>,
    config: Config,
    theme: Theme,
    selected_clipboard_index: usize,
    gg_pressed: bool,
    filter_mode: FilterMode,
    filter_text: String,
    filter_active: bool,  // Whether filter is currently applied (after Enter)
    show_filter_input: bool,  // Whether to show TextEdit widget
    is_filtering: bool,  // Whether we're currently in filter mode (editing or applied)
    original_selected_index: usize,
    bookmark_groups: Vec<BookmarkGroup>,
    show_bookmark_groups: bool,
    show_bookmark_input: bool,  // Whether to show text input for creating groups
    new_bookmark_group_name: String,
    selected_bookmark_group_index: usize,
    bookmark_gg_pressed: bool,
    selected_bookmark_clip_index: usize,
    bookmark_clip_gg_pressed: bool,
    selected_bookmark_group_for_clips: Option<String>,
    bookmark_access_mode: bool,  // true for access mode (backtick), false for add mode (m)
    just_switched_to_clips: bool,  // prevent immediate Enter key handling after switching to clips view
    show_settings: bool,  // Whether to show settings dialog
    just_started_filtering: bool,  // prevent initial / from being added to filter text
    show_help: bool,  // Whether to show help dialog
    help_scroll_offset: f32,  // Scroll position for help dialog
    
}

impl MyApp {
    fn new(visible: Arc<Mutex<bool>>, config: Config) -> Self {
        let theme = load_theme(&config.theme);
        let loaded_items = load_clipboard_items(&config);
        let next_id = loaded_items.iter().map(|item| item.id).max().unwrap_or(0) + 1;
        let bookmark_groups = load_bookmark_groups(&config);
        
        let clipboard_items = Arc::new(Mutex::new(loaded_items));
        let last_clipboard_content = Arc::new(Mutex::new(None));
        let next_id = Arc::new(Mutex::new(next_id));
        
        // Ensure encryption key exists before starting monitoring thread
        let config_with_key = if config.encryption_enabled {
            match get_or_create_encryption_key(&config) {
                Ok(_) => config.clone(), // Key exists or was created successfully
                Err(e) => {
                    eprintln!("Failed to initialize encryption: {}", e);
                    // Disable encryption if key creation fails
                    let mut fallback_config = config.clone();
                    fallback_config.encryption_enabled = false;
                    fallback_config
                }
            }
        } else {
            config.clone()
        };

        // Start clipboard monitoring thread
        Self::start_clipboard_monitoring(
            clipboard_items.clone(),
            last_clipboard_content.clone(),
            next_id.clone(),
            config_with_key,
        );
        
        Self { 
            visible,
            should_quit: false,
            clipboard_items,
            last_clipboard_content,
            next_id,
            config,
            theme,
            selected_clipboard_index: 0,
            gg_pressed: false,
            filter_mode: FilterMode::None,
            filter_text: String::new(),
            filter_active: false,
            show_filter_input: false,
            is_filtering: false,
            original_selected_index: 0,
            bookmark_groups,
            show_bookmark_groups: false,
            show_bookmark_input: false,
            new_bookmark_group_name: String::new(),
            selected_bookmark_group_index: 0,
            bookmark_gg_pressed: false,
            selected_bookmark_clip_index: 0,
            bookmark_clip_gg_pressed: false,
            selected_bookmark_group_for_clips: None,
            bookmark_access_mode: false,
            just_switched_to_clips: false,
            show_settings: false,
            just_started_filtering: false,
            show_help: false,
            help_scroll_offset: 0.0,
            
        }
    }
    
    fn start_clipboard_monitoring(
        items: Arc<Mutex<Vec<ClipboardItem>>>,
        last_content: Arc<Mutex<Option<String>>>,
        next_id: Arc<Mutex<usize>>,
        config: Config,
    ) {
        thread::spawn(move || {
            let mut clipboard = Clipboard::new().expect("Failed to initialize clipboard");
            
            loop {
                if let Ok(content) = clipboard.get_text() {
                    let mut last = last_content.lock().unwrap();
                    
                    if last.as_ref() != Some(&content) {
                        *last = Some(content.clone());
                        
                        let mut items_vec = items.lock().unwrap();
                        let mut id = next_id.lock().unwrap();
                        
                        // Check if item already exists
                        if let Some(existing_index) = items_vec.iter().position(|item| item.content == content) {
                            // Move existing item to top and update timestamp
                            let mut item = items_vec.remove(existing_index);
                            item.timestamp = Utc::now();
                            items_vec.insert(0, item);
                        } else {
                            // Add new item to the beginning
                            items_vec.insert(0, ClipboardItem::new(content.clone(), *id));
                            *id += 1;
                        }
                        
                        // Keep only last max_items items
                        let max_items = config.max_items;
                        if items_vec.len() > max_items {
                            items_vec.truncate(max_items);
                        }
                        
                        // Save to file
                        if let Err(e) = save_clipboard_items(&items_vec, &config) {
                            eprintln!("Failed to save clipboard items: {}", e);
                        }
                    }
                }
                
                thread::sleep(Duration::from_millis(1000));  // Reduced polling frequency from 500ms to 1000ms
            }
        });
    }
}

impl eframe::App for MyApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        // Theme is now only loaded at startup for better performance
        
        // Handle keyboard input
        let mut bookmark_enter_handled = false;
        let mut bookmark_clip_enter_handled = false;
        ctx.input(|i| {
            if i.key_pressed(egui::Key::Escape) {
                if self.show_help {
                    self.show_help = false;
                } else if self.show_settings {
                    self.show_settings = false;
                } else if self.show_bookmark_groups && self.selected_bookmark_group_for_clips.is_some() {
                    // Go back from clips view to groups view
                    self.selected_bookmark_group_for_clips = None;
                    self.selected_bookmark_clip_index = 0;
                    self.bookmark_clip_gg_pressed = false;
                    self.just_switched_to_clips = false;
                } else if self.show_bookmark_groups {
                    // Hide bookmark groups view
                    self.show_bookmark_groups = false;
                    self.show_bookmark_input = false;
                    self.new_bookmark_group_name.clear();
                    self.selected_bookmark_group_index = 0;
                    self.bookmark_gg_pressed = false;
                    self.bookmark_access_mode = false;
                    self.just_switched_to_clips = false;
                } else if self.filter_mode != FilterMode::None {
                    // Cancel filter mode - clear filter completely
                    self.filter_mode = FilterMode::None;
                    self.show_filter_input = false;
                    self.is_filtering = false;
                    self.filter_text.clear();
                    self.filter_active = false;
                    self.selected_clipboard_index = self.original_selected_index;
                } else if self.filter_active {
                    // Escape when filter is active but not editing - clear filter
                    self.filter_text.clear();
                    self.filter_active = false;
                    self.is_filtering = false;
                    self.selected_clipboard_index = 0;
                } else {
                    // Close window
                    let mut vis = self.visible.lock().unwrap();
                    *vis = false;
                }
            }
            
            if i.modifiers.ctrl && i.key_pressed(egui::Key::Q) && !self.show_help {
                self.should_quit = true;
            }
            
            if i.modifiers.ctrl && i.key_pressed(egui::Key::S) && !self.show_help {
                self.show_settings = !self.show_settings;
            }
            
            // Help keybinding - ? to show help dialog (only when not in other dialogs)
            if !self.show_help && !self.show_settings && !self.show_bookmark_groups && !self.is_filtering {
                for event in &i.events {
                    if let egui::Event::Text(text) = event {
                        if text == "?" {
                            self.show_help = true;
                            self.help_scroll_offset = 0.0; // Reset scroll when opening help
                        }
                    }
                }
            }
            
            // Filter keybindings
            if self.filter_mode == FilterMode::None && !self.show_bookmark_groups && !self.show_help {
                // / - start generous filter
                if i.key_pressed(egui::Key::Slash) && !i.modifiers.ctrl && !i.modifiers.alt && !i.modifiers.shift {
                    self.filter_mode = FilterMode::Generous;
                    self.show_filter_input = true;
                    self.is_filtering = true;
                    self.filter_text.clear();
                    self.original_selected_index = self.selected_clipboard_index;
                    self.selected_clipboard_index = 0;
                    self.just_started_filtering = true;  // Flag to prevent initial / from being added
                }
            }
            
            // M/m - show bookmark groups dialog (only when not filtering)
            if !self.show_bookmark_groups && !self.is_filtering && !self.show_help {
                for event in &i.events {
                    if let egui::Event::Key { key, pressed: true, .. } = event {
                        if *key == egui::Key::M && !i.modifiers.ctrl && !i.modifiers.alt {
                            if i.modifiers.shift {
                                // Shift+M - show dialog with input for creating groups
                                self.show_bookmark_groups = true;
                                self.show_bookmark_input = true;
                                self.new_bookmark_group_name.clear();
                                self.bookmark_access_mode = false;  // Set add mode flag
                            } else {
                                // m (lowercase) - show dialog without input for adding clips
                                self.show_bookmark_groups = true;
                                self.show_bookmark_input = false;
                                self.new_bookmark_group_name.clear();
                                self.selected_bookmark_group_index = 0;
                                self.bookmark_gg_pressed = false;
                                self.bookmark_access_mode = false;  // Set add mode flag
                            }
                        }
                    }
                }
            }
            
            // Vim navigation for bookmark groups (when in bookmark dialog and not creating groups)
            if self.show_bookmark_groups && !self.show_bookmark_input {
                // Get the effective list length (filtered or all groups)
                let effective_bookmark_len = if self.show_bookmark_input && !self.new_bookmark_group_name.is_empty() {
                    let filter_lower = self.new_bookmark_group_name.to_lowercase();
                    self.bookmark_groups.iter()
                        .filter(|group| group.name.to_lowercase().contains(&filter_lower))
                        .count()
                } else {
                    self.bookmark_groups.len()
                };
                
                if effective_bookmark_len > 0 {
                    // j - move down
                    if i.key_pressed(egui::Key::J) && !i.modifiers.ctrl && !i.modifiers.alt && !i.modifiers.shift {
                        if self.selected_bookmark_group_index < effective_bookmark_len - 1 {
                            self.selected_bookmark_group_index += 1;
                        }
                    }
                    
                    // k - move up
                    if i.key_pressed(egui::Key::K) && !i.modifiers.ctrl && !i.modifiers.alt && !i.modifiers.shift {
                        if self.selected_bookmark_group_index > 0 {
                            self.selected_bookmark_group_index -= 1;
                        }
                    }
                    
                    // G - go to last item (Shift+G)
                    if i.key_pressed(egui::Key::G) && i.modifiers.shift && !i.modifiers.ctrl && !i.modifiers.alt {
                        self.selected_bookmark_group_index = effective_bookmark_len - 1;
                    }
                    
                    // g - first part of gg
                    if i.key_pressed(egui::Key::G) && !i.modifiers.shift && !i.modifiers.ctrl && !i.modifiers.alt {
                        if self.bookmark_gg_pressed {
                            // gg - go to first item
                            self.selected_bookmark_group_index = 0;
                            self.bookmark_gg_pressed = false;
                        } else {
                            // First g pressed, wait for second g
                            self.bookmark_gg_pressed = true;
                        }
                    }
                    
                    // D - delete selected bookmark group (Shift+D)
                    if i.key_pressed(egui::Key::D) && i.modifiers.shift && !i.modifiers.ctrl && !i.modifiers.alt {
                        if !self.bookmark_groups.is_empty() {
                            // Get the actual group index to delete (works for both filtered and unfiltered)
                            let group_index_to_delete = if self.show_bookmark_input && !self.new_bookmark_group_name.is_empty() {
                                let filter_lower = self.new_bookmark_group_name.to_lowercase();
                                let filtered_indices: Vec<usize> = self.bookmark_groups.iter()
                                    .enumerate()
                                    .filter(|(_, group)| group.name.to_lowercase().contains(&filter_lower))
                                    .map(|(index, _)| index)
                                    .collect();
                                filtered_indices.get(self.selected_bookmark_group_index).copied()
                            } else {
                                Some(self.selected_bookmark_group_index)
                            };
                            
                            if let Some(index_to_delete) = group_index_to_delete {
                                if index_to_delete < self.bookmark_groups.len() {
                                    self.bookmark_groups.remove(index_to_delete);
                                    
                                    // Adjust selection if needed
                                    if self.bookmark_groups.is_empty() {
                                        self.selected_bookmark_group_index = 0;
                                    } else if self.selected_bookmark_group_index >= self.bookmark_groups.len() {
                                        self.selected_bookmark_group_index = self.bookmark_groups.len() - 1;
                                    }
                                    
                                    // Save to file
                                    if let Err(e) = save_bookmark_groups(&self.bookmark_groups, &self.config) {
                                        eprintln!("Failed to save bookmark groups: {}", e);
                                    }
                        }
                    }
                }
            }

                }
            }
            
            // Backtick - show bookmark groups for accessing clips (only when not filtering)
            if !self.show_bookmark_groups && !self.is_filtering && !self.show_help {
                for event in &i.events {
                    if let egui::Event::Key { key, pressed: true, .. } = event {
                        if *key == egui::Key::Backtick && !i.modifiers.ctrl && !i.modifiers.alt && !i.modifiers.shift {
                            self.show_bookmark_groups = true;
                            self.show_bookmark_input = false;
                            self.new_bookmark_group_name.clear();
                            self.selected_bookmark_group_index = 0;
                            self.bookmark_gg_pressed = false;
                            self.bookmark_access_mode = true;  // Set access mode flag
                        }
                    }
                }
            }
            
            if self.is_filtering {
                // Handle backspace key press
                if i.key_pressed(egui::Key::Backspace) {
                    self.filter_text.pop();
                }
                
                // Handle text input for filter (only when actively filtering)
                for event in &i.events {
                    if let egui::Event::Text(text) = event {
                        if !text.chars().any(|c| c.is_control()) {
                            // Skip the initial / that triggered filter mode
                            if self.just_started_filtering && text == "/" {
                                self.just_started_filtering = false;
                                continue;
                            }
                            // Regular text input
                            self.filter_text.push_str(text);
                            self.just_started_filtering = false;  // Reset flag after any other input
                        }
                    }
                }
                
                // Handle up/down arrows when filtering
                if self.is_filtering {
                    let effective_len = if self.filter_active || self.filter_mode != FilterMode::None {
                        let items = self.clipboard_items.lock().unwrap();
                        let filter_lower = self.filter_text.to_lowercase();
                        items.iter().filter(|item| {
                            let content_lower = item.content.to_lowercase();
                            content_lower.contains(&filter_lower)
                        }).count()
                    } else {
                        0
                    };
                    
                    if effective_len > 0 {
                        // Up arrow
                        if i.key_pressed(egui::Key::ArrowUp) && !i.modifiers.ctrl && !i.modifiers.alt && !i.modifiers.shift {
                            if self.selected_clipboard_index > 0 {
                                self.selected_clipboard_index -= 1;
                            }
                        }
                        
                        // Down arrow
                        if i.key_pressed(egui::Key::ArrowDown) && !i.modifiers.ctrl && !i.modifiers.alt && !i.modifiers.shift {
                            if self.selected_clipboard_index < effective_len - 1 {
                                self.selected_clipboard_index += 1;
                            }
                        }
                    }
                }
            }
            
            // Help dialog navigation with j/k
            if self.show_help {
                // j - scroll down
                if i.key_pressed(egui::Key::J) && !i.modifiers.ctrl && !i.modifiers.alt && !i.modifiers.shift {
                    self.help_scroll_offset += 30.0; // Scroll down by 30 pixels
                    self.help_scroll_offset = self.help_scroll_offset.max(0.0);
                }
                
                // k - scroll up  
                if i.key_pressed(egui::Key::K) && !i.modifiers.ctrl && !i.modifiers.alt && !i.modifiers.shift {
                    self.help_scroll_offset -= 30.0; // Scroll up by 30 pixels
                    self.help_scroll_offset = self.help_scroll_offset.max(0.0);
                }
            }
            
            // Vim navigation keybindings (when not editing filter)
            
            // Vim navigation keybindings (when not editing filter)
            if self.filter_mode == FilterMode::None && !self.show_help {
                // Get the effective list length (filtered or all items)
                let effective_len = if self.filter_active {
                    let items = self.clipboard_items.lock().unwrap();
                    let filter_lower = self.filter_text.to_lowercase();
                    items.iter().filter(|item| {
                        let content_lower = item.content.to_lowercase();
                        content_lower.contains(&filter_lower)
                    }).count()
                } else {
                    self.clipboard_items.lock().unwrap().len()
                };
                
                if effective_len > 0 {
                    // j - move down
                    if i.key_pressed(egui::Key::J) && !i.modifiers.ctrl && !i.modifiers.alt && !i.modifiers.shift {
                        if self.selected_clipboard_index < effective_len - 1 {
                            self.selected_clipboard_index += 1;
                        }
                    }
                    
                    // k - move up
                    if i.key_pressed(egui::Key::K) && !i.modifiers.ctrl && !i.modifiers.alt && !i.modifiers.shift {
                        if self.selected_clipboard_index > 0 {
                            self.selected_clipboard_index -= 1;
                        }
                    }
                    
                    // G - go to last item (Shift+G)
                    if i.key_pressed(egui::Key::G) && i.modifiers.shift && !i.modifiers.ctrl && !i.modifiers.alt {
                        self.selected_clipboard_index = effective_len - 1;
                    }
                    
                    // g - first part of gg
                    if i.key_pressed(egui::Key::G) && !i.modifiers.shift && !i.modifiers.ctrl && !i.modifiers.alt {
                        if self.gg_pressed {
                            // gg - go to first item
                            self.selected_clipboard_index = 0;
                            self.gg_pressed = false;
                        } else {
                            // First g pressed, wait for second g
                            self.gg_pressed = true;
                        }
                    }
                }
            }
            
            // Vim navigation for bookmark clips (when viewing clips in a group)
            if self.show_bookmark_groups && self.selected_bookmark_group_for_clips.is_some() {
                // Check if any navigation key is pressed and reset the flag if needed
                let nav_key_pressed = i.key_pressed(egui::Key::J) || i.key_pressed(egui::Key::K) || 
                                     i.key_pressed(egui::Key::G);
                
                if nav_key_pressed && self.just_switched_to_clips {
                    self.just_switched_to_clips = false;
                }
                
                if !self.just_switched_to_clips {
                    if let Some(group_name) = &self.selected_bookmark_group_for_clips {
                        // Get clips length for navigation bounds checking
                        let clips_len = if let Some(group) = self.bookmark_groups.iter().find(|g| g.name == *group_name) {
                            group.clips.len()
                        } else {
                            0
                        };
                        
                        if clips_len > 0 {
                            // j - move down
                            if i.key_pressed(egui::Key::J) && !i.modifiers.ctrl && !i.modifiers.alt && !i.modifiers.shift {
                                if self.selected_bookmark_clip_index < clips_len - 1 {
                                    self.selected_bookmark_clip_index += 1;
                                }
                            }
                            
                            // k - move up
                            if i.key_pressed(egui::Key::K) && !i.modifiers.ctrl && !i.modifiers.alt && !i.modifiers.shift {
                                if self.selected_bookmark_clip_index > 0 {
                                    self.selected_bookmark_clip_index -= 1;
                                }
                            }
                            
                            // G - go to last item (Shift+G)
                            if i.key_pressed(egui::Key::G) && i.modifiers.shift && !i.modifiers.ctrl && !i.modifiers.alt {
                                self.selected_bookmark_clip_index = clips_len - 1;
                            }
                            
                            // g - first part of gg
                            if i.key_pressed(egui::Key::G) && !i.modifiers.shift && !i.modifiers.ctrl && !i.modifiers.alt {
                                if self.bookmark_clip_gg_pressed {
                                    // gg - go to first item
                                    self.selected_bookmark_clip_index = 0;
                                    self.bookmark_clip_gg_pressed = false;
                                } else {
                                    // First g pressed, wait for second g
                                    self.bookmark_clip_gg_pressed = true;
                                }
                            }
                            
                            // d - remove selected clip from bookmark group
                            if i.key_pressed(egui::Key::D) && !i.modifiers.shift && !i.modifiers.ctrl && !i.modifiers.alt {
                                if let Some(group) = self.bookmark_groups.iter_mut().find(|g| g.name == *group_name) {
                                    if self.selected_bookmark_clip_index < group.clips.len() {
                                        group.clips.remove(self.selected_bookmark_clip_index);
                                        
                                        // Adjust selection if needed
                                        if group.clips.is_empty() {
                                            self.selected_bookmark_clip_index = 0;
                                        } else if self.selected_bookmark_clip_index >= group.clips.len() {
                                            self.selected_bookmark_clip_index = group.clips.len() - 1;
                                        }
                                        
                                        // Save to file
                                        if let Err(e) = save_bookmark_groups(&self.bookmark_groups, &self.config) {
                                            eprintln!("Failed to save bookmark groups: {}", e);
                                        }
                                    }
                                }
                            }
                            
                            // Enter - copy selected bookmark clip to clipboard
                            if i.key_pressed(egui::Key::Enter) && !i.modifiers.ctrl && !i.modifiers.alt && !i.modifiers.shift {
                                if let Some(group) = self.bookmark_groups.iter().find(|g| g.name == *group_name) {
                                    if let Some(clip) = group.clips.get(self.selected_bookmark_clip_index) {
                                        // Copy to clipboard using the same method as main clipboard
                                        match Clipboard::new() {
                                            Ok(mut clipboard) => {
                                                let content_to_copy = clip.content.clone();
                                                match clipboard.set_text(content_to_copy) {
                                                    Ok(()) => {
                                                        bookmark_clip_enter_handled = true;
                                                        // Success - close everything
                                                        self.show_bookmark_groups = false;
                                                        self.selected_bookmark_group_index = 0;
                                                        self.bookmark_gg_pressed = false;
                                                        self.selected_bookmark_clip_index = 0;
                                                        self.bookmark_clip_gg_pressed = false;
                                                        self.selected_bookmark_group_for_clips = None;
                                                        self.just_switched_to_clips = false;
                                                        
                                                        // Close window
                                                        let mut vis = self.visible.lock().unwrap();
                                                        *vis = false;
                                                    }
                                                    Err(e) => {
                                                        eprintln!("Failed to set clipboard text: {}", e);
                                                    }
                                                }
                                            }
                                            Err(e) => {
                                                eprintln!("Failed to initialize clipboard: {}", e);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            

            
            // D - delete selected item (Shift+D)
            if i.key_pressed(egui::Key::D) && i.modifiers.shift && !i.modifiers.ctrl && !i.modifiers.alt && !self.show_help {
                let mut items_vec = self.clipboard_items.lock().unwrap();
                if !items_vec.is_empty() && self.selected_clipboard_index < items_vec.len() {
                    items_vec.remove(self.selected_clipboard_index);
                    
                    // Adjust selection if needed
                    if items_vec.is_empty() {
                        self.selected_clipboard_index = 0;
                    } else if self.selected_clipboard_index >= items_vec.len() {
                        self.selected_clipboard_index = items_vec.len() - 1;
                    }
                    
                        // Save to file
                        if let Err(e) = save_clipboard_items(&items_vec, &self.config) {
                            eprintln!("Failed to save clipboard items: {}", e);
                        }
                }
            }
            
            // Handle Enter key for bookmark groups (before main Enter handler)
            if self.show_bookmark_groups && !self.show_bookmark_input && self.selected_bookmark_group_for_clips.is_none() && i.key_pressed(egui::Key::Enter) && !i.modifiers.ctrl && !i.modifiers.alt && !i.modifiers.shift && !self.show_help {
                // Get the selected bookmark group index in the full list
                let selected_group_index = if self.show_bookmark_input && !self.new_bookmark_group_name.is_empty() {
                    let filter_lower = self.new_bookmark_group_name.to_lowercase();
                    let filtered_indices: Vec<usize> = self.bookmark_groups.iter()
                        .enumerate()
                        .filter(|(_, group)| group.name.to_lowercase().contains(&filter_lower))
                        .map(|(index, _)| index)
                        .collect();
                    filtered_indices.get(self.selected_bookmark_group_index).copied()
                } else {
                    Some(self.selected_bookmark_group_index)
                };
                
                if let Some(group_index) = selected_group_index {
                    if let Some(group) = self.bookmark_groups.get(group_index) {
                        // Check if we're in "access mode" (backtick) or "add mode" (m)
                        if self.bookmark_access_mode {
                            // Access mode - show clips in this group (in same dialog)
                            self.selected_bookmark_group_for_clips = Some(group.name.clone());
                            self.selected_bookmark_clip_index = 0;
                            self.bookmark_clip_gg_pressed = false;
                            self.just_switched_to_clips = false;  // Allow immediate Enter handling
                        } else {
                            // Add mode - add current clipboard item to this group
                            if let Some(group_mut) = self.bookmark_groups.get_mut(group_index) {
                                // Get the currently selected clipboard item
                                let selected_clipboard_item = {
                                    let items = self.clipboard_items.lock().unwrap();
                                    
                                    // Find the actual item to select (works for both filtered and unfiltered)
                                    if self.filter_active {
                                        let filter_lower = self.filter_text.to_lowercase();
                                        let filtered_items: Vec<&ClipboardItem> = items.iter()
                                            .filter(|item| {
                                                let content_lower = item.content.to_lowercase();
                                                content_lower.contains(&filter_lower)
                                            })
                                            .collect();
                                        
                                        filtered_items.get(self.selected_clipboard_index).map(|&item| item.clone())
                                    } else {
                                        items.get(self.selected_clipboard_index).map(|item| item.clone())
                                    }
                                };
                                
                                if let Some(clipboard_item) = selected_clipboard_item {
                                    // Check if item already exists in the group
                                    if !group_mut.clips.iter().any(|clip| clip.id == clipboard_item.id) {
                                        group_mut.clips.push(clipboard_item);
                                        
                                        // Save to file
                                        if let Err(e) = save_bookmark_groups(&self.bookmark_groups, &self.config) {
                                            eprintln!("Failed to save bookmark groups: {}", e);
                                        }
                                    }
                                }
                            }
                            
                            // Close the dialog but not the window (only for add mode)
                            self.show_bookmark_groups = false;
                            self.selected_bookmark_group_index = 0;
                            self.bookmark_gg_pressed = false;
                        }
                    }
                }
                bookmark_enter_handled = true;
            }
            
            // Handle Enter key (works in both normal and filter mode, but not in bookmark groups dialog)
            if i.key_pressed(egui::Key::Enter) && !i.modifiers.ctrl && !i.modifiers.alt && !i.modifiers.shift && !self.show_bookmark_groups && !bookmark_enter_handled && !bookmark_clip_enter_handled && self.selected_bookmark_group_for_clips.is_none() && !self.show_help {
                if let Ok(mut clipboard) = Clipboard::new() {
                    let selected_item_id = {
                        let items = self.clipboard_items.lock().unwrap();
                        
                        // Find the actual item to select (works for both filtered and unfiltered)
                        if self.filter_active {
                            let filter_lower = self.filter_text.to_lowercase();
                            let filtered_items: Vec<&ClipboardItem> = items.iter()
                                .filter(|item| {
                                    let content_lower = item.content.to_lowercase();
                                    content_lower.contains(&filter_lower)
                                })
                                .collect();
                            
                            filtered_items.get(self.selected_clipboard_index).map(|item| item.id)
                        } else {
                            items.get(self.selected_clipboard_index).map(|item| item.id)
                        }
                    };
                    
                    if let Some(selected_item_id) = selected_item_id {
                        // Now get the items again for modification
                        let mut items_vec = self.clipboard_items.lock().unwrap();
                        
                        // Find the item in the full list and get its content
                        let item_content = if let Some(pos) = items_vec.iter().position(|item| item.id == selected_item_id) {
                            let content = items_vec[pos].content.clone();
                            
                            // Move selected item to top and update timestamp
                            let mut item = items_vec.remove(pos);
                            item.timestamp = Utc::now();
                            items_vec.insert(0, item);
                            
                            Some(content)
                        } else {
                            None
                        };
                        
                        // Set clipboard content
                        if let Some(content) = item_content {
                            let _ = clipboard.set_text(content);
                        }
                        
                        self.selected_clipboard_index = 0;
                        
                        // Save to file
                        if let Err(e) = save_clipboard_items(&items_vec, &self.config) {
                            eprintln!("Failed to save clipboard items: {}", e);
                        }
                        
                        // Clear filter when closing window after copying
                        self.filter_active = false;
                        self.is_filtering = false;
                        self.filter_text.clear();
                        
                        // Close window
                        let mut vis = self.visible.lock().unwrap();
                        *vis = false;
                    }
                }
            }
        });
        
        // Check if we should quit
        if self.should_quit {
            ctx.send_viewport_cmd(egui::ViewportCommand::Close);
            return;
        }
        
        let is_visible = *self.visible.lock().unwrap();
        
        if !is_visible {
            ctx.send_viewport_cmd(egui::ViewportCommand::Visible(false));
            // Request slower repaint when window is hidden to save CPU
            ctx.request_repaint_after(std::time::Duration::from_millis(500));
            return;
        }
        
        ctx.send_viewport_cmd(egui::ViewportCommand::Visible(true));
        
        if self.show_bookmark_groups {
            // Show bookmark groups as a centered dialog
            egui::CentralPanel::default()
                .frame(
                    egui::Frame::none()
                        .fill(hex_to_color(&self.theme.background_color))
                        .stroke(egui::Stroke::new(1.0, hex_to_color(&self.theme.border_color)))
                        .inner_margin(8.0)
                )
                .show(ctx, |ui| {
                    ui.style_mut().visuals.override_text_color = Some(hex_to_color(&self.theme.text_color));
                    ui.style_mut().visuals.panel_fill = hex_to_color(&self.theme.background_color);
                    ui.style_mut().visuals.window_fill = hex_to_color(&self.theme.background_color);
                    
                    // Create a centered dialog
                    let dialog_width = 400.0;
                    let dialog_height = 500.0;
                    
                    egui::Area::new(egui::Id::new("bookmark_groups_dialog"))
                        .anchor(egui::Align2::CENTER_CENTER, [0.0, 0.0])
                        .show(ctx, |ui| {
                            ui.set_width(dialog_width);
                            ui.set_min_height(dialog_height);
                            
                            let frame = egui::Frame::none()
                                .fill(hex_to_color(&self.theme.background_color))
                                .stroke(egui::Stroke::new(2.0, hex_to_color(&self.theme.selected_border_color)))
                                .inner_margin(16.0)
                                .rounding(8.0);
                            
                            frame.show(ui, |ui| {
                                ui.style_mut().visuals.override_text_color = Some(hex_to_color(&self.theme.text_color));
                                ui.style_mut().visuals.panel_fill = hex_to_color(&self.theme.background_color);
                                ui.style_mut().visuals.window_fill = hex_to_color(&self.theme.background_color);
                                
                                // Dialog title
                                if let Some(group_name) = &self.selected_bookmark_group_for_clips {
                                    ui.heading(egui::RichText::new(format!("Clips in '{}'", group_name))
                                        .color(hex_to_color(&self.theme.text_color))
                                        .family(egui::FontFamily::Monospace));
                                } else {
                                    ui.heading(egui::RichText::new("Bookmark Groups")
                                        .color(hex_to_color(&self.theme.text_color))
                                        .family(egui::FontFamily::Monospace));
                                }
                                ui.add_space(16.0);
                                
                                // Text input for new bookmark group name (only when creating groups)
                                if self.show_bookmark_input {
                                    ui.horizontal(|ui| {
                                        ui.label(egui::RichText::new("New group:")
                                            .color(hex_to_color(&self.theme.text_color))
                                            .family(egui::FontFamily::Monospace));
                                        
                                        let response = ui.add(
                                            egui::TextEdit::singleline(&mut self.new_bookmark_group_name)
                                                .desired_width(ui.available_width() - 80.0)
                                                .text_color(hex_to_color(&self.theme.text_color))
                                                .font(egui::FontId::monospace(14.0))
                                                .hint_text(egui::RichText::new("Enter group name...")
                                                    .color(hex_to_color(&self.theme.text_color))
                                                    .family(egui::FontFamily::Monospace))
                                        );
                                        
                                        // Request focus for the text input
                                        response.request_focus();
                                        
                                        // Handle Enter key to create new group
                                        if ui.input(|i| i.key_pressed(egui::Key::Enter)) {
                                            if !self.new_bookmark_group_name.trim().is_empty() {
                                                let group_name = self.new_bookmark_group_name.trim().to_string();
                                                
                                                // Check if group already exists
                                                if !self.bookmark_groups.iter().any(|g| g.name == group_name) {
                                                    let new_group = BookmarkGroup {
                                                        name: group_name.clone(),
                                                        created_at: Utc::now(),
                                                        clips: Vec::new(),
                                                    };
                                                    self.bookmark_groups.push(new_group);
                                                    
                                                    // Save to file
                                                    if let Err(e) = save_bookmark_groups(&self.bookmark_groups, &self.config) {
                                                        eprintln!("Failed to save bookmark groups: {}", e);
                                                    }
                    }
                    
                }
            }
                                        ui.add_space(16.0);
                                    });
                                }
                                ui.separator();
                                ui.add_space(16.0);
                                
                                 // Display content based on whether we're viewing groups or clips
                                if let Some(group_name) = &self.selected_bookmark_group_for_clips {
                                    // Show clips in selected group
                                    if let Some(group) = self.bookmark_groups.iter().find(|g| g.name == *group_name) {
                                        if group.clips.is_empty() {
                                            ui.label(egui::RichText::new("No clips in this group...")
                                                .color(hex_to_color(&self.theme.text_color))
                                                .family(egui::FontFamily::Monospace));
                                        } else {
                                            egui::ScrollArea::vertical()
                                                .max_height(300.0)
                                                .show(ui, |ui| {
                                                    ui.set_width(ui.available_width());
                                                    for (clip_index, clip) in group.clips.iter().enumerate() {
                                                        let is_selected = clip_index == self.selected_bookmark_clip_index;
                                                        
                                                        // Scroll to selected item
                                                        if is_selected {
                                                            let rect = ui.available_rect_before_wrap();
                                                            ui.scroll_to_rect(rect, Some(egui::Align::Center));
                                                        }
                                                        
                                                        let frame = egui::Frame::none()
                                                            .fill(if is_selected {
                                                                hex_to_color(&self.theme.selected_background_color)
                                                            } else if clip_index % 2 == 0 { 
                                                                hex_to_color(&self.theme.background_color)
                                                            } else { 
                                                                hex_to_color(&self.theme.alternate_row_color)
                                                            })
                                                            .stroke(if is_selected {
                                                                egui::Stroke::new(2.0, hex_to_color(&self.theme.selected_border_color))
                                                            } else {
                                                                egui::Stroke::new(1.0, hex_to_color(&self.theme.border_color))
                                                            })
                                                            .inner_margin(8.0);
                                                        
                                                        frame.show(ui, |ui| {
                                                            ui.set_width(ui.available_width());
                                                            ui.add_space(5.0);
                                                            
                                                            // Timestamp and content preview
                                                            ui.horizontal(|ui| {
                                                                ui.label(egui::RichText::new(format!("{}", 
                                                                    clip.timestamp.format("%Y-%m-%d %H:%M:%S")))
                                                                    .color(if is_selected {
                                                                        hex_to_color(&self.theme.selected_text_color)
                                                                    } else {
                                                                        hex_to_color(&self.theme.text_color)
                                                                    })
                                                                    .family(egui::FontFamily::Monospace));
                                                            });
                                                            
                                                            ui.label(egui::RichText::new(clip.preview())
                                                                .color(if is_selected {
                                                                    hex_to_color(&self.theme.selected_text_color)
                                                                } else {
                                                                    hex_to_color(&self.theme.text_color)
                                                                })
                                                                .family(egui::FontFamily::Monospace));
                                                            
                                                            // Show line count for multiline content
                                                            if clip.content.lines().count() > 1 {
                                                                ui.label(egui::RichText::new(format!("({} lines)", clip.content.lines().count()))
                                                                    .color(if is_selected {
                                                                        hex_to_color(&self.theme.selected_text_color)
                                                                    } else {
                                                                        hex_to_color(&self.theme.text_color)
                                                                    })
                                                                    .family(egui::FontFamily::Monospace));
                                                            }
                                                            
                                                            ui.add_space(5.0);
                                                        });
                                                        
                                                        ui.add_space(4.0);
                                                    }
                                                });
                                        }
                                    }
                                } else {
                                    // Show bookmark groups
                                    let filtered_groups: Vec<&BookmarkGroup> = if self.show_bookmark_input && !self.new_bookmark_group_name.is_empty() {
                                        let filter_lower = self.new_bookmark_group_name.to_lowercase();
                                        self.bookmark_groups.iter()
                                            .filter(|group| group.name.to_lowercase().contains(&filter_lower))
                                            .collect()
                                    } else {
                                        self.bookmark_groups.iter().collect()
                                    };
                                    
                                    if filtered_groups.is_empty() && self.show_bookmark_input && !self.new_bookmark_group_name.is_empty() {
                                        ui.label(egui::RichText::new("No groups match filter...")
                                            .color(hex_to_color(&self.theme.text_color))
                                            .family(egui::FontFamily::Monospace));
                                    } else if filtered_groups.is_empty() {
                                        ui.label(egui::RichText::new("No bookmark groups yet...")
                                            .color(hex_to_color(&self.theme.text_color))
                                            .family(egui::FontFamily::Monospace));
                                    } else {
                                        egui::ScrollArea::vertical()
                                            .max_height(300.0)
                                            .show(ui, |ui| {
                                                ui.set_width(ui.available_width());
                                                for (filter_index, group) in filtered_groups.iter().enumerate() {
                                                    let is_selected = filter_index == self.selected_bookmark_group_index;
                                                    
                                                    // Scroll to selected item
                                                    if is_selected {
                                                        let rect = ui.available_rect_before_wrap();
                                                        ui.scroll_to_rect(rect, Some(egui::Align::Center));
                                                    }
                                                    
                                                    let frame = egui::Frame::none()
                                                        .fill(if is_selected {
                                                            hex_to_color(&self.theme.selected_background_color)
                                                        } else if group.name.len() % 2 == 0 { 
                                                            hex_to_color(&self.theme.background_color)
                                                        } else { 
                                                            hex_to_color(&self.theme.alternate_row_color)
                                                        })
                                                        .stroke(if is_selected {
                                                            egui::Stroke::new(2.0, hex_to_color(&self.theme.selected_border_color))
                                                        } else {
                                                            egui::Stroke::new(1.0, hex_to_color(&self.theme.border_color))
                                                        })
                                                        .inner_margin(8.0);
                                                    
                                                    frame.show(ui, |ui| {
                                                        ui.set_width(ui.available_width());
                                                        
                                                        ui.horizontal(|ui| {
                                                            ui.label(egui::RichText::new(&group.name)
                                                                .color(if is_selected {
                                                                    hex_to_color(&self.theme.selected_text_color)
                                                                } else {
                                                                    hex_to_color(&self.theme.text_color)
                                                                })
                                                                .family(egui::FontFamily::Monospace));
                                                            
                                                            ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                                                                ui.label(egui::RichText::new(format!("{} clips", group.clips.len()))
                                                                    .color(if is_selected {
                                                                        hex_to_color(&self.theme.selected_text_color)
                                                                    } else {
                                                                        hex_to_color(&self.theme.text_color)
                                                                    })
                                                                    .family(egui::FontFamily::Monospace));
                                                                ui.add_space(10.0);
                                                                ui.label(egui::RichText::new(format!("{}", 
                                                                    group.created_at.format("%Y-%m-%d %H:%M")))
                                                                    .color(if is_selected {
                                                                        hex_to_color(&self.theme.selected_text_color)
                                                                    } else {
                                                                        hex_to_color(&self.theme.text_color)
                                                                    })
                                                                    .family(egui::FontFamily::Monospace));
                                                                });
                                                        });
                                                    });
                                                    
                                                    ui.add_space(4.0);
                                                }
                                            });
                                    }
                                }
                                
                                ui.add_space(16.0);
                                ui.separator();
                                ui.add_space(8.0);
                                
                                // Instructions
                                let instruction = if self.show_bookmark_input {
                                    "Press Escape to close • Enter to create group"
                                } else if self.selected_bookmark_group_for_clips.is_some() {
                                    "Press Escape to go back • j/k to navigate • d to remove clip • Enter to copy clip"
                                } else {
                                    "Press Escape to close • j/k to navigate • Enter to view clips"
                                };
                                ui.label(egui::RichText::new(instruction)
                                    .color(hex_to_color(&self.theme.text_color))
                                    .family(egui::FontFamily::Monospace)
                                    .size(12.0));
                            });
                        });
                });
        } else {
            // Normal clipboard view
            egui::CentralPanel::default()
                .frame(
                    egui::Frame::none()
                        .fill(hex_to_color(&self.theme.background_color))
                        .stroke(egui::Stroke::new(1.0, hex_to_color(&self.theme.border_color)))
                        .inner_margin(8.0)
                )
                .show(ctx, |ui| {
                    ui.style_mut().visuals.override_text_color = Some(hex_to_color(&self.theme.text_color));
                    ui.style_mut().visuals.panel_fill = hex_to_color(&self.theme.background_color);
                    ui.style_mut().visuals.window_fill = hex_to_color(&self.theme.background_color);
                    
                    // Filter input UI (only show when editing)
                    if self.show_filter_input {
                        let filter_prefix = match self.filter_mode {
                            FilterMode::Generous => "/",
                            FilterMode::None => "",
                        };
                        
                        ui.horizontal(|ui| {
                            ui.label(egui::RichText::new(filter_prefix)
                                .color(hex_to_color(&self.theme.selected_text_color))
                                .family(egui::FontFamily::Monospace));
                            
                            // Editable filter input
                            ui.add(
                                egui::TextEdit::singleline(&mut self.filter_text)
                                    .desired_width(ui.available_width() - 20.0)
                                    .text_color(hex_to_color(&self.theme.text_color))
                                    .font(egui::FontId::monospace(14.0))
                            );
                        });
                        ui.add_space(10.0);
                    } else if self.filter_active {
                        // Show active filter indicator (read-only)
                        ui.horizontal(|ui| {
                            ui.label(egui::RichText::new("/")
                                .color(hex_to_color(&self.theme.selected_text_color))
                                .family(egui::FontFamily::Monospace));
                            ui.label(egui::RichText::new(&self.filter_text)
                                .color(hex_to_color(&self.theme.text_color))
                                .family(egui::FontFamily::Monospace));
                        });
                        ui.add_space(10.0);
                    }
                    
                    ui.separator();
                    ui.add_space(10.0);
                    
                    // Display clipboard items
                    let items = self.clipboard_items.lock().unwrap();
                    if items.is_empty() {
                        ui.label(egui::RichText::new("No clipboard items yet...").color(hex_to_color(&self.theme.text_color)).family(egui::FontFamily::Monospace));
                    } else {
                        // Filter items if filter is active or being edited
                        let filtered_items: Vec<(usize, &ClipboardItem)> = if self.filter_active || self.filter_mode != FilterMode::None {
                            let filter_lower = self.filter_text.to_lowercase();
                            items.iter().enumerate().filter(|(_, item)| {
                                let content_lower = item.content.to_lowercase();
                                match self.filter_mode {
                                    FilterMode::Generous => content_lower.contains(&filter_lower),
                                    FilterMode::None => true,
                                }
                            }).collect()
                        } else {
                            items.iter().enumerate().collect()
                        };
                        
                        if filtered_items.is_empty() && (self.filter_active || self.filter_mode != FilterMode::None) {
                            ui.label(egui::RichText::new("No items match filter").color(hex_to_color(&self.theme.text_color)).family(egui::FontFamily::Monospace));
                        } else {
                            if self.config.verbose {
                                ui.heading(egui::RichText::new("Recent Clipboard Items:").color(hex_to_color(&self.theme.text_color)).family(egui::FontFamily::Monospace));
                            }
                            ui.add_space(10.0);
                            
                            egui::ScrollArea::vertical()
                                .max_height(ui.available_height())
                                .show(ui, |ui| {
                                    ui.set_width(ui.available_width());
                                    for (filter_index, (original_index, item)) in filtered_items.iter().enumerate() {
                                        let is_selected = filter_index == self.selected_clipboard_index;
                                        
                                        // Scroll to selected item
                                        if is_selected {
                                            let rect = ui.available_rect_before_wrap();
                                            ui.scroll_to_rect(rect, Some(egui::Align::Center));
                                        }
                                    let is_selected = filter_index == self.selected_clipboard_index;
                                    let frame = egui::Frame::none()
                                        .fill(if is_selected {
                                            hex_to_color(&self.theme.selected_background_color)
                                        } else if original_index % 2 == 0 { 
                                            hex_to_color(&self.theme.background_color)
                                        } else { 
                                            hex_to_color(&self.theme.alternate_row_color)
                                        })
                                        .stroke(if is_selected {
                                            egui::Stroke::new(2.0, hex_to_color(&self.theme.selected_border_color))
                                        } else {
                                            egui::Stroke::new(1.0, hex_to_color(&self.theme.border_color))
                                        });
                                    
                                    frame.show(ui, |ui| {
                                        ui.set_width(ui.available_width());
                                        ui.add_space(5.0);
                                        
                                        if self.config.verbose {
                                            // Item number and timestamp
                                            ui.horizontal(|ui| {
                                                ui.label(egui::RichText::new(format!("#{}", items.len() - original_index))
                                                    .color(hex_to_color(&self.theme.text_color))
                                                    .family(egui::FontFamily::Monospace));
                                                
                                                ui.label(egui::RichText::new(format!("{}", 
                                                    item.timestamp.format("%H:%M:%S")))
                                                    .color(hex_to_color(&self.theme.text_color))
                                                    .family(egui::FontFamily::Monospace));
                                            });
                                        }
                                        
                                        // Content preview
                                        ui.label(egui::RichText::new(item.preview())
                                            .color(if is_selected {
                                                hex_to_color(&self.theme.selected_text_color)
                                            } else {
                                                hex_to_color(&self.theme.text_color)
                                            })
                                            .family(egui::FontFamily::Monospace));
                                        
                                        // Show line count for multiline content
                                        if item.content.lines().count() > 1 {
                                            ui.label(egui::RichText::new(format!("({} lines)", item.content.lines().count()))
                                                .color(hex_to_color(&self.theme.text_color))
                                                .family(egui::FontFamily::Monospace));
                                        }
                                        
                                        ui.add_space(5.0);
                                    });
                                    
                                    ui.add_space(2.0);
                                }
                            });
                        }
                    }
                });
        }
        
        // Help dialog
        if self.show_help {
            egui::Window::new("Help - Key Bindings")
                .collapsible(false)
                .resizable(true)
                .default_width(600.0)
                .anchor(egui::Align2::CENTER_CENTER, [0.0, 0.0])
                .show(ctx, |ui| {
                    ui.style_mut().visuals.override_text_color = Some(hex_to_color(&self.theme.text_color));
                    ui.style_mut().visuals.panel_fill = hex_to_color(&self.theme.background_color);
                    
                    ui.horizontal(|ui| {
                        ui.label(egui::RichText::new("Press Escape to close")
                            .color(hex_to_color(&self.theme.text_color))
                            .family(egui::FontFamily::Monospace));
                    });
                    ui.add_space(10.0);
                    
                    ui.separator();
                    ui.add_space(10.0);
                    
                    // Define all keybindings
                    let keybindings = vec![
                        // Help dialog navigation
                        KeyBinding::new("j", "Scroll down in help dialog", Context::Help),
                        KeyBinding::new("k", "Scroll up in help dialog", Context::Help),
                        KeyBinding::new("Escape", "Close help dialog", Context::Help),
                        
                        // Always available
                        KeyBinding::new("Escape", "Close window/dialog", Context::Always),
                        KeyBinding::new("Ctrl+Q", "Quit application", Context::Always),
                        KeyBinding::new("Ctrl+S", "Toggle settings dialog", Context::Always),
                        KeyBinding::new("?", "Show this help dialog", Context::Always),
                        
                        // Clips list navigation
                        KeyBinding::new("j", "Move down in list", Context::ClipsList),
                        KeyBinding::new("k", "Move up in list", Context::ClipsList),
                        KeyBinding::new("gg", "Go to first item", Context::ClipsList),
                        KeyBinding::new("G", "Go to last item", Context::ClipsList),
                        KeyBinding::new("Enter", "Copy selected item to clipboard", Context::ClipsList),
                        KeyBinding::new("Shift+D", "Delete selected item", Context::ClipsList),
                        
                        // Filtering
                        KeyBinding::new("/", "Start filtering clips", Context::ClipsList),
                        KeyBinding::new("Enter", "Apply filter", Context::Filtering),
                        KeyBinding::new("Escape", "Clear filter", Context::Filtering),
                        KeyBinding::new("↑/↓", "Navigate filtered results", Context::Filtering),
                        KeyBinding::new("Backspace", "Delete filter character", Context::Filtering),
                        
                        // Bookmark groups
                        KeyBinding::new("m", "Show bookmark groups (add mode)", Context::ClipsList),
                        KeyBinding::new("Shift+M", "Show bookmark groups with input", Context::ClipsList),
                        KeyBinding::new("`", "Show bookmark groups (access mode)", Context::ClipsList),
                        KeyBinding::new("j/k", "Navigate groups", Context::BookmarkGroups),
                        KeyBinding::new("Enter", "View clips in group / Add to group", Context::BookmarkGroups),
                        KeyBinding::new("Shift+D", "Delete bookmark group", Context::BookmarkGroups),
                        
                        // Bookmark clips
                        KeyBinding::new("j/k", "Navigate clips in group", Context::BookmarkClips),
                        KeyBinding::new("Enter", "Copy clip to clipboard", Context::BookmarkClips),
                        KeyBinding::new("d", "Remove clip from group", Context::BookmarkClips),
                        KeyBinding::new("Escape", "Go back to groups", Context::BookmarkClips),
                    ];
                    
                    egui::ScrollArea::vertical()
                        .max_height(500.0)
                        .vertical_scroll_offset(self.help_scroll_offset)
                        .show(ui, |ui| {
                            ui.set_width(ui.available_width() - 20.0); // Make scrollable area 20px narrower
                            // Group by context
                            let mut last_context: Option<Context> = None;
                            
                            for keybinding in &keybindings {
                                // Show context header if changed
                                if last_context.as_ref() != Some(&keybinding.context) {
                                    if last_context.is_some() {
                                        ui.add_space(10.0);
                                    }
                                    
                                    let context_name = match keybinding.context {
                                        Context::Help => "Help Dialog",
                                        Context::Always => "Always Available",
                                        Context::ClipsList => "Clips List",
                                        Context::Filtering => "Filtering Mode",
                                        Context::BookmarkGroups => "Bookmark Groups",
                                        Context::BookmarkClips => "Bookmark Clips",
                                    };
                                    
                                    ui.heading(egui::RichText::new(context_name)
                                        .color(hex_to_color(&self.theme.selected_border_color))
                                        .family(egui::FontFamily::Monospace)
                                        .size(16.0));
                                    ui.add_space(5.0);
                                    
                                    last_context = Some(keybinding.context.clone());
                                }
                                
                                // Keybinding row
                                ui.horizontal(|ui| {
                                    // Key column
                                    ui.add_sized(
                                        [120.0, 0.0],
                                        egui::Label::new(
                                            egui::RichText::new(&keybinding.key)
                                                .color(hex_to_color(&self.theme.selected_text_color))
                                                .family(egui::FontFamily::Monospace)
                                                .strong()
                                        )
                                    );
                                    
                                    // Description column
                                    ui.label(egui::RichText::new(&keybinding.description)
                                        .color(hex_to_color(&self.theme.text_color))
                                        .family(egui::FontFamily::Monospace));
                                });
                                
                                ui.add_space(2.0);
                            }
                        });
                    
                    ui.add_space(10.0);
                    ui.separator();
                    ui.add_space(5.0);
                    
                    // Instructions
                    ui.label(egui::RichText::new("Scroll to see all available keybindings")
                        .color(hex_to_color(&self.theme.text_color))
                        .family(egui::FontFamily::Monospace)
                        .size(12.0));
                });
        }
        
        // Settings dialog
        if self.show_settings {
            egui::Window::new("Settings")
                .collapsible(false)
                .resizable(false)
                .anchor(egui::Align2::CENTER_CENTER, [0.0, 0.0])
                .show(ctx, |ui| {
                    ui.style_mut().visuals.override_text_color = Some(hex_to_color(&self.theme.text_color));
                    ui.style_mut().visuals.panel_fill = hex_to_color(&self.theme.background_color);
                    
                    ui.heading("Application Settings");
                    ui.add_space(10.0);
                    
                    // Autostart setting
                    let mut autostart_enabled = self.config.autostart_enabled;
                    if ui.checkbox(&mut autostart_enabled, "Launch at startup").changed() {
                        self.config.autostart_enabled = autostart_enabled;
                        
                        if autostart_enabled {
                            if let Err(e) = enable_autostart() {
                                eprintln!("Failed to enable autostart: {}", e);
                            }
                        } else {
                            if let Err(e) = disable_autostart() {
                                eprintln!("Failed to disable autostart: {}", e);
                            }
                        }
                        
                        // Save config
                        if let Err(e) = save_config(&self.config) {
                            eprintln!("Failed to save config: {}", e);
                        }
                    }
                    
                    ui.add_space(10.0);
                    ui.separator();
                    ui.add_space(10.0);
                    
                    // Current status
                    ui.label("Current Status:");
                    ui.horizontal(|ui| {
                        ui.label("• Autostart:");
                        if let Ok(enabled) = is_autostart_enabled() {
                            ui.label(if enabled { "Enabled" } else { "Disabled" });
                        } else {
                            ui.label("Unknown");
                        }
                    });
                    
                    ui.add_space(10.0);
                    ui.separator();
                    ui.add_space(10.0);
                    
                    // Instructions
                    ui.label("Press Ctrl+S to toggle settings dialog");
                    
                    ui.add_space(10.0);
                    
                    if ui.button("Close").clicked() {
                        self.show_settings = false;
                    }
                });
        }
    }
}
