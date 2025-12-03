# CogBridge Quick Reference Card

## Initialization

```c
#include <cogbridge/cogbridge.h>

// Get defaults
CogBridgeConfig config;
cogbridge_get_default_config(&config);

// Customize
config.width = 1920;
config.height = 1080;
config.enable_console = true;

// Initialize
GError *error = NULL;
if (!cogbridge_init(&config, &error)) {
    g_error_free(error);
    return 1;
}
```

## Creating a Bridge

```c
// Create
CogBridge *bridge = cogbridge_new("my-app");

// Load content
cogbridge_load_uri(bridge, "https://example.com");
// or
cogbridge_load_html(bridge, html_string, "file:///");

// Wait for ready
cogbridge_wait_ready(bridge, 10000); // 10 second timeout
```

## Binding C Functions

```c
// Define callback
static char *my_function(CogBridge *bridge, const char *function_name,
                         const char *args_json, void *user_data)
{
    // Parse args_json with json-glib
    // Do work
    // Return JSON string (caller will free)
    return g_strdup("{\"result\":42}");
}

// Bind to JavaScript
cogbridge_bind_function(bridge, "myFunction", my_function, NULL, NULL);

// With user data
MyData *data = g_new0(MyData, 1);
cogbridge_bind_function(bridge, "func", callback, data, 
                       (GDestroyNotify)my_data_free);

// Unbind
cogbridge_unbind_function(bridge, "myFunction");
```

## Calling from JavaScript

```javascript
// All bound functions available on window.cogbridge
const result = await window.cogbridge.myFunction(arg1, arg2, arg3);

// Error handling
try {
    const result = await window.cogbridge.doSomething();
    console.log('Success:', result);
} catch (error) {
    console.error('Failed:', error);
}
```

## Executing JavaScript from C

```c
// Asynchronous
void on_result(CogBridge *bridge, const char *name,
               const char *result_json, void *user_data) {
    printf("Result: %s\n", result_json);
}

cogbridge_execute_script(bridge, "document.title = 'New'; 42;",
                        on_result, NULL);

// Synchronous (careful with this)
GError *error = NULL;
char *result = cogbridge_execute_script_sync(bridge, "2 + 2", &error);
if (result) {
    printf("Result: %s\n", result);
    g_free(result);
}
```

## Event System

### C Side (Emit)

```c
// Emit event with JSON data
const char *json = "{\"temperature\":25.5,\"unit\":\"C\"}";
cogbridge_emit_event(bridge, "sensor_update", json);

// With json-glib
JsonBuilder *builder = json_builder_new();
json_builder_begin_object(builder);
json_builder_set_member_name(builder, "status");
json_builder_add_string_value(builder, "ok");
json_builder_end_object(builder);

JsonNode *root = json_builder_get_root(builder);
JsonGenerator *gen = json_generator_new();
json_generator_set_root(gen, root);
char *json = json_generator_to_data(gen, NULL);

cogbridge_emit_event(bridge, "status_change", json);

g_free(json);
g_object_unref(gen);
g_object_unref(builder);
json_node_free(root);
```

### JavaScript Side (Listen)

```javascript
// Register listener
window.cogbridge.on('sensor_update', function(data) {
    console.log('Temperature:', data.temperature, data.unit);
});

// Multiple listeners OK
window.cogbridge.on('status_change', handleStatus);
window.cogbridge.on('status_change', logStatus);
```

## JSON Parsing in C

```c
#include <json-glib/json-glib.h>

// Parse arguments
JsonParser *parser = json_parser_new();
if (!json_parser_load_from_data(parser, args_json, -1, NULL)) {
    g_object_unref(parser);
    return g_strdup("{\"error\":\"Invalid JSON\"}");
}

JsonNode *root = json_parser_get_root(parser);
JsonArray *array = json_node_get_array(root);

// Get elements
if (json_array_get_length(array) > 0) {
    const char *str = json_array_get_string_element(array, 0);
    gint64 num = json_array_get_int_element(array, 1);
    double dbl = json_array_get_double_element(array, 2);
    gboolean bool_val = json_array_get_boolean_element(array, 3);
}

g_object_unref(parser);
```

## Event Loop

```c
// Run event loop (blocks until quit)
cogbridge_run(bridge);

// Quit from callback or signal handler
cogbridge_quit(bridge);

// Timer example
gboolean on_timeout(gpointer user_data) {
    CogBridge *bridge = (CogBridge *)user_data;
    // Do periodic work
    return G_SOURCE_CONTINUE; // or G_SOURCE_REMOVE
}

g_timeout_add_seconds(1, on_timeout, bridge);
```

## Thread Safety

```c
// From worker thread - use g_idle_add
void *worker_thread(void *arg) {
    CogBridge *bridge = (CogBridge *)arg;
    
    // Do work...
    Result *result = do_work();
    
    // Marshal back to main thread
    g_idle_add((GSourceFunc)update_ui, result);
    
    return NULL;
}

// Runs on main thread
gboolean update_ui(gpointer data) {
    Result *result = (Result *)data;
    
    // Safe to call CogBridge API here
    char *script = g_strdup_printf("updateValue(%d)", result->value);
    cogbridge_execute_script(bridge, script, NULL, NULL);
    g_free(script);
    
    free_result(result);
    return G_SOURCE_REMOVE;
}
```

## Console Handler

```c
char *console_handler(CogBridge *bridge, const char *name,
                      const char *json, void *user_data) {
    // json format: {"level":"ERROR","message":"...","source":"...","line":42}
    printf("Console: %s\n", json);
    return NULL;
}

cogbridge_set_console_handler(bridge, console_handler, NULL);
```

## Cleanup

```c
// Free bridge
cogbridge_free(bridge);

// Cleanup library (before exit)
cogbridge_cleanup();
```

## Common Patterns

### Reading Sensor

```c
static char *read_sensor(CogBridge *bridge, const char *fn,
                         const char *args, void *user_data) {
    Sensor *sensor = (Sensor *)user_data;
    double value = sensor_read(sensor);
    return g_strdup_printf("{\"value\":%.2f,\"unit\":\"C\"}", value);
}
```

### Controlling Hardware

```c
static char *set_led(CogBridge *bridge, const char *fn,
                     const char *args, void *user_data) {
    // Parse: ["3", true] -> pin 3, state true
    JsonParser *parser = json_parser_new();
    json_parser_load_from_data(parser, args, -1, NULL);
    JsonArray *array = json_node_get_array(json_parser_get_root(parser));
    
    int pin = json_array_get_int_element(array, 0);
    gboolean state = json_array_get_boolean_element(array, 1);
    
    g_object_unref(parser);
    
    // Validate
    if (pin < 0 || pin > 7) {
        return g_strdup("{\"error\":\"Invalid pin\"}");
    }
    
    // Control hardware
    gpio_set_value(pin, state ? 1 : 0);
    
    return g_strdup("{\"success\":true}");
}
```

### Periodic Updates

```c
static gboolean update_timer(gpointer user_data) {
    CogBridge *bridge = (CogBridge *)user_data;
    
    // Read sensor
    double temp = read_temperature();
    
    // Send to UI
    char *json = g_strdup_printf("{\"temperature\":%.2f}", temp);
    cogbridge_emit_event(bridge, "sensor_update", json);
    g_free(json);
    
    return G_SOURCE_CONTINUE;
}

// Setup timer (in main)
g_timeout_add_seconds(1, update_timer, bridge);
```

## Build & Link

### Using pkg-config

```bash
gcc -o myapp myapp.c $(pkg-config --cflags --libs cogbridge)
```

### Manual

```bash
gcc -o myapp myapp.c \
    -I/usr/include/cog \
    -I/usr/include/cogbridge \
    -lcogbridge -lcogcore \
    $(pkg-config --cflags --libs wpe-webkit-1.0 glib-2.0 json-glib-1.0)
```

### Meson

```meson
cogbridge_dep = dependency('cogbridge')

executable('myapp',
    'myapp.c',
    dependencies: [cogbridge_dep],
)
```

## Error Handling

```c
GError *error = NULL;

if (!cogbridge_init(&config, &error)) {
    fprintf(stderr, "Init failed: %s\n", error->message);
    g_error_free(error);
    return 1;
}

// Check return values
if (!cogbridge_wait_ready(bridge, 5000)) {
    fprintf(stderr, "Timeout waiting for page\n");
}

// Validate inputs
if (value < MIN || value > MAX) {
    return g_strdup("{\"error\":\"Value out of range\"}");
}
```

## Debugging Tips

```c
// Enable console output
config.enable_console = true;
config.enable_developer_extras = true;

// Custom console handler for debugging
char *debug_console(CogBridge *bridge, const char *name,
                    const char *json, void *data) {
    g_print("[JS] %s\n", json);
    return NULL;
}
cogbridge_set_console_handler(bridge, debug_console, NULL);

// Log in JavaScript
console.log('Debug:', value);
console.error('Error:', error);
```

## Memory Management

```c
// Always free strings returned by CogBridge
char *result = cogbridge_execute_script_sync(bridge, script, NULL);
if (result) {
    process(result);
    g_free(result); // Don't forget!
}

// Callbacks must return allocated strings
static char *callback(...) {
    return g_strdup("result"); // Caller frees
}

// Use GLib allocators
char *str = g_strdup("hello");
char *fmt = g_strdup_printf("value: %d", 42);
void *mem = g_malloc(size);
g_free(str);
g_free(fmt);
g_free(mem);
```

## Best Practices

1. **Validate all inputs** from JavaScript
2. **Check return values** and handle errors
3. **Free allocated memory** (use g_free for GLib allocations)
4. **Use json-glib** for complex JSON handling
5. **Marshal to main thread** from workers (g_idle_add)
6. **Limit API surface** - only bind necessary functions
7. **Document your API** - JavaScript developers need to know what's available
8. **Test error paths** - what happens with invalid input?
9. **Use timeouts** - don't wait forever (cogbridge_wait_ready)
10. **Profile performance** - monitor CPU and memory usage

## Resources

- Full docs: `README.md`
- Tutorial: `GETTING_STARTED.md`
- Architecture: `OVERVIEW.md`
- Examples: `examples/`
