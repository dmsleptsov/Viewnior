#include "vnr-gdbus.h"

#define VNR_DBUS_SERVICE "home.dm.Viewnior"
#define VNR_DBUS_INTERFACE VNR_DBUS_SERVICE
#define VNR_DBUS_PATH "/home/dm/Viewnior"
#define VNR_DBUS_METHOD_SWITCH "SwitchAndFocus"
#define VNR_DBUS_METHOD_QUIT "Quit"

#define VNR_DEFAULT_CONNECTION_TIMEOUT_MS 2000

static constexpr gchar vnr_gdbus_introspection_xml[] =
        "<node>"
        "<interface name='" VNR_DBUS_INTERFACE "'>"
        "<method name='" VNR_DBUS_METHOD_SWITCH "'>"
        "<arg type='as' name='files' direction='in'/>"
        "</method>"
        "<method name='" VNR_DBUS_METHOD_QUIT "'/>"
        "</interface>"
        "</node>";

static void gdbus_method_call(GDBusConnection *connection,
                              const gchar *sender,
                              const gchar *object_path,
                              const gchar *interface_name,
                              const gchar *method_name,
                              GVariant *parameters,
                              GDBusMethodInvocation *invocation,
                              gpointer user_data) {
    g_return_if_fail(!g_strcmp0 (object_path, VNR_DBUS_PATH));
    g_return_if_fail(!g_strcmp0 (interface_name, VNR_DBUS_INTERFACE));

    g_message("Received dbus method '%s'", method_name);

    if (g_strcmp0(method_name, VNR_DBUS_METHOD_QUIT) == 0) {
        g_dbus_method_invocation_return_value(invocation, nullptr);

        g_message("Force quit", method_name);
        gtk_main_quit();
    } else {
        g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD,
                                              "Unknown method for dbus service" VNR_DBUS_SERVICE);
    }
}


static void bus_acquired_handler(GDBusConnection *connection, const gchar *name, gpointer user_data) {
    GError *error = nullptr;

    GDBusNodeInfo *info = g_dbus_node_info_new_for_xml(vnr_gdbus_introspection_xml, &error);
    if (info == nullptr || *info->interfaces == nullptr) {
        g_message("Failed to create dbus node: %s", error->message);
        g_error_free(error);
        return;
    }

    static const GDBusInterfaceVTable vnr_gdbus_vtable =
    {
        gdbus_method_call,
        nullptr,
        nullptr
    };
    guint register_id = g_dbus_connection_register_object(connection,
                                                          VNR_DBUS_PATH,
                                                          *info->interfaces,
                                                          &vnr_gdbus_vtable,
                                                          user_data,
                                                          nullptr,
                                                          &error);

    g_message("registered interface with id %d", register_id);
    if (register_id == 0) {
        g_message("Failed to register object: %s", error->message);
        g_error_free(error);
    }

    g_dbus_node_info_unref(info);
}

gboolean vnr_register_dbus_service() {
    const guint owner_id = g_bus_own_name(G_BUS_TYPE_SESSION,
                                          VNR_DBUS_SERVICE,
                                          G_BUS_NAME_OWNER_FLAGS_NONE,
                                          bus_acquired_handler,
                                          nullptr,
                                          nullptr,
                                          nullptr,
                                          nullptr);
    g_message("Dbus register result: '%d'", owner_id);
    return !owner_id;
}

gboolean vnr_send_switch_and_focus(gint argc, gchar **argv) {
}

gboolean vnr_send_quit() {
    GError *errorConnection = nullptr;
    GDBusConnection *connection = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &errorConnection);
    if (connection == nullptr) {
        g_message("Failed to create gbus connection: '%s'", errorConnection->message);
        g_error_free(errorConnection);
        return FALSE;
    }

    GError *errorConnectionCall = nullptr;
    GVariant *reply = g_dbus_connection_call_sync(connection,
                                                  VNR_DBUS_SERVICE,
                                                  VNR_DBUS_PATH,
                                                  VNR_DBUS_INTERFACE,
                                                  VNR_DBUS_METHOD_QUIT,
                                                  nullptr,
                                                  nullptr,
                                                  G_DBUS_CALL_FLAGS_NO_AUTO_START,
                                                  0,
                                                  nullptr,
                                                  &errorConnectionCall);

    g_object_unref(connection);

    const gboolean success = reply != nullptr;
    if (success) {
        g_variant_unref(reply);
    } else {
        g_message("Failed to exec connection call: '%s'", errorConnectionCall->message);
        g_error_free(errorConnectionCall);
    }
    return success;
}
