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
#include "vnr-config.h"
#include "vnr-gdbus.h"

static gchar **files = nullptr;
static gboolean version = false;

/* List of option entries
 * The only option is for specifying file to be opened. */
static GOptionEntry opt_entries[] = {
    {G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &files, NULL, "[FILE]"},
    {"version", 0, 0, G_OPTION_ARG_NONE, &version, NULL, NULL},
    {NULL}
};

gint main(gint argc, gchar **argv) {
    bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    GError *error = nullptr;
    if (!gtk_init_with_args(&argc, &argv, "- Elegant Image Viewer", opt_entries, nullptr, &error)) {
        printf("%s\nRun 'viewnior --help' to see a full list of available command line options.\n",
               g_error_get_msg(error));
        g_error_free(error);
        return EXIT_FAILURE;
    }

    if (version) {
        printf("%s\n", PACKAGE_STRING);
        return EXIT_SUCCESS;
    }

    vnr_dbus_register();

    if (vnr_config_get()->use_existing_process && vnr_dbus_send_ping_pong()) {
        vnr_dbus_send_switch_and_focus(files);
    } else {
        vnr_window_new();
        vnr_window_parse_and_show(files);
        gtk_main();
    }

    vnr_dbus_close();
    vnr_config_free();

    return EXIT_SUCCESS;
}
