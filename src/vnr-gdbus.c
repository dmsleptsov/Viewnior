#include "vnr-gdbus.h"
#include "vnr-tools.h"
#include "vnr-window.h"

#define VNR_DBUS_SERVICE VIEWNIOR_ID
#define VNR_DBUS_INTERFACE VIEWNIOR_ID
#define VNR_DBUS_PATH VIEWNIOR_PATH
#define VNR_DBUS_METHOD_SWITCH "SwitchAndFocus"
#define VNR_DBUS_METHOD_PING "PingPong"
#define VNR_DBUS_METHOD_QUIT "Quit"

#define DEFAULT_REG_ID 0

static guint _signals_reg_id = DEFAULT_REG_ID;
static guint _methods_reg_id = DEFAULT_REG_ID;
static GDBusConnection *_connection = nullptr;
static gchar *_empty_string_array[] = {nullptr};
static const gchar *_empty_string = "";

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
        vnr_window_destroy();
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
    } else if (g_strcmp0(method_name, VNR_DBUS_METHOD_PING) == 0) {
        g_info("Send response for ping-pong");
        g_dbus_method_invocation_return_value(invocation, parameters);
    } else if (g_strcmp0(method_name, VNR_DBUS_METHOD_SWITCH) == 0) {
        gchar **files = nullptr;
        gchar *startup_id = nullptr;
        g_variant_get(parameters, "(^ass)", &files, &startup_id);

        vnr_window_parse_and_show(files, startup_id);

        g_free(files);
        g_dbus_method_invocation_return_value(invocation, nullptr);
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
                        "<arg type='as' name='files' direction='in'/>"
                        "<arg type='s' name='startup_id' direction='in'/>"
                    "</method>"
                    "<method name='" VNR_DBUS_METHOD_PING "'/>"
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
        g_critical("Failed to create dbus node info: '%s'", g_error_get_msg(error));
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
        g_critical("Failed register methods with message: '%s'", g_error_get_msg(error));
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

    GError *error = nullptr;
    _connection = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error);
    if (_connection == nullptr) {
        g_critical("Failed to create dbus connection: '%s'", g_error_get_msg(error));
        g_error_free(error);
    } else {
        const guint owner_id = g_bus_own_name_on_connection(_connection,
                                                            VNR_DBUS_SERVICE,
                                                            G_BUS_NAME_OWNER_FLAGS_NONE,
                                                            nullptr,
                                                            nullptr,
                                                            nullptr,
                                                            nullptr);
        g_info("Success create dbus '%s' connection and '%d' own id", g_dbus_connection_get_unique_name(_connection),
               owner_id);
    }
    g_mutex_unlock(&_lock);
    return _connection;
}

gboolean vnr_dbus_register() {
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
            g_critical("Failed register methods: '%d'", _methods_reg_id);
        } else {
            g_info("Success register methods with '%d' id", _methods_reg_id);
        }
    }
    g_mutex_unlock(&methods_lock);

    const gboolean success = is_signals_init() && is_methods_init();
    if (!success) {
        vnr_dbus_close();
    }
    return success;
}

void vnr_dbus_close() {
    if (_connection == nullptr) {
        return;
    }

    if (is_signals_init()) {
        g_dbus_connection_signal_unsubscribe(_connection, _signals_reg_id);
        g_info("Unsubscribe from signals '%d'", _signals_reg_id);
        _signals_reg_id = DEFAULT_REG_ID;
    }

    if (is_methods_init()) {
        g_dbus_connection_unregister_object(_connection, _methods_reg_id);
        g_info("Unsubscribe from method '%d'", _methods_reg_id);
        _methods_reg_id = DEFAULT_REG_ID;
    }

    const gchar *unique_name = g_dbus_connection_get_unique_name(_connection);
    g_info("Closing dbus '%s' connection", unique_name);

    GError *flush_error = nullptr;
    if (!g_dbus_connection_flush_sync(_connection, nullptr, &flush_error)) {
        g_critical("Error during flushing '%s' connection: '%s'", unique_name, g_error_get_msg(flush_error));
        g_error_free(flush_error);
    }

    GError *close_error = nullptr;
    if (!g_dbus_connection_close_sync(_connection, nullptr, &close_error)) {
        g_critical("Error during closing '%s' connection: '%s'", unique_name, g_error_get_msg(close_error));
        g_error_free(close_error);
    }

    g_clear_object(&_connection);
}

gboolean vnr_dbus_send_switch_and_focus(gchar **files, gchar* startup_id) {
    GDBusConnection *connection = get_connection();
    if (connection == nullptr) {
        return false;
    }

    GError *error = nullptr;
    GVariant *parameters =  g_variant_new(
        "(^ass)",
        files == nullptr ? _empty_string_array : files,
        startup_id == nullptr ? _empty_string : startup_id
        );

    GVariant *reply = g_dbus_connection_call_sync(connection,
                                                  VNR_DBUS_SERVICE,
                                                  VNR_DBUS_PATH,
                                                  VNR_DBUS_INTERFACE,
                                                  VNR_DBUS_METHOD_SWITCH,
                                                  parameters,
                                                  nullptr,
                                                  G_DBUS_CALL_FLAGS_NONE,
                                                  2000,
                                                  nullptr,
                                                  &error);

    const gboolean success = reply != nullptr;
    if (!success) {
        g_critical("Error during method call: '%s'", g_error_get_msg(error));
        g_error_free(error);
    } else {
        g_info("Success " VNR_DBUS_METHOD_SWITCH " method call");
        g_variant_unref(reply);
    }
    return success;
}

gboolean vnr_dbus_send_quit() {
    GDBusConnection *connection = get_connection();
    if (connection == nullptr) {
        return false;
    }

    GError *error = nullptr;
    const gboolean success = g_dbus_connection_emit_signal(connection,
                                                           nullptr,
                                                           VNR_DBUS_PATH,
                                                           VNR_DBUS_INTERFACE,
                                                           VNR_DBUS_METHOD_QUIT,
                                                           nullptr,
                                                           &error);
    if (!success) {
        g_critical("Failed emit signal: '%s'", g_error_get_msg(error));
        g_error_free(error);
    } else {
        g_info("Success emit '" VNR_DBUS_METHOD_QUIT "' signal");
    }
    return success;
}

gboolean vnr_dbus_send_ping_pong() {
    GDBusConnection *connection = get_connection();
    if (connection == nullptr) {
        return false;
    }

    GError *error = nullptr;
    GVariant *reply = g_dbus_connection_call_sync(connection,
                                                  VNR_DBUS_SERVICE,
                                                  VNR_DBUS_PATH,
                                                  VNR_DBUS_INTERFACE,
                                                  VNR_DBUS_METHOD_PING,
                                                  nullptr,
                                                  nullptr,
                                                  G_DBUS_CALL_FLAGS_NONE,
                                                  10,
                                                  nullptr,
                                                  &error);
    const gboolean is_reply = reply != nullptr;
    const gboolean is_error = error != nullptr;
    const gboolean answered = is_reply && !is_error;

    if (is_reply) {
        g_variant_unref(reply);
    }

    if (is_error) {
        g_error_free(error);
    }

    g_info("Have another instances: %s", answered ? "true" : "false");
    return answered;
}
