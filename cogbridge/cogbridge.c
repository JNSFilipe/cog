/*
 * cogbridge.c
 * Copyright (C) 2025 CogBridge
 *
 * SPDX-License-Identifier: MIT
 *
 * CogBridge - A headless embedded WebView framework for C
 * Implementation file
 */

#include "cogbridge.h"
#include "../core/cog.h"
#include <json-glib/json-glib.h>
#include <stdlib.h>
#include <string.h>

/* Global state */
static CogShell *global_shell = NULL;
static CogPlatform *global_platform = NULL;
static GMainLoop *global_main_loop = NULL;
static CogBridgeConfig global_config = {0};

/* CogBridge structure */
struct _CogBridge {
    char *name;
    CogViewport *viewport;
    CogView *view;
    WebKitWebView *webview;
    WebKitUserContentManager *content_manager;
    GHashTable *bound_functions;
    GMainLoop *loop;
    bool is_ready;
    CogBridgeCallbackFunc console_handler;
    void *console_handler_data;
};

/* Callback data for bound functions */
typedef struct {
    CogBridgeCallbackFunc callback;
    void *user_data;
    GDestroyNotify destroy_notify;
} BoundFunctionData;

/* Async script execution data */
typedef struct {
    CogBridgeCallbackFunc callback;
    void *user_data;
} ScriptCallbackData;

static void
bound_function_data_free(BoundFunctionData *data)
{
    if (data->destroy_notify && data->user_data)
        data->destroy_notify(data->user_data);
    g_free(data);
}

void
cogbridge_get_default_config(CogBridgeConfig *config)
{
    g_return_if_fail(config != NULL);

    config->width = 1920;
    config->height = 1080;
    config->enable_console = true;
    config->enable_developer_extras = false;
    config->cache_dir = NULL;
    config->data_dir = NULL;
    config->user_agent = NULL;
    
    // Platform selection (default to auto-detect)
    config->platform = COGBRIDGE_PLATFORM_AUTO;
    config->platform_name = NULL;  // Deprecated field
    
    // Module directory (NULL = auto-detect from COG_MODULEDIR env or built-in path)
    config->module_dir = NULL;
}

#if COG_USE_WPE2
static void
on_console_message(WebKitWebView *webview,
                   WebKitConsoleMessage *message,
                   CogBridge *bridge)
{
    if (!global_config.enable_console)
        return;

    const char *msg = webkit_console_message_get_text(message);
    guint line = webkit_console_message_get_line(message);
    const char *source = webkit_console_message_get_source(message);
    WebKitConsoleMessageLevel level = webkit_console_message_get_level(message);

    const char *level_str = "LOG";
    switch (level) {
        case WEBKIT_CONSOLE_MESSAGE_LEVEL_ERROR:
            level_str = "ERROR";
            break;
        case WEBKIT_CONSOLE_MESSAGE_LEVEL_WARNING:
            level_str = "WARN";
            break;
        case WEBKIT_CONSOLE_MESSAGE_LEVEL_INFO:
            level_str = "INFO";
            break;
        case WEBKIT_CONSOLE_MESSAGE_LEVEL_DEBUG:
            level_str = "DEBUG";
            break;
        default:
            break;
    }

    if (bridge->console_handler) {
        g_autofree char *json = g_strdup_printf(
            "{\"level\":\"%s\",\"message\":\"%s\",\"source\":\"%s\",\"line\":%u}",
            level_str, msg, source, line);
        bridge->console_handler(bridge, "console", json, bridge->console_handler_data);
    } else {
        g_message("[JS %s] %s:%u: %s", level_str, source, line, msg);
    }
}
#endif

static void
on_load_changed(WebKitWebView *webview,
                WebKitLoadEvent load_event,
                CogBridge *bridge)
{
    if (load_event == WEBKIT_LOAD_FINISHED) {
        bridge->is_ready = true;
        g_message("CogBridge: Page loaded and ready");
    }
}

static void
on_message_received(WebKitUserContentManager *manager,
#if COG_USE_WPE2
                    WebKitUserMessage *message,
#else
                    WebKitJavascriptResult *message,
#endif
                    CogBridge *bridge)
{
#if COG_USE_WPE2
    const char *name = webkit_user_message_get_name(message);
    GVariant *parameters = webkit_user_message_get_parameters(message);
    
    if (g_strcmp0(name, "cogbridge-call") != 0)
        return;

    const char *function_name = NULL;
    const char *args_json = NULL;
    g_variant_get(parameters, "(&s&s)", &function_name, &args_json);
#else
    JSCValue *value = webkit_javascript_result_get_js_value(message);
    if (!jsc_value_is_object(value))
        return;

    JSCValue *name_val = jsc_value_object_get_property(value, "function");
    JSCValue *args_val = jsc_value_object_get_property(value, "args");
    
    if (!jsc_value_is_string(name_val) || !jsc_value_is_string(args_val)) {
        g_object_unref(name_val);
        g_object_unref(args_val);
        return;
    }

    g_autofree char *function_name = jsc_value_to_string(name_val);
    g_autofree char *args_json = jsc_value_to_string(args_val);
    g_object_unref(name_val);
    g_object_unref(args_val);
#endif

    BoundFunctionData *func_data = g_hash_table_lookup(bridge->bound_functions, function_name);
    if (!func_data) {
        g_warning("CogBridge: Called unbound function: %s", function_name);
        return;
    }

    g_autofree char *result = func_data->callback(bridge, function_name, args_json, func_data->user_data);
    
    if (result) {
        g_autofree char *script = g_strdup_printf(
            "window.cogbridge._resolveCall('%s', %s);",
            function_name, result);
        webkit_web_view_run_javascript(bridge->webview, script, NULL, NULL, NULL);
    }
}

bool
cogbridge_init(const CogBridgeConfig *config, GError **error)
{
    if (global_shell) {
        g_set_error(error, g_quark_from_static_string("cogbridge"), 1,
                    "CogBridge already initialized");
        return false;
    }

    // Set default config if not provided
    if (config) {
        global_config = *config;
    } else {
        cogbridge_get_default_config(&global_config);
    }

    // Determine platform name from enum or string
    const char *platform_name = NULL;
    
    // New enum-based platform selection takes precedence
    if (global_config.platform != COGBRIDGE_PLATFORM_AUTO) {
        switch (global_config.platform) {
            case COGBRIDGE_PLATFORM_DRM:
                platform_name = "drm";
                break;
            case COGBRIDGE_PLATFORM_HEADLESS:
                platform_name = "headless";
                break;
            case COGBRIDGE_PLATFORM_GTK4:
                platform_name = "gtk4";
                break;
            case COGBRIDGE_PLATFORM_WAYLAND:
                platform_name = "wayland";
                break;
            case COGBRIDGE_PLATFORM_X11:
                platform_name = "x11";
                break;
            case COGBRIDGE_PLATFORM_AUTO:
                // Will use default below
                break;
        }
    }
    
    // Fall back to deprecated platform_name string field
    if (!platform_name && global_config.platform_name) {
        platform_name = global_config.platform_name;
    }
    
    // Final fallback to "drm" for direct framebuffer
    if (!platform_name) {
        platform_name = "drm";
    }

    // Initialize cog with platform and module directory
    cog_init(platform_name, global_config.module_dir);

    // Create shell
    global_shell = cog_shell_new("cogbridge", false);
    if (!global_shell) {
        g_set_error(error, g_quark_from_static_string("cogbridge"), 2,
                    "Failed to create CogShell");
        return false;
    }

    // Set device scale factor to ensure valid value for WPE (requires 0.05-5.0 range)
    g_object_set(global_shell, "device-scale-factor", 1.0, NULL);

    // Configure WebKit settings
    WebKitSettings *settings = cog_shell_get_web_settings(global_shell);
    webkit_settings_set_enable_developer_extras(settings, global_config.enable_developer_extras);
    webkit_settings_set_enable_write_console_messages_to_stdout(settings, global_config.enable_console);
    webkit_settings_set_javascript_can_access_clipboard(settings, true);
    webkit_settings_set_javascript_can_open_windows_automatically(settings, false);
    
    if (global_config.user_agent)
        webkit_settings_set_user_agent(settings, global_config.user_agent);

    // Get platform
    global_platform = cog_platform_get();
    if (!global_platform) {
        g_set_error(error, g_quark_from_static_string("cogbridge"), 3,
                    "Failed to get platform");
        g_object_unref(global_shell);
        global_shell = NULL;
        return false;
    }

    // Setup platform (headless)
    if (!cog_platform_setup(global_platform, global_shell, NULL, error)) {
        g_object_unref(global_shell);
        global_shell = NULL;
        return false;
    }

    // Create main loop
    global_main_loop = g_main_loop_new(NULL, false);

    g_message("CogBridge initialized successfully");
    return true;
}

void
cogbridge_cleanup(void)
{
    if (global_main_loop) {
        g_main_loop_unref(global_main_loop);
        global_main_loop = NULL;
    }

    if (global_platform) {
        global_platform = NULL;
    }

    if (global_shell) {
        g_object_unref(global_shell);
        global_shell = NULL;
    }

    g_message("CogBridge cleaned up");
}

CogBridge *
cogbridge_new(const char *name)
{
    g_return_val_if_fail(global_shell != NULL, NULL);

    CogBridge *bridge = g_new0(CogBridge, 1);
    bridge->name = g_strdup(name ? name : "cogbridge");
    bridge->bound_functions = g_hash_table_new_full(g_str_hash, g_str_equal,
                                                     g_free, (GDestroyNotify)bound_function_data_free);
    bridge->is_ready = false;

    // Create viewport
    bridge->viewport = cog_viewport_new();
    
    // Create view using platform-specific type
    CogPlatformClass *platform_class = COG_PLATFORM_GET_CLASS(global_platform);
    GType view_type = COG_TYPE_VIEW_IMPL;
    
    if (platform_class && platform_class->get_view_type) {
        view_type = platform_class->get_view_type();
    }
    
    bridge->view = g_object_new(view_type, NULL);
    bridge->webview = WEBKIT_WEB_VIEW(bridge->view);

    // Initialize web view with platform
    cog_platform_init_web_view(global_platform, bridge->webview);
    
    // Add view to viewport and make it visible
    cog_viewport_add(bridge->viewport, bridge->view);
    cog_viewport_set_visible_view(bridge->viewport, bridge->view);

    // Get user content manager
    bridge->content_manager = webkit_web_view_get_user_content_manager(bridge->webview);

    // Connect signals
#if COG_USE_WPE2
    g_signal_connect(bridge->webview, "console-message",
                     G_CALLBACK(on_console_message), bridge);
#endif
    g_signal_connect(bridge->webview, "load-changed",
                     G_CALLBACK(on_load_changed), bridge);

    // Register message handler for function calls
#if COG_USE_WPE2
    webkit_user_content_manager_register_script_message_handler(
        bridge->content_manager, "cogbridge", NULL);
    g_signal_connect(bridge->content_manager, "script-message-received::cogbridge",
                     G_CALLBACK(on_message_received), bridge);
#else
    webkit_user_content_manager_register_script_message_handler(
        bridge->content_manager, "cogbridge");
    g_signal_connect(bridge->content_manager, "script-message-received::cogbridge",
                     G_CALLBACK(on_message_received), bridge);
#endif

    // Inject CogBridge JavaScript API
    const char *init_script = 
        "window.cogbridge = {"
        "  _callbacks: {},"
        "  _eventListeners: {},"
        "  call: function(name, ...args) {"
        "    return new Promise((resolve, reject) => {"
        "      const id = Math.random().toString(36);"
        "      this._callbacks[id] = { resolve, reject, name };"
        "      const message = { function: name, args: JSON.stringify(args), id };"
#if COG_USE_WPE2
        "      window.webkit.messageHandlers.cogbridge.postMessage(JSON.stringify(message));"
#else
        "      window.webkit.messageHandlers.cogbridge.postMessage(message);"
#endif
        "    });"
        "  },"
        "  _resolveCall: function(name, result) {"
        "    for (let id in this._callbacks) {"
        "      if (this._callbacks[id].name === name) {"
        "        this._callbacks[id].resolve(result);"
        "        delete this._callbacks[id];"
        "        break;"
        "      }"
        "    }"
        "  },"
        "  on: function(event, callback) {"
        "    if (!this._eventListeners[event]) {"
        "      this._eventListeners[event] = [];"
        "    }"
        "    this._eventListeners[event].push(callback);"
        "  },"
        "  _emit: function(event, data) {"
        "    if (this._eventListeners[event]) {"
        "      this._eventListeners[event].forEach(cb => cb(data));"
        "    }"
        "  }"
        "};";

    WebKitUserScript *script = webkit_user_script_new(
        init_script,
        WEBKIT_USER_CONTENT_INJECT_TOP_FRAME,
        WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
        NULL, NULL);
    webkit_user_content_manager_add_script(bridge->content_manager, script);
    webkit_user_script_unref(script);

    g_message("CogBridge instance created: %s", bridge->name);
    return bridge;
}

void
cogbridge_free(CogBridge *bridge)
{
    g_return_if_fail(bridge != NULL);

    g_message("Freeing CogBridge instance: %s", bridge->name);

    if (bridge->bound_functions)
        g_hash_table_destroy(bridge->bound_functions);

    if (bridge->viewport)
        g_object_unref(bridge->viewport);

    if (bridge->view)
        g_object_unref(bridge->view);

    g_free(bridge->name);
    g_free(bridge);
}

void
cogbridge_load_uri(CogBridge *bridge, const char *uri)
{
    g_return_if_fail(bridge != NULL);
    g_return_if_fail(uri != NULL);

    bridge->is_ready = false;
    webkit_web_view_load_uri(bridge->webview, uri);
    g_message("CogBridge: Loading URI: %s", uri);
}

void
cogbridge_load_html(CogBridge *bridge, const char *html, const char *base_uri)
{
    g_return_if_fail(bridge != NULL);
    g_return_if_fail(html != NULL);

    bridge->is_ready = false;
    webkit_web_view_load_html(bridge->webview, html, base_uri);
    g_message("CogBridge: Loading HTML content");
}

static void
on_execute_script_finished(GObject *object, GAsyncResult *result, gpointer user_data)
{
    ScriptCallbackData *data = user_data;
    WebKitJavascriptResult *js_result;
    GError *error = NULL;

    js_result = webkit_web_view_run_javascript_finish(WEBKIT_WEB_VIEW(object), result, &error);

    if (error) {
        g_warning("CogBridge: Script execution error: %s", error->message);
        if (data->callback)
            data->callback(NULL, NULL, NULL, data->user_data);
        g_error_free(error);
    } else if (data->callback) {
        JSCValue *value = webkit_javascript_result_get_js_value(js_result);
        g_autofree char *result_str = jsc_value_to_string(value);
        data->callback(NULL, NULL, result_str, data->user_data);
        webkit_javascript_result_unref(js_result);
    }

    g_free(data);
}

void
cogbridge_execute_script(CogBridge *bridge,
                         const char *script,
                         CogBridgeCallbackFunc callback,
                         void *user_data)
{
    g_return_if_fail(bridge != NULL);
    g_return_if_fail(script != NULL);

    ScriptCallbackData *data = g_new0(ScriptCallbackData, 1);
    data->callback = callback;
    data->user_data = user_data;

    webkit_web_view_run_javascript(bridge->webview, script, NULL,
                                    on_execute_script_finished, data);
}

char *
cogbridge_execute_script_sync(CogBridge *bridge, const char *script, GError **error)
{
    g_return_val_if_fail(bridge != NULL, NULL);
    g_return_val_if_fail(script != NULL, NULL);

    // Note: Synchronous execution not fully implemented for WPE 1.1
    // This is a placeholder that executes asynchronously
    g_warning("cogbridge_execute_script_sync: Synchronous execution not fully supported, executing asynchronously");
    webkit_web_view_run_javascript(bridge->webview, script, NULL, NULL, NULL);
    return NULL;
}

bool
cogbridge_bind_function(CogBridge *bridge,
                        const char *function_name,
                        CogBridgeCallbackFunc callback,
                        void *user_data,
                        GDestroyNotify destroy_notify)
{
    g_return_val_if_fail(bridge != NULL, false);
    g_return_val_if_fail(function_name != NULL, false);
    g_return_val_if_fail(callback != NULL, false);

    BoundFunctionData *data = g_new0(BoundFunctionData, 1);
    data->callback = callback;
    data->user_data = user_data;
    data->destroy_notify = destroy_notify;

    g_hash_table_insert(bridge->bound_functions, g_strdup(function_name), data);

    // Add JavaScript wrapper
    g_autofree char *script = g_strdup_printf(
        "window.cogbridge.%s = function(...args) {"
        "  return window.cogbridge.call('%s', ...args);"
        "};",
        function_name, function_name);

    webkit_web_view_run_javascript(bridge->webview, script, NULL, NULL, NULL);

    g_message("CogBridge: Bound function: %s", function_name);
    return true;
}

void
cogbridge_unbind_function(CogBridge *bridge, const char *function_name)
{
    g_return_if_fail(bridge != NULL);
    g_return_if_fail(function_name != NULL);

    g_hash_table_remove(bridge->bound_functions, function_name);

    g_autofree char *script = g_strdup_printf("delete window.cogbridge.%s;", function_name);
    webkit_web_view_run_javascript(bridge->webview, script, NULL, NULL, NULL);

    g_message("CogBridge: Unbound function: %s", function_name);
}

void
cogbridge_emit_event(CogBridge *bridge, const char *event_name, const char *data_json)
{
    g_return_if_fail(bridge != NULL);
    g_return_if_fail(event_name != NULL);

    const char *json = data_json ? data_json : "null";
    g_autofree char *script = g_strdup_printf(
        "window.cogbridge._emit('%s', %s);",
        event_name, json);

    webkit_web_view_run_javascript(bridge->webview, script, NULL, NULL, NULL);
}

void
cogbridge_run(CogBridge *bridge)
{
    g_return_if_fail(bridge != NULL);
    g_return_if_fail(global_main_loop != NULL);

    g_message("CogBridge: Starting main loop");
    g_main_loop_run(global_main_loop);
}

void
cogbridge_quit(CogBridge *bridge)
{
    g_return_if_fail(bridge != NULL);
    g_return_if_fail(global_main_loop != NULL);

    g_message("CogBridge: Quitting main loop");
    g_main_loop_quit(global_main_loop);
}

void
cogbridge_set_console_handler(CogBridge *bridge,
                               CogBridgeCallbackFunc handler,
                               void *user_data)
{
    g_return_if_fail(bridge != NULL);

    bridge->console_handler = handler;
    bridge->console_handler_data = user_data;
}

bool
cogbridge_is_ready(CogBridge *bridge)
{
    g_return_val_if_fail(bridge != NULL, false);
    return bridge->is_ready;
}

bool
cogbridge_wait_ready(CogBridge *bridge, int timeout_ms)
{
    g_return_val_if_fail(bridge != NULL, false);

    GTimer *timer = g_timer_new();
    
    while (!bridge->is_ready) {
        g_main_context_iteration(NULL, false);
        
        if (timeout_ms >= 0) {
            double elapsed = g_timer_elapsed(timer, NULL);
            if (elapsed * 1000 >= timeout_ms) {
                g_timer_destroy(timer);
                return false;
            }
        }
        
        g_usleep(10000); // 10ms
    }
    
    g_timer_destroy(timer);
    return true;
}