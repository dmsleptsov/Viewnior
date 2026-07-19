/*
 * Copyright © 2009-2018 Siyan Panayotov <contact@siyanpanayotov.com>
 *
 * This file is part of Viewnior.
 *
 * Viewnior is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Viewnior is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Viewnior.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VNR_PREFS_H__
#define __VNR_PREFS_H__

#include "config.h"

#include <glib.h>
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <gdk/gdkkeysyms.h>

G_BEGIN_DECLS

typedef struct _VnrPrefs VnrPrefs;
typedef struct _VnrPrefsClass VnrPrefsClass;

#define VNR_TYPE_PREFS             (vnr_prefs_get_type ())
#define VNR_PREFS(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), VNR_TYPE_PREFS, VnrPrefs))
#define VNR_PREFS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  VNR_TYPE_PREFS, VnrPrefsClass))
#define VNR_IS_PREFS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VNR_TYPE_PREFS))
#define VNR_IS_PREFS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  VNR_TYPE_PREFS))
#define VNR_PREFS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  VNR_TYPE_PREFS, VnrPrefsClass))

struct _VnrPrefs {
    GObject parent;
    GtkWidget *dialog;
    GtkWidget *vnr_win;

    GtkWidget *window;
    GObject *close_button;

    GtkToggleButton *show_hidden;
    gulong show_hidden_handler_id;

    GtkToggleButton *dark_background;
    gulong dark_background_handler_id;

    GtkToggleButton *use_existing_process;
    gulong use_existing_process_handler_id;

    GtkBox *zoom_mode_box;
    GtkComboBoxText *zoom_mode;
    gulong zoom_mode_handler_id;

    GtkToggleButton *smooth_images;
    gulong smooth_images_handler_id;

    GtkToggleButton *confirm_delete;
    gulong confirm_delete_handler_id;

    GtkToggleButton *reload_on_save;
    gulong reload_on_save_handler_id;

    GtkTable *behavior_table;

    GtkComboBoxText *action_wheel;
    gulong action_wheel_handler_id;

    GtkComboBoxText *action_click;
    gulong action_click_handler_id;

    GtkComboBoxText *action_modify;
    gulong action_modify_handler_id;

    GtkRange *jpeg_scale;
    gulong jpeg_scale_handler_id;

    GtkRange *png_scale;
    gulong png_scale_handler_id;

    GtkBox *desktop_box;
    GtkComboBoxText *desktop_env;
    gulong desktop_env_handler_id;
};

struct _VnrPrefsClass {
    GObjectClass parent_class;
};

GType     vnr_prefs_get_type (void) G_GNUC_CONST;

GObject*  vnr_prefs_new (GtkWidget *window);
void      vnr_prefs_show_dialog (VnrPrefs *prefs);
void      vnr_prefs_set_show_menu_bar     (VnrPrefs *prefs, gboolean show_menu_bar);
void      vnr_prefs_set_show_toolbar      (VnrPrefs *prefs, gboolean show_toolbar);
void      vnr_prefs_set_show_scrollbar    (VnrPrefs *prefs, gboolean show_scollbar);
void      vnr_prefs_set_show_statusbar    (VnrPrefs *prefs, gboolean show_statusbar);

void vnr_prefs_config_reload(VnrPrefs *prefs);

G_END_DECLS
#endif /* __VNR_PREFS_H__ */
