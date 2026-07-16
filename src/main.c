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
#define _(String) gettext (String)

#include <config.h>
#include <gtk/gtk.h>
#include "config.h"
#include "vnr-window.h"
#include "vnr-message-area.h"
#include "vnr-file.h"
#include "vnr-tools.h"

#define PIXMAP_DIR        PACKAGE_DATA_DIR"/viewnior/pixmaps/"

static gchar **files = NULL;     //array of files specified to be opened
static gboolean version = FALSE;

/* List of option entries
 * The only option is for specifying file to be opened. */
static GOptionEntry opt_entries[] = {
    {G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &files, NULL, "[FILE]"},
    {"version", 0, 0, G_OPTION_ARG_NONE, &version, NULL, NULL},
    {NULL}
};

gint
main (gint argc, gchar **argv)
{
    GError *error = NULL;
    GOptionContext *opt_context;

    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    opt_context = g_option_context_new ("- Elegant Image Viewer");
    g_option_context_add_main_entries (opt_context, opt_entries, NULL);
    g_option_context_add_group (opt_context, gtk_get_option_group (TRUE));
    g_option_context_parse (opt_context, &argc, &argv, &error);

    if (error != NULL)
    {
        printf
            ("%s\nRun 'viewnior --help' to see a full list of available command line options.\n",
             error->message);
        return 1;
    }
    else if(version)
    {
        printf("%s\n", PACKAGE_STRING);
        return 0;
    }

    gtk_icon_theme_append_search_path(gtk_icon_theme_get_default(), PIXMAP_DIR);

    vnr_window_new();

    if (vnr_window_get_main()->prefs->use_existing_process && vnr_dbus_send_ping_pong()) {
        vnr_window_get_main()->one_shot_process = true;
        vnr_dbus_send_switch_and_focus(files);
        vnr_window_destroy();
    } else {
        vnr_window_parse_and_show(files);
        gtk_main();
    }
    return 0;
}
