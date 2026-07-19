#ifndef __VNR_DBUS_H__
#define __VNR_DBUS_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <gio/gio.h>

G_BEGIN_DECLS

gboolean vnr_dbus_register();

void vnr_dbus_close();

gboolean vnr_dbus_send_switch_and_focus(gchar **files, gchar* startup_id);

gboolean vnr_dbus_send_config_update();

gboolean vnr_dbus_send_quit();

gboolean vnr_dbus_send_ping_pong();

G_END_DECLS

#endif
