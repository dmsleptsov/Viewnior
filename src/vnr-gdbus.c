#include "vnr-gdbus.h"

#define VNR_DBUS_SERVICE "home.dm.Viewnior"
#define VNR_DBUS_INTERFACE VNR_DBUS_SERVICE
#define VNR_DBUS_PATH "/home/dm/Viewnior"
#define VNR_DBUS_METHOD_SWITCH "SwitchAndFocus"
#define VNR_DBUS_METHOD_QUIT "Quit"

#define DEFAULT_REG_ID 0

static guint _signals_reg_id = DEFAULT_REG_ID;
static guint _methods_reg_id = DEFAULT_REG_ID;
static GDBusConnection *_connection = nullptr;

static void gdbus_signal_received(GDBusConnection *connection,
                                  const gchar *sender,
                                  const gchar *object_path,
                                  const gchar *interface_name,
                                  const gchar *signal_name,
                                  GVariant *parameters,
                                  gpointer user_data) {
    g_info("Received dbus signal '%s'", signal_name);

    const auto unique_name = g_dbus_connection_get_unique_name(connection);
    if (g_strcmp0(unique_name, sender) == 0) {
        g_info("Sender signal is equal by own '%s'=='%s', will be skipping", sender, unique_name);
        return;
    }

    if (g_strcmp0(signal_name, VNR_DBUS_METHOD_QUIT) == 0) {
        g_warning("Call force quit on '%s' signal", signal_name);
        gtk_main_quit();
    } else {
        g_warning("Unknown signal '%s' will be skipped", signal_name);
    }
}

static guint subscribe_signals(GDBusConnection *connection) {
    return g_dbus_connection_signal_subscribe(connection,
                                              nullptr,
                                              VNR_DBUS_INTERFACE,
                                              nullptr,
                                              VNR_DBUS_PATH,
                                              nullptr,
                                              G_DBUS_SIGNAL_FLAGS_NONE,
                                              gdbus_signal_received,
                                              nullptr,
                                              nullptr);
}

static gboolean is_signals_init() {
    return _signals_reg_id != DEFAULT_REG_ID;
}

static void gdbus_method_call(GDBusConnection *connection,
                              const gchar *sender,
                              const gchar *object_path,
                              const gchar *interface_name,
                              const gchar *method_name,
                              GVariant *parameters,
                              GDBusMethodInvocation *invocation,
                              gpointer user_data) {
    g_info("Received gdbus method '%s'", method_name);

    const auto unique_name = g_dbus_connection_get_unique_name(connection);
    if (g_strcmp0(unique_name, sender) == 0) {
        g_dbus_method_invocation_return_error(invocation,
                                              G_DBUS_ERROR,
                                              G_DBUS_ERROR_OBJECT_PATH_IN_USE,
                                              "Sender method is equal by own '%s'=='%s'",
                                              unique_name,
                                              method_name);
        return;
    }

    if (g_strcmp0(method_name, VNR_DBUS_METHOD_SWITCH) == 0) {
        gsize argc = g_variant_get_int32(parameters);
        const gchar **argv = g_variant_get_strv(parameters, &argc);

        g_info("Run with new files", argv);
        //TODO

        g_free(argv);
        g_dbus_method_invocation_return_value(invocation, parameters);
    } else {
        g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD,
                                              "Unknown method for dbus service" VNR_DBUS_SERVICE);
    }
}

static guint subscribe_methods(GDBusConnection *connection) {
    static constexpr gchar vnr_gdbus_introspection_xml[] =
            "<node>"
                "<interface name='" VNR_DBUS_INTERFACE "'>"
                    "<method name='" VNR_DBUS_METHOD_SWITCH "'>"
                        "<arg type='i' name='argc' direction='in'/>"
                        "<arg type='as' name='argv' direction='in'/>"
                    "</method>"
                "</interface>"
            "</node>";

    static const GDBusInterfaceVTable vnr_gdbus_vtable =
    {
        gdbus_method_call,
        nullptr,
        nullptr
    };

    GError *error = nullptr;
    GDBusNodeInfo *info = g_dbus_node_info_new_for_xml(vnr_gdbus_introspection_xml, &error);
    if (info == nullptr || *info->interfaces == nullptr) {
        g_critical("Failed to create dbus node info: %s", error->message);
        g_error_free(error);
        return DEFAULT_REG_ID;
    }

    const guint id = g_dbus_connection_register_object(connection,
                                                       VNR_DBUS_PATH,
                                                       *info->interfaces,
                                                       &vnr_gdbus_vtable,
                                                       nullptr,
                                                       nullptr,
                                                       &error);
    g_dbus_node_info_unref(info);

    if (id == DEFAULT_REG_ID) {
        g_critical("Failed register methods with message: %s", error->message);
        g_error_free(error);
    }
    return id;
}

static gboolean is_methods_init() {
    return _methods_reg_id != DEFAULT_REG_ID;
}

static GDBusConnection *get_connection() {
    if (_connection != nullptr) {
        return _connection;
    }

    static GMutex _lock;

    g_mutex_lock(&_lock);
    if (_connection != nullptr) {
        g_mutex_unlock(&_lock);
        return _connection;
    }

    GError *errorConnection = nullptr;
    _connection = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &errorConnection);
    if (_connection == nullptr) {
        g_critical("Failed to create dbus connection: '%s'", errorConnection->message);
        g_error_free(errorConnection);
    }
    g_mutex_unlock(&_lock);
    return _connection;
}

gboolean vnr_register_dbus_service() {
    GDBusConnection *connection = get_connection();
    if (connection == nullptr) {
        return false;
    }

    static GMutex signals_lock;
    g_mutex_lock(&signals_lock);
    if (!is_signals_init()) {
        _signals_reg_id = subscribe_signals(connection);
        if (!is_signals_init()) {
            g_critical("Failed subscribe to signals: '%d'", _signals_reg_id);
        } else {
            g_info("Success subscribe to signals with '%d' id", _signals_reg_id);
        }
    }
    g_mutex_unlock(&signals_lock);

    static GMutex methods_lock;
    g_mutex_lock(&methods_lock);
    if (!is_methods_init()) {
        _methods_reg_id = subscribe_methods(connection);
        if (!is_methods_init()) {
            g_critical("Failed subscribe to methods: '%d'", _methods_reg_id);
        } else {
            g_info("Success subscribe to methods with '%d' id", _methods_reg_id);
        }
    }
    g_mutex_unlock(&methods_lock);

    const gboolean success = is_signals_init() && is_methods_init();
    if (!success) {
        vnr_close_dbus_service();
    }
    return success;
}

void vnr_close_dbus_service() {
    if (_connection == nullptr) {
        return;
    }

    if (is_signals_init()) {
        g_dbus_connection_signal_unsubscribe(_connection, _signals_reg_id);
    }

    if (is_methods_init()) {
        g_dbus_connection_unregister_object(_connection, _methods_reg_id);
    }

    g_object_unref(_connection);
}


gboolean vnr_send_switch_and_focus(gint argc, gchar **argv) {
}

gboolean vnr_send_quit() {
    GDBusConnection *connection = get_connection();
    if (connection == nullptr) {
        return false;
    }

    GError *errorConnectionCall = nullptr;
    const gboolean success = g_dbus_connection_emit_signal(connection,
                                                           nullptr,
                                                           VNR_DBUS_PATH,
                                                           VNR_DBUS_INTERFACE,
                                                           VNR_DBUS_METHOD_QUIT,
                                                           nullptr,
                                                           &errorConnectionCall);
    if (!success) {
        g_critical("Failed emit signal: '%s'", errorConnectionCall->message);
        g_error_free(errorConnectionCall);
    }
    return success;
}
