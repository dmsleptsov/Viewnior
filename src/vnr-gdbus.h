#ifndef __VNR_DBUS_H__
#define __VNR_DBUS_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <gio/gio.h>

G_BEGIN_DECLS

gboolean vnr_register_dbus_service();

void vnr_close_dbus_service();

gboolean vnr_send_switch_and_focus(gint argc, gchar **argv);

gboolean vnr_send_quit();

G_END_DECLS

#endif
