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

#include <libintl.h>
#include <glib/gi18n.h>
#define _(String) gettext (String)

#include "config.h"
#include "vnr-prefs.h"
#include "vnr-window.h"
#include "vnr-config.h"

#define UI_PATH PACKAGE_DATA_DIR"/viewnior/vnr-preferences-dialog.ui"

G_DEFINE_TYPE (VnrPrefs, vnr_prefs, G_TYPE_OBJECT);

/*************************************************************/
/***** Private signal handlers *******************************/
/*************************************************************/

static void
toggle_show_hidden_cb (GtkToggleButton *togglebutton, gpointer user_data)
{
    vnr_config_set((VnrConfigUpdate){.show_hidden = &(gboolean){gtk_toggle_button_get_active(togglebutton)}});
}

static void
toggle_dark_background_cb (GtkToggleButton *togglebutton, gpointer user_data)
{
    vnr_config_set((VnrConfigUpdate){.dark_background = &(gboolean){gtk_toggle_button_get_active(togglebutton)}});
    vnr_window_apply_preferences(VNR_WINDOW(VNR_PREFS(user_data)->vnr_win));
}

static void toggle_use_existing_process_cb(GtkToggleButton *togglebutton, gpointer user_data) {
    vnr_config_set((VnrConfigUpdate){.use_existing_process = &(gboolean){gtk_toggle_button_get_active(togglebutton)}});
    vnr_window_toggle_use_existing_process(VNR_WINDOW(VNR_PREFS(user_data)->vnr_win));
}

static void
toggle_smooth_images_cb (GtkToggleButton *togglebutton, gpointer user_data)
{
    vnr_config_set((VnrConfigUpdate){.smooth_images = &(gboolean){gtk_toggle_button_get_active(togglebutton)}});
    vnr_window_apply_preferences(VNR_WINDOW(VNR_PREFS(user_data)->vnr_win));
}

static void
toggle_confirm_delete_cb (GtkToggleButton *togglebutton, gpointer user_data)
{
    vnr_config_set((VnrConfigUpdate){.confirm_delete = &(gboolean){gtk_toggle_button_get_active(togglebutton)}});
}

static void
toggle_reload_on_save_cb (GtkToggleButton *togglebutton, gpointer user_data)
{
    vnr_config_set((VnrConfigUpdate){.reload_on_save = &(gboolean){gtk_toggle_button_get_active(togglebutton)}});
}

static void
change_zoom_mode_cb (GtkComboBox *widget, gpointer user_data)
{
    vnr_config_set((VnrConfigUpdate){.zoom = &(VnrPrefsZoom){gtk_combo_box_get_active(widget)}});
}

static void
change_desktop_env_cb (GtkComboBox *widget, gpointer user_data)
{
    vnr_config_set((VnrConfigUpdate){.desktop = &(VnrPrefsDesktop){gtk_combo_box_get_active(widget)}});
}

static void
change_jpeg_quality_cb (GtkRange *range, gpointer user_data)
{
    vnr_config_set((VnrConfigUpdate){.jpeg_quality = &(gint){(gint)gtk_range_get_value(range)}});
}

static void
change_png_compression_cb (GtkRange *range, gpointer user_data)
{
    vnr_config_set((VnrConfigUpdate){.png_compression = &(gint){(gint)gtk_range_get_value(range)}});
}

static void
change_action_wheel_cb (GtkComboBox *widget, gpointer user_data)
{
    vnr_config_set((VnrConfigUpdate){.behavior_wheel = &(VnrPrefsWheel){gtk_combo_box_get_active(widget)}});
}

static void
change_action_click_cb (GtkComboBox *widget, gpointer user_data)
{
    vnr_config_set((VnrConfigUpdate){.behavior_click = &(VnrPrefsClick){gtk_combo_box_get_active(widget)}});
}

static void
change_action_modify_cb (GtkComboBox *widget, gpointer user_data)
{
    vnr_config_set((VnrConfigUpdate){.behavior_modify = &(VnrPrefsModify){gtk_combo_box_get_active(widget)}});
}

static gboolean
key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    if(event->keyval == GDK_KEY_Escape)
    {
        gtk_widget_hide(widget);
        return TRUE;
    }
    else
        return FALSE;
}

/*************************************************************/
/***** Private actions ***************************************/
/*************************************************************/

static GtkWidget *
build_dialog (VnrPrefs *prefs)
{
    GtkBuilder *builder;
    GtkWidget *window;
    GError *error = NULL;

    GObject *close_button;
    GtkToggleButton *show_hidden;
    GtkToggleButton *dark_background;
    GtkToggleButton *use_existing_process;
    GtkBox *zoom_mode_box;
    GtkComboBoxText *zoom_mode;
    GtkToggleButton *smooth_images;
    GtkToggleButton *confirm_delete;
    GtkToggleButton *reload_on_save;
    GtkTable *behavior_table;
    GtkComboBoxText *action_wheel;
    GtkComboBoxText *action_click;
    GtkComboBoxText *action_modify;
    GtkRange *jpeg_scale;
    GtkRange *png_scale;

    GtkBox *desktop_box;
    GtkComboBoxText *desktop_env;

    builder = gtk_builder_new ();
    gtk_builder_add_from_file (builder, UI_PATH, &error);

    if (error != NULL)
    {
        g_warning ("%s\n", error->message);
        g_object_unref(builder);
        return NULL;
    }

    window = GTK_WIDGET (gtk_builder_get_object (builder, "window"));

    /* Close button */
    close_button = gtk_builder_get_object (builder, "close_button");
    g_signal_connect_swapped(close_button, "clicked",
                             G_CALLBACK(gtk_widget_hide_on_delete), window);

    /* Show hidden files checkbox */
    show_hidden = GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder, "show_hidden"));
    gtk_toggle_button_set_active( show_hidden, vnr_config_get()->show_hidden );
    g_signal_connect(G_OBJECT(show_hidden), "toggled", G_CALLBACK(toggle_show_hidden_cb), prefs);

    /* Show dark background checkbox */
    dark_background = GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder, "dark_background"));
    gtk_toggle_button_set_active( dark_background, vnr_config_get()->dark_background );
    g_signal_connect(G_OBJECT(dark_background), "toggled", G_CALLBACK(toggle_dark_background_cb), prefs);

    /* Use Existing Process checkbox*/
    use_existing_process = GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder, "use_existing_process"));
    gtk_toggle_button_set_active( use_existing_process, vnr_config_get()->use_existing_process );
    g_signal_connect(G_OBJECT(use_existing_process), "toggled", G_CALLBACK(toggle_use_existing_process_cb), prefs);

    /* Smooth images checkbox */
    smooth_images = GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder, "smooth_images"));
    gtk_toggle_button_set_active( smooth_images, vnr_config_get()->smooth_images );
    g_signal_connect(G_OBJECT(smooth_images), "toggled", G_CALLBACK(toggle_smooth_images_cb), prefs);

    /* Confirm delete checkbox */
    confirm_delete = GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder, "confirm_delete"));
    gtk_toggle_button_set_active( confirm_delete, vnr_config_get()->confirm_delete );
    g_signal_connect(G_OBJECT(confirm_delete), "toggled", G_CALLBACK(toggle_confirm_delete_cb), prefs);

    /* Reload image after save checkbox */
    reload_on_save = GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder, "reload"));
    gtk_toggle_button_set_active( reload_on_save, vnr_config_get()->reload_on_save );
    g_signal_connect(G_OBJECT(reload_on_save), "toggled", G_CALLBACK(toggle_reload_on_save_cb), prefs);

    /* JPEG quality scale */
    jpeg_scale = GTK_RANGE (gtk_builder_get_object (builder, "jpeg_scale"));
    gtk_range_set_value(jpeg_scale, (gdouble)vnr_config_get()->jpeg_quality);
    g_signal_connect(G_OBJECT(jpeg_scale), "value-changed", G_CALLBACK(change_jpeg_quality_cb), prefs);

    /* PNG compression scale */
    png_scale = GTK_RANGE (gtk_builder_get_object (builder, "png_scale"));
    gtk_range_set_value(png_scale, (gdouble)vnr_config_get()->png_compression);
    g_signal_connect(G_OBJECT(png_scale), "value-changed", G_CALLBACK(change_png_compression_cb), prefs);

    /* Zoom mode combo box */
    zoom_mode_box = GTK_BOX (gtk_builder_get_object (builder, "zoom_mode_box"));

    zoom_mode = (GtkComboBoxText*) gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(zoom_mode, _("Smart Mode"));
    gtk_combo_box_text_append_text(zoom_mode, _("1:1 Mode"));
    gtk_combo_box_text_append_text(zoom_mode, _("Fit To Window Mode"));
    gtk_combo_box_text_append_text(zoom_mode, _("Last Used Mode"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(zoom_mode), vnr_config_get()->zoom);

    gtk_box_pack_end (zoom_mode_box, GTK_WIDGET(zoom_mode), FALSE, FALSE, 0);
    gtk_widget_show(GTK_WIDGET(zoom_mode));

    g_signal_connect(G_OBJECT(zoom_mode), "changed", G_CALLBACK(change_zoom_mode_cb), prefs);

    /* Desktop combo box */
    desktop_box = GTK_BOX (gtk_builder_get_object (builder, "desktop_box"));

    desktop_env = (GtkComboBoxText*) gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(desktop_env, "GNOME 2");
    gtk_combo_box_text_append_text(desktop_env, "GNOME 3");
    gtk_combo_box_text_append_text(desktop_env, "XFCE");
    gtk_combo_box_text_append_text(desktop_env, "LXDE");
    gtk_combo_box_text_append_text(desktop_env, "PUPPY");
    gtk_combo_box_text_append_text(desktop_env, "FluxBox");
    gtk_combo_box_text_append_text(desktop_env, "Nitrogen");
    gtk_combo_box_text_append_text(desktop_env, "MATE");
    gtk_combo_box_text_append_text(desktop_env, "Cinnamon");
    gtk_combo_box_text_append_text(desktop_env, _("Autodetect"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(desktop_env), vnr_config_get()->desktop);

    gtk_box_pack_end (desktop_box, GTK_WIDGET(desktop_env), FALSE, FALSE, 0);
    gtk_widget_show(GTK_WIDGET(desktop_env));

    g_signal_connect(G_OBJECT(desktop_env), "changed", G_CALLBACK(change_desktop_env_cb), prefs);

    /* Behavior combo boxes */
    behavior_table = GTK_TABLE (gtk_builder_get_object (builder, "behavior_table"));

    action_wheel = (GtkComboBoxText*) gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(action_wheel, _("Navigate images"));
    gtk_combo_box_text_append_text(action_wheel, _("Zoom image"));
    gtk_combo_box_text_append_text(action_wheel, _("Scroll image up/down"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(action_wheel), vnr_config_get()->behavior_wheel);

    gtk_table_attach (behavior_table, GTK_WIDGET(action_wheel), 1,2,0,1, GTK_FILL,0, 0,0);
    gtk_widget_show(GTK_WIDGET(action_wheel));
    g_signal_connect(G_OBJECT(action_wheel), "changed", G_CALLBACK(change_action_wheel_cb), prefs);

    action_click = (GtkComboBoxText*) gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(action_click, _("Switch zoom modes"));
    gtk_combo_box_text_append_text(action_click, _("Enter fullscreen mode"));
    gtk_combo_box_text_append_text(action_click, _("Navigate images"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(action_click), vnr_config_get()->behavior_click);

    gtk_table_attach (behavior_table, GTK_WIDGET(action_click), 1,2,1,2, GTK_FILL,0, 0,0);
    gtk_widget_show(GTK_WIDGET(action_click));
    g_signal_connect(G_OBJECT(action_click), "changed", G_CALLBACK(change_action_click_cb), prefs);

    action_modify = (GtkComboBoxText*) gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(action_modify, _("Ask every time"));
    gtk_combo_box_text_append_text(action_modify, _("Autosave"));
    gtk_combo_box_text_append_text(action_modify, _("Ignore changes"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(action_modify), vnr_config_get()->behavior_modify);

    gtk_table_attach (behavior_table, GTK_WIDGET(action_modify), 1,2,2,3, GTK_FILL,0, 0,0);
    gtk_widget_show(GTK_WIDGET(action_modify));
    g_signal_connect(G_OBJECT(action_modify), "changed", G_CALLBACK(change_action_modify_cb), prefs);

    /* Window signals */
    g_signal_connect(G_OBJECT(window), "delete-event",
                     G_CALLBACK(gtk_widget_hide_on_delete), NULL);
    g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(key_press_cb), NULL);


    g_object_unref (G_OBJECT (builder));

    return window;
}



/*************************************************************/
/***** Stuff that deals with the type ************************/
/*************************************************************/

static void
vnr_prefs_class_init (VnrPrefsClass * klass) {}

GObject *
vnr_prefs_new (GtkWidget *vnr_win)
{
    VnrPrefs *prefs;

    prefs = g_object_new (VNR_TYPE_PREFS, NULL);

    prefs->vnr_win = vnr_win;

    return (GObject *) prefs;
}

static void
vnr_prefs_init (VnrPrefs * prefs)
{

    prefs->dialog = NULL;
}

/*************************************************************/
/***** Actions ***********************************************/
/*************************************************************/

void
vnr_prefs_show_dialog(VnrPrefs *prefs)
{
    if (prefs->dialog == NULL)
    {
        prefs->dialog = build_dialog (prefs);
        if (prefs->dialog == NULL)
            return;
    }
    gtk_window_present(GTK_WINDOW(prefs->dialog));
}

void
vnr_prefs_set_show_toolbar (VnrPrefs *prefs, gboolean show_toolbar)
{
    vnr_config_set((VnrConfigUpdate){.show_toolbar = &(gboolean){show_toolbar}});
}

void
vnr_prefs_set_show_scrollbar (VnrPrefs *prefs, gboolean show_scrollbar)
{
    vnr_config_set((VnrConfigUpdate){.show_scrollbar = &(gboolean){show_scrollbar}});
}

void
vnr_prefs_set_show_statusbar (VnrPrefs *prefs, gboolean show_statusbar)
{
    vnr_config_set((VnrConfigUpdate){.show_statusbar = &(gboolean){show_statusbar}});
}

void
vnr_prefs_set_show_menu_bar (VnrPrefs *prefs, gboolean show_menu_bar)
{
    vnr_config_set((VnrConfigUpdate){.show_menu_bar = &(gboolean){show_menu_bar}});
}
