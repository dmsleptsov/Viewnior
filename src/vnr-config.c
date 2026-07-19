#include "vnr-config.h"

#define VNR_PREF_LOAD_KEY(PK, PT, KN, SECTION)  \
    do { \
        config. PK = g_key_file_get_ ## PT (conf, SECTION, KN, &read_error); \
        if(read_error != nullptr) { \
            g_warning("Error read property '%s': '%s'", KN, g_error_get_msg(read_error)); \
            g_clear_error(&read_error); \
        } else{ \
            _config-> PK = config. PK; \
            g_info("Success read property '%s'", KN); \
        } \
    } while(0)
#define PREFS_GROUP_NAME "prefs"
#define LAST_VALUES_GROUP_NAME "last-values"

#define CONFIG_FILENAME "viewnior.conf"
#define CONFIG_DIR g_get_user_config_dir(), PACKAGE, nullptr
#define CONFIG_FILE g_get_user_config_dir(), PACKAGE, CONFIG_FILENAME, nullptr

static VnrConfig *_config = nullptr;
static GMutex _write_lock;

static void vnr_config_init_default() {
    _config->zoom = VNR_PREFS_ZOOM_SMART;
    _config->show_hidden = true;
    _config->dark_background = true;
    _config->smooth_images = true;
    _config->confirm_delete = true;
    _config->behavior_wheel = VNR_PREFS_WHEEL_ZOOM;
    _config->behavior_click = VNR_PREFS_CLICK_ZOOM;
    _config->behavior_modify = VNR_PREFS_MODIFY_ASK;
    _config->jpeg_quality = 90;
    _config->png_compression = 9;
    _config->reload_on_save = false;
    _config->show_menu_bar = true;
    _config->show_toolbar = true;
    _config->show_scrollbar = true;
    _config->show_statusbar = true;
    _config->auto_resize = false;
    _config->desktop = VNR_PREFS_DESKTOP_AUTO;
    _config->use_existing_process = true;
    _config->last_size.a = -1;
    _config->last_size.b = -1;
    _config->last_position.a = -1;
    _config->last_position.b = -1;
}

static gboolean vnr_config_load() {
    vnr_config_init_default();

    GKeyFile *conf = g_key_file_new();
    gchar *file = g_build_filename(CONFIG_FILE);

    GError *load_file_error = nullptr;
    g_key_file_load_from_file(conf, file, G_KEY_FILE_NONE, &load_file_error);
    if (load_file_error != nullptr) {
        g_critical("Error during load config file '%s'", file);
        g_key_file_free(conf);
        g_free(file);
        return false;
    }

    VnrConfig config;
    GError *read_error = nullptr;
    VNR_PREF_LOAD_KEY(zoom, integer, "zoom-mode", PREFS_GROUP_NAME);
    VNR_PREF_LOAD_KEY(show_hidden, boolean, "show-hidden", PREFS_GROUP_NAME);
    VNR_PREF_LOAD_KEY(dark_background, boolean, "dark-background", PREFS_GROUP_NAME);
    VNR_PREF_LOAD_KEY(smooth_images, boolean, "smooth-images", PREFS_GROUP_NAME);
    VNR_PREF_LOAD_KEY(confirm_delete, boolean, "confirm-delete", PREFS_GROUP_NAME);
    VNR_PREF_LOAD_KEY(reload_on_save, boolean, "reload-on-save", PREFS_GROUP_NAME);
    VNR_PREF_LOAD_KEY(show_menu_bar, boolean, "show-menu-bar", PREFS_GROUP_NAME);
    VNR_PREF_LOAD_KEY(show_toolbar, boolean, "show-toolbar", PREFS_GROUP_NAME);
    VNR_PREF_LOAD_KEY(show_scrollbar, boolean, "show-scrollbar", PREFS_GROUP_NAME);
    VNR_PREF_LOAD_KEY(show_statusbar, boolean, "show-statusbar", PREFS_GROUP_NAME);
    VNR_PREF_LOAD_KEY(auto_resize, boolean, "auto-resize", PREFS_GROUP_NAME);
    VNR_PREF_LOAD_KEY(behavior_wheel, integer, "behavior-wheel", PREFS_GROUP_NAME);
    VNR_PREF_LOAD_KEY(behavior_click, integer, "behavior-click", PREFS_GROUP_NAME);
    VNR_PREF_LOAD_KEY(behavior_modify, integer, "behavior-modify", PREFS_GROUP_NAME);
    VNR_PREF_LOAD_KEY(jpeg_quality, integer, "jpeg-quality", PREFS_GROUP_NAME);
    VNR_PREF_LOAD_KEY(png_compression, integer, "png-compression", PREFS_GROUP_NAME);
    VNR_PREF_LOAD_KEY(desktop, integer, "desktop", PREFS_GROUP_NAME);
    VNR_PREF_LOAD_KEY(use_existing_process, boolean, "use-existing-process", PREFS_GROUP_NAME);

    VNR_PREF_LOAD_KEY(last_size.a, integer, "size.w", LAST_VALUES_GROUP_NAME);
    VNR_PREF_LOAD_KEY(last_size.b, integer, "size.h", LAST_VALUES_GROUP_NAME);
    VNR_PREF_LOAD_KEY(last_position.a, integer, "position.x", LAST_VALUES_GROUP_NAME);
    VNR_PREF_LOAD_KEY(last_position.b, integer, "position.y", LAST_VALUES_GROUP_NAME);

    g_info("Successfully load config file('%s')", file);

    g_key_file_free(conf);
    g_free(file);

    return true;
}

const VnrConfig *vnr_config_get() {
    if (_config != nullptr) {
        return _config;
    }

    g_mutex_lock(&_write_lock);
    if (_config != nullptr) {
        g_mutex_unlock(&_write_lock);
        return _config;
    }
    _config = g_malloc(sizeof(VnrConfig));
    vnr_config_load();
    g_mutex_unlock(&_write_lock);

    g_info("Successfully initialize global configuration");
    return _config;
}

void vnr_config_set(const VnrConfigUpdate update) {
    gboolean has_update = false;
    if (update.zoom != nullptr && _config->zoom != *update.zoom) {
        _config->zoom = *update.zoom;
        has_update |= true;
    }
    if (update.desktop != nullptr && _config->desktop != *update.desktop) {
        _config->desktop = *update.desktop;
        has_update |= true;
    }
    if (update.behavior_wheel != nullptr && _config->behavior_wheel != *update.behavior_wheel) {
        _config->behavior_wheel = *update.behavior_wheel;
        has_update |= true;
    }
    if (update.behavior_click != nullptr && _config->behavior_click != *update.behavior_click) {
        _config->behavior_click = *update.behavior_click;
        has_update |= true;
    }
    if (update.behavior_modify != nullptr && _config->behavior_modify != *update.behavior_modify) {
        _config->behavior_modify = *update.behavior_modify;
        has_update |= true;
    }
    if (update.show_hidden != nullptr && _config->show_hidden != *update.show_hidden) {
        _config->show_hidden = *update.show_hidden;
        has_update |= true;
    }
    if (update.smooth_images != nullptr && _config->smooth_images != *update.smooth_images) {
        _config->smooth_images = *update.smooth_images;
        has_update |= true;
    }
    if (update.confirm_delete != nullptr && _config->confirm_delete != *update.confirm_delete) {
        _config->confirm_delete = *update.confirm_delete;
        has_update |= true;
    }
    if (update.reload_on_save != nullptr && _config->reload_on_save != *update.reload_on_save) {
        _config->reload_on_save = *update.reload_on_save;
        has_update |= true;
    }
    if (update.show_menu_bar != nullptr && _config->show_menu_bar != *update.show_menu_bar) {
        _config->show_menu_bar = *update.show_menu_bar;
        has_update |= true;
    }
    if (update.show_toolbar != nullptr && _config->show_toolbar != *update.show_toolbar) {
        _config->show_toolbar = *update.show_toolbar;
        has_update |= true;
    }
    if (update.show_scrollbar != nullptr && _config->show_scrollbar != *update.show_scrollbar) {
        _config->show_scrollbar = *update.show_scrollbar;
        has_update |= true;
    }
    if (update.show_statusbar != nullptr && _config->show_statusbar != *update.show_statusbar) {
        _config->show_statusbar = *update.show_statusbar;
        has_update |= true;
    }
    if (update.auto_resize != nullptr && _config->auto_resize != *update.auto_resize) {
        _config->auto_resize = *update.auto_resize;
        has_update |= true;
    }
    if (update.dark_background != nullptr && _config->dark_background != *update.dark_background) {
        _config->dark_background = *update.dark_background;
        has_update |= true;
    }
    if (update.jpeg_quality != nullptr && _config->jpeg_quality != *update.jpeg_quality) {
        _config->jpeg_quality = *update.jpeg_quality;
        has_update |= true;
    }
    if (update.jpeg_quality != nullptr && _config->jpeg_quality != *update.jpeg_quality) {
        _config->jpeg_quality = *update.jpeg_quality;
        has_update |= true;
    }
    if (update.png_compression != nullptr && _config->png_compression != *update.png_compression) {
        _config->png_compression = *update.png_compression;
        has_update |= true;
    }
    if (update.use_existing_process != nullptr && _config->use_existing_process != *update.use_existing_process) {
        _config->use_existing_process = *update.use_existing_process;
        has_update |= true;
    }

    if (has_update) {
        vnr_config_save();
        vnr_dbus_send_config_update();
    }
}

void vnr_config_free() {
    g_mutex_lock(&_write_lock);
    g_free(_config);
    _config = nullptr;
    g_mutex_unlock(&_write_lock);
    g_info("Successfully free global configuration");
}

gboolean vnr_config_save() {
    vnr_config_get();

    g_mutex_lock(&_write_lock);

    GKeyFile *conf = g_key_file_new();
    g_key_file_set_integer(conf, PREFS_GROUP_NAME, "zoom-mode", _config->zoom);
    g_key_file_set_boolean(conf, PREFS_GROUP_NAME, "show-hidden", _config->show_hidden);
    g_key_file_set_boolean(conf, PREFS_GROUP_NAME, "dark-background", _config->dark_background);
    g_key_file_set_boolean(conf, PREFS_GROUP_NAME, "smooth-images", _config->smooth_images);
    g_key_file_set_boolean(conf, PREFS_GROUP_NAME, "confirm-delete", _config->confirm_delete);
    g_key_file_set_boolean(conf, PREFS_GROUP_NAME, "reload-on-save", _config->reload_on_save);
    g_key_file_set_boolean(conf, PREFS_GROUP_NAME, "show-menu-bar", _config->show_menu_bar);
    g_key_file_set_boolean(conf, PREFS_GROUP_NAME, "show-toolbar", _config->show_toolbar);
    g_key_file_set_boolean(conf, PREFS_GROUP_NAME, "show-scrollbar", _config->show_scrollbar);
    g_key_file_set_boolean(conf, PREFS_GROUP_NAME, "show-statusbar", _config->show_statusbar);
    g_key_file_set_boolean(conf, PREFS_GROUP_NAME, "auto-resize", _config->auto_resize);
    g_key_file_set_integer(conf, PREFS_GROUP_NAME, "behavior-wheel", _config->behavior_wheel);
    g_key_file_set_integer(conf, PREFS_GROUP_NAME, "behavior-click", _config->behavior_click);
    g_key_file_set_integer(conf, PREFS_GROUP_NAME, "behavior-modify", _config->behavior_modify);
    g_key_file_set_integer(conf, PREFS_GROUP_NAME, "jpeg-quality", _config->jpeg_quality);
    g_key_file_set_integer(conf, PREFS_GROUP_NAME, "png-compression", _config->png_compression);
    g_key_file_set_integer(conf, PREFS_GROUP_NAME, "desktop", _config->desktop);
    g_key_file_set_boolean(conf, PREFS_GROUP_NAME, "use-existing-process", _config->use_existing_process);

    GIntPair size = vnr_window_get_size();
    GIntPair position = vnr_window_get_position();
    g_key_file_set_integer(conf, LAST_VALUES_GROUP_NAME, "size.w", size.a);
    g_key_file_set_integer(conf, LAST_VALUES_GROUP_NAME, "size.h", size.b);
    g_key_file_set_integer(conf, LAST_VALUES_GROUP_NAME, "position.x", position.a);
    g_key_file_set_integer(conf, LAST_VALUES_GROUP_NAME, "position.y", position.b);

    gchar *pathname = g_build_filename(CONFIG_DIR);
    if (g_mkdir_with_parents(pathname, 0700) != 0) {
        g_warning("Error creating config file's parent directory (%s)\n", pathname);
    }
    g_free(pathname);

    GError *error = nullptr;
    gchar *filename = g_build_filename(CONFIG_FILE);
    const gboolean result = g_key_file_save_to_file(conf, filename, &error);
    if (!result) {
        g_warning("Error during saving config file('%s'): '%s'", filename, g_error_get_msg(error));
        g_error_free(error);
    } else {
        g_info("Successfully save config file('%s')", filename);
    }
    g_key_file_free(conf);
    g_free(filename);
    g_mutex_unlock(&_write_lock);

    return result;
}

void vnr_config_reload() {
    vnr_config_free();
    vnr_config_get();
}
