/*
 * minimal.c
 * Copyright (C) 2024 CogBridge Examples
 *
 * SPDX-License-Identifier: MIT
 *
 * Truly minimal CogBridge example demonstrating core features:
 * - Loading HTML content
 * - Binding C functions callable from JavaScript
 * - Emitting events from C to JavaScript
 * - Running the event loop
 *
 * Build:
 *   meson setup build -Dplatforms=drm -Dexamples=true
 *   ninja -C build
 *
 * Run:
 *   sudo COG_MODULEDIR=./build/platform/drm ./build/cogbridge/examples/cogbridge-minimal
 *
 * Platform selection: Set config.platform_name to "drm", "headless", or "gtk4"
 */

#include "../cogbridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <json-glib/json-glib.h>

/* Simple HTML with interactive buttons */
static const char *html_content =
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "    <meta charset='UTF-8'>"
    "    <title>CogBridge Minimal</title>"
    "    <style>"
    "        body { font-family: Arial, sans-serif; padding: 20px; background: #f0f0f0; }"
    "        .container { max-width: 600px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; }"
    "        button { padding: 10px 20px; margin: 5px; cursor: pointer; background: #4CAF50; color: white; border: none; border-radius: 4px; }"
    "        button:hover { background: #45a049; }"
    "        #log { margin-top: 15px; padding: 10px; background: #f5f5f5; border-radius: 4px; font-family: monospace; font-size: 12px; max-height: 300px; overflow-y: auto; }"
    "        .entry { margin: 3px 0; }"
    "    </style>"
    "</head>"
    "<body>"
    "    <div class='container'>"
    "        <h1>CogBridge Minimal Example</h1>"
    "        <p>Bidirectional C ↔ JavaScript communication</p>"
    "        <button onclick='callAdd()'>Call add(5, 7)</button>"
    "        <button onclick='callGreet()'>Call greet()</button>"
    "        <button onclick='requestEvent()'>Request Event</button>"
    "        <div id='log'></div>"
    "    </div>"
    "    <script>"
    "        function log(msg) {"
    "            const log = document.getElementById('log');"
    "            const entry = document.createElement('div');"
    "            entry.className = 'entry';"
    "            entry.textContent = new Date().toLocaleTimeString() + ': ' + msg;"
    "            log.insertBefore(entry, log.firstChild);"
    "        }"
    "        "
    "        async function callAdd() {"
    "            const result = await window.cogbridge.add(5, 7);"
    "            log('add(5, 7) = ' + result);"
    "        }"
    "        "
    "        async function callGreet() {"
    "            const result = await window.cogbridge.greet('CogBridge');"
    "            log('greet() returned: ' + result);"
    "        }"
    "        "
    "        function requestEvent() {"
    "            window.cogbridge.request_event();"
    "        }"
    "        "
    "        window.cogbridge.on('notification', (data) => {"
    "            log('EVENT: ' + data.message);"
    "        });"
    "        "
    "        log('Page loaded!');"
    "    </script>"
    "</body>"
    "</html>";

/* Add two numbers */
static char *
on_add(CogBridge *bridge, const char *fn, const char *args_json, void *user_data)
{
    JsonParser *parser = json_parser_new();
    if (!json_parser_load_from_data(parser, args_json, -1, NULL)) {
        g_object_unref(parser);
        return g_strdup("null");
    }
    
    JsonArray *array = json_node_get_array(json_parser_get_root(parser));
    int a = json_array_get_int_element(array, 0);
    int b = json_array_get_int_element(array, 1);
    g_object_unref(parser);
    
    int result = a + b;
    g_print("[C] add(%d, %d) = %d\n", a, b, result);
    
    return g_strdup_printf("%d", result);
}

/* Greet someone */
static char *
on_greet(CogBridge *bridge, const char *fn, const char *args_json, void *user_data)
{
    JsonParser *parser = json_parser_new();
    if (!json_parser_load_from_data(parser, args_json, -1, NULL)) {
        g_object_unref(parser);
        return g_strdup("\"Error\"");
    }
    
    JsonArray *array = json_node_get_array(json_parser_get_root(parser));
    const char *name = json_array_get_string_element(array, 0);
    g_object_unref(parser);
    
    g_print("[C] greet('%s')\n", name);
    
    return g_strdup_printf("\"Hello, %s!\"", name);
}

/* Request event - triggers C to emit event to JavaScript */
static char *
on_request_event(CogBridge *bridge, const char *fn, const char *args_json, void *user_data)
{
    g_print("[C] Emitting event to JavaScript\n");
    cogbridge_emit_event(bridge, "notification", "{\"message\":\"Event from C!\"}");
    return g_strdup("\"Event emitted\"");
}

int
main(int argc, char *argv[])
{
    g_print("=== CogBridge Minimal Example ===\n\n");
    
    /* Initialize CogBridge with compile-time platform selection */
    CogBridgeConfig config;
    cogbridge_get_default_config(&config);
    config.enable_console = TRUE;
    
    /* Platform is set at compile time via build system */
#ifdef COGBRIDGE_PLATFORM_DEFAULT
    config.platform = COGBRIDGE_PLATFORM_DEFAULT;
#endif
#ifdef COGBRIDGE_MODULE_DIR
    config.module_dir = COGBRIDGE_MODULE_DIR;
#endif
    
    if (!cogbridge_init(&config, NULL)) {
        g_printerr("ERROR: Failed to initialize CogBridge\n");
        return 1;
    }
    
    /* Create bridge instance */
    CogBridge *bridge = cogbridge_new("minimal-example");
    if (!bridge) {
        g_printerr("ERROR: Failed to create CogBridge instance\n");
        return 1;
    }
    
    /* Bind C functions to JavaScript */
    cogbridge_bind_function(bridge, "add", on_add, NULL, NULL);
    cogbridge_bind_function(bridge, "greet", on_greet, NULL, NULL);
    cogbridge_bind_function(bridge, "request_event", on_request_event, bridge, NULL);
    
    /* Load HTML content */
    cogbridge_load_html(bridge, html_content, "file:///");
    
    g_print("Starting event loop...\n");
    g_print("Interact with the UI in the browser/display\n\n");
    
    /* Run event loop (blocks until quit) */
    cogbridge_run(bridge);
    
    /* Cleanup */
    g_object_unref(bridge);
    g_print("\nExample finished.\n");
    
    return 0;
}