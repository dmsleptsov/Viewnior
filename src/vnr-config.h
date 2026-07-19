#ifndef VN_CONFIG_H
#define VN_CONFIG_H

#include <glib.h>
#include <gtk/gtk.h>
#include <gio/gio.h>
#include "vnr-tools.h"
#include "vnr-window.h"
#include "config.h"

G_BEGIN_DECLS

typedef enum {
    VNR_PREFS_ZOOM_SMART,
    VNR_PREFS_ZOOM_NORMAL,
    VNR_PREFS_ZOOM_FIT,
    VNR_PREFS_ZOOM_LAST_USED,
} VnrPrefsZoom;

typedef enum {
    VNR_PREFS_DESKTOP_GNOME2,
    VNR_PREFS_DESKTOP_GNOME3,
    VNR_PREFS_DESKTOP_XFCE,
    VNR_PREFS_DESKTOP_LXDE,
    VNR_PREFS_DESKTOP_PUPPY,
    VNR_PREFS_DESKTOP_FLUXBOX,
    VNR_PREFS_DESKTOP_NITROGEN,
    VNR_PREFS_DESKTOP_MATE,
    VNR_PREFS_DESKTOP_CINNAMON,
    VNR_PREFS_DESKTOP_AUTO,
} VnrPrefsDesktop;

typedef enum {
    VNR_PREFS_WHEEL_NAVIGATE,
    VNR_PREFS_WHEEL_ZOOM,
    VNR_PREFS_WHEEL_SCROLL,
} VnrPrefsWheel;

typedef enum {
    VNR_PREFS_CLICK_ZOOM,
    VNR_PREFS_CLICK_NEXT,
} VnrPrefsClick;

typedef enum {
    VNR_PREFS_MODIFY_ASK,
    VNR_PREFS_MODIFY_SAVE,
    VNR_PREFS_MODIFY_IGNORE,
} VnrPrefsModify;

typedef struct {
    VnrPrefsZoom zoom;
    VnrPrefsDesktop desktop;
    VnrPrefsWheel behavior_wheel;
    VnrPrefsClick behavior_click;
    VnrPrefsModify behavior_modify;
    gboolean show_hidden;
    gboolean smooth_images;
    gboolean confirm_delete;
    gboolean reload_on_save;
    gboolean show_menu_bar;
    gboolean show_toolbar;
    gboolean show_scrollbar;
    gboolean show_statusbar;
    gboolean auto_resize;
    gboolean dark_background;
    gint jpeg_quality;
    gint png_compression;
    gboolean use_existing_process;
    GIntPair last_size;
    GIntPair last_position;
} VnrConfig;

typedef struct {
    VnrPrefsZoom *zoom;
    VnrPrefsDesktop *desktop;
    VnrPrefsWheel *behavior_wheel;
    VnrPrefsClick *behavior_click;
    VnrPrefsModify *behavior_modify;
    gboolean *show_hidden;
    gboolean *smooth_images;
    gboolean *confirm_delete;
    gboolean *reload_on_save;
    gboolean *show_menu_bar;
    gboolean *show_toolbar;
    gboolean *show_scrollbar;
    gboolean *show_statusbar;
    gboolean *auto_resize;
    gboolean *dark_background;
    gint *jpeg_quality;
    gint *png_compression;
    gboolean *use_existing_process;
    GIntPair *last_size;
    GIntPair *last_position;
} VnrConfigUpdate;

const VnrConfig *vnr_config_get();

void vnr_config_set(VnrConfigUpdate update);

void vnr_config_free();

//TODO: add config sync via dbus, but now call save to file every time
gboolean vnr_config_save();

void vnr_config_reload();

G_END_DECLS

#endif
