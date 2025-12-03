/*
 * cogbridge.h
 * Copyright (C) 2025 CogBridge
 *
 * SPDX-License-Identifier: MIT
 *
 * CogBridge - A headless embedded WebView framework for C
 * Allows bidirectional communication between C and JavaScript
 */

#pragma once

#include <glib.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
typedef struct _CogBridge CogBridge;
typedef struct _CogBridgeCallback CogBridgeCallback;

/**
 * CogBridgePlatform:
 * @COGBRIDGE_PLATFORM_DRM: Direct Rendering Manager (framebuffer, no X/Wayland)
 * @COGBRIDGE_PLATFORM_HEADLESS: Headless mode (no visual output, for testing)
 * @COGBRIDGE_PLATFORM_GTK4: GTK4 desktop window (requires X/Wayland)
 * @COGBRIDGE_PLATFORM_WAYLAND: Native Wayland (requires compositor)
 * @COGBRIDGE_PLATFORM_X11: Native X11 (requires X server)
 * @COGBRIDGE_PLATFORM_AUTO: Auto-detect or use default
 *
 * Platform backend options for CogBridge.
 */
typedef enum {
    COGBRIDGE_PLATFORM_DRM,
    COGBRIDGE_PLATFORM_HEADLESS,
    COGBRIDGE_PLATFORM_GTK4,
    COGBRIDGE_PLATFORM_WAYLAND,
    COGBRIDGE_PLATFORM_X11,
    COGBRIDGE_PLATFORM_AUTO
} CogBridgePlatform;

/**
 * CogBridgeCallbackFunc:
 * @bridge: The CogBridge instance
 * @function_name: Name of the function being called
 * @args_json: JSON string containing the arguments
 * @user_data: User data passed during registration
 *
 * Callback function type for JavaScript->C function calls.
 * The callback should return a JSON string with the result,
 * or NULL if there's no return value.
 *
 * Returns: (transfer full): JSON string with result, or NULL
 */
typedef char *(*CogBridgeCallbackFunc)(CogBridge     *bridge,
                                       const char    *function_name,
                                       const char    *args_json,
                                       void          *user_data);

/**
 * CogBridgeConfig:
 * @width: Viewport width (default: 1920)
 * @height: Viewport height (default: 1080)
 * @enable_console: Enable console messages (default: true)
 * @enable_developer_extras: Enable developer tools (default: false)
 * @cache_dir: Cache directory path (NULL for default)
 * @data_dir: Data directory path (NULL for default)
 * @user_agent: Custom user agent string (NULL for default)
 * @platform: Platform backend to use (default: COGBRIDGE_PLATFORM_AUTO)
 * @platform_name: Platform name string (deprecated, use @platform instead; NULL for auto)
 * @module_dir: Platform module directory (NULL to use built-in or COG_MODULEDIR env var)
 *
 * Configuration structure for CogBridge initialization.
 * 
 * Note: If @platform is set to anything other than COGBRIDGE_PLATFORM_AUTO,
 * it takes precedence over @platform_name.
 */
typedef struct {
    int                  width;
    int                  height;
    bool                 enable_console;
    bool                 enable_developer_extras;
    const char          *cache_dir;
    const char          *data_dir;
    const char          *user_agent;
    CogBridgePlatform    platform;
    const char          *platform_name;  /* Deprecated */
    const char          *module_dir;
} CogBridgeConfig;

/**
 * cogbridge_init:
 * @config: (nullable): Configuration options, or NULL for defaults
 * @error: (out) (optional): Error location
 *
 * Initialize CogBridge library. Must be called before any other functions.
 *
 * Returns: true on success, false on error
 */
bool cogbridge_init(const CogBridgeConfig *config, GError **error);

/**
 * cogbridge_cleanup:
 *
 * Cleanup and shutdown the CogBridge library.
 * Should be called before application exit.
 */
void cogbridge_cleanup(void);

/**
 * cogbridge_new:
 * @name: (nullable): Name/identifier for this bridge instance
 *
 * Create a new CogBridge instance.
 *
 * Returns: (transfer full): A new CogBridge instance, free with cogbridge_free()
 */
CogBridge *cogbridge_new(const char *name);

/**
 * cogbridge_free:
 * @bridge: The CogBridge instance to free
 *
 * Free a CogBridge instance and associated resources.
 */
void cogbridge_free(CogBridge *bridge);

/**
 * cogbridge_load_uri:
 * @bridge: The CogBridge instance
 * @uri: URI to load (http://, https://, file://, or data:)
 *
 * Load a URI in the webview.
 */
void cogbridge_load_uri(CogBridge *bridge, const char *uri);

/**
 * cogbridge_load_html:
 * @bridge: The CogBridge instance
 * @html: HTML content to load
 * @base_uri: (nullable): Base URI for relative resources
 *
 * Load HTML content directly into the webview.
 */
void cogbridge_load_html(CogBridge *bridge, const char *html, const char *base_uri);

/**
 * cogbridge_execute_script:
 * @bridge: The CogBridge instance
 * @script: JavaScript code to execute
 * @callback: (nullable): Callback for the result
 * @user_data: User data for the callback
 *
 * Execute JavaScript code in the webview.
 * The callback will be called with the result as a JSON string.
 */
void cogbridge_execute_script(CogBridge              *bridge,
                               const char             *script,
                               CogBridgeCallbackFunc   callback,
                               void                   *user_data);

/**
 * cogbridge_execute_script_sync:
 * @bridge: The CogBridge instance
 * @script: JavaScript code to execute
 * @error: (out) (optional): Error location
 *
 * Execute JavaScript code synchronously and return the result.
 *
 * Returns: (transfer full): JSON string with result, or NULL on error
 */
char *cogbridge_execute_script_sync(CogBridge *bridge, const char *script, GError **error);

/**
 * cogbridge_bind_function:
 * @bridge: The CogBridge instance
 * @function_name: Name of the function to expose to JavaScript
 * @callback: Callback function to invoke
 * @user_data: User data to pass to callback
 * @destroy_notify: (nullable): Function to free user_data
 *
 * Bind a C function to be callable from JavaScript.
 * The function will be available as window.cogbridge.function_name() in JavaScript.
 *
 * Returns: true on success, false on error
 */
bool cogbridge_bind_function(CogBridge              *bridge,
                              const char             *function_name,
                              CogBridgeCallbackFunc   callback,
                              void                   *user_data,
                              GDestroyNotify          destroy_notify);

/**
 * cogbridge_unbind_function:
 * @bridge: The CogBridge instance
 * @function_name: Name of the function to remove
 *
 * Remove a previously bound function.
 */
void cogbridge_unbind_function(CogBridge *bridge, const char *function_name);

/**
 * cogbridge_emit_event:
 * @bridge: The CogBridge instance
 * @event_name: Name of the event
 * @data_json: (nullable): JSON data for the event
 *
 * Emit an event to JavaScript listeners.
 * JavaScript code can listen with: window.cogbridge.on('event_name', (data) => { ... })
 */
void cogbridge_emit_event(CogBridge *bridge, const char *event_name, const char *data_json);

/**
 * cogbridge_run:
 * @bridge: The CogBridge instance
 *
 * Run the main event loop. This function blocks until cogbridge_quit() is called.
 */
void cogbridge_run(CogBridge *bridge);

/**
 * cogbridge_quit:
 * @bridge: The CogBridge instance
 *
 * Quit the main event loop.
 */
void cogbridge_quit(CogBridge *bridge);

/**
 * cogbridge_set_console_handler:
 * @bridge: The CogBridge instance
 * @handler: (nullable): Callback for console messages (NULL to disable)
 * @user_data: User data for the callback
 *
 * Set a custom handler for JavaScript console messages.
 */
void cogbridge_set_console_handler(CogBridge              *bridge,
                                   CogBridgeCallbackFunc   handler,
                                   void                   *user_data);

/**
 * cogbridge_get_default_config:
 * @config: (out): Config structure to fill with defaults
 *
 * Fill a config structure with default values.
 */
void cogbridge_get_default_config(CogBridgeConfig *config);

/**
 * cogbridge_is_ready:
 * @bridge: The CogBridge instance
 *
 * Check if the webview is ready (page loaded).
 *
 * Returns: true if ready, false otherwise
 */
bool cogbridge_is_ready(CogBridge *bridge);

/**
 * cogbridge_wait_ready:
 * @bridge: The CogBridge instance
 * @timeout_ms: Timeout in milliseconds, or -1 for no timeout
 *
 * Wait for the webview to be ready.
 *
 * Returns: true if ready within timeout, false on timeout
 */
bool cogbridge_wait_ready(CogBridge *bridge, int timeout_ms);

#ifdef __cplusplus
}
#endif