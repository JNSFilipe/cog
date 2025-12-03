# Getting Started with CogBridge

This guide will walk you through creating your first CogBridge application in 5 minutes.

## Prerequisites

Before you begin, ensure you have:

- A Linux-based system (embedded or desktop)
- Build tools: `gcc`, `meson`, `ninja`
- Dependencies installed (see below)

### Installing Dependencies

#### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    meson ninja-build \
    libwpewebkit-1.0-dev \
    libwpe-1.0-dev \
    libglib2.0-dev \
    libjson-glib-dev
```

#### Fedora/RHEL

```bash
sudo dnf install -y \
    gcc meson ninja-build \
    wpewebkit-devel \
    wpe-devel \
    glib2-devel \
    json-glib-devel
```

#### Buildroot/Yocto (Embedded Systems)

Add the following to your configuration:
- `wpewebkit`
- `wpe`
- `libglib2`
- `json-glib`

## Step 1: Build CogBridge

Clone and build the Cog repository with CogBridge:

```bash
# Clone the repository
git clone https://github.com/Igalia/cog.git
cd cog

# Configure with CogBridge examples enabled
meson setup build -Dexamples=true

# Build
ninja -C build

# Optional: Install system-wide
sudo ninja -C build install
```

## Step 2: Run the Examples

Try the minimal example first:

```bash
./build/cogbridge/examples/cogbridge-minimal
```

You should see output showing the initialization, function binding, and page loading.

Try the dashboard example:

```bash
./build/cogbridge/examples/cogbridge-dashboard
```

This shows a more complete embedded system dashboard with sensors, LEDs, and motor control.

## Step 3: Create Your First Application

Create a new file `hello_cogbridge.c`:

```c
#include <cogbridge/cogbridge.h>
#include <stdio.h>

// C function that will be called from JavaScript
static char *
say_hello(CogBridge *bridge, const char *function_name,
          const char *args_json, void *user_data)
{
    printf("[C] say_hello() called from JavaScript!\n");
    return g_strdup("\"Hello from C!\"");
}

int main(int argc, char *argv[])
{
    GError *error = NULL;
    
    // Initialize CogBridge with defaults
    if (!cogbridge_init(NULL, &error)) {
        fprintf(stderr, "Failed to initialize: %s\n", error->message);
        return 1;
    }
    
    // Create a bridge instance
    CogBridge *bridge = cogbridge_new("hello-app");
    
    // Bind the C function to JavaScript
    cogbridge_bind_function(bridge, "say_hello", say_hello, NULL, NULL);
    
    // HTML with JavaScript that calls our C function
    const char *html = 
        "<!DOCTYPE html>"
        "<html>"
        "<head><title>Hello CogBridge</title></head>"
        "<body>"
        "  <h1>Hello CogBridge!</h1>"
        "  <button onclick='callC()'>Call C Function</button>"
        "  <div id='result'></div>"
        "  <script>"
        "    async function callC() {"
        "      const result = await window.cogbridge.say_hello();"
        "      document.getElementById('result').textContent = result;"
        "      console.log('Result from C:', result);"
        "    }"
        "    // Call automatically on load"
        "    window.onload = callC;"
        "  </script>"
        "</body>"
        "</html>";
    
    // Load the HTML
    cogbridge_load_html(bridge, html, NULL);
    
    // Wait for page to load
    if (!cogbridge_wait_ready(bridge, 5000)) {
        fprintf(stderr, "Timeout waiting for page\n");
        return 1;
    }
    
    printf("Page loaded! Check the output above.\n");
    printf("Press Ctrl+C to exit (in 5 seconds)...\n");
    
    // Run for 5 seconds then quit
    g_timeout_add_seconds(5, (GSourceFunc)cogbridge_quit, bridge);
    cogbridge_run(bridge);
    
    // Cleanup
    cogbridge_free(bridge);
    cogbridge_cleanup();
    
    return 0;
}
```

## Step 4: Compile Your Application

If you installed CogBridge system-wide:

```bash
gcc -o hello_cogbridge hello_cogbridge.c \
    $(pkg-config --cflags --libs cogbridge)
```

If building from the build directory:

```bash
gcc -o hello_cogbridge hello_cogbridge.c \
    -I./core \
    -I./cogbridge \
    -L./build/cogbridge \
    -L./build/core \
    -lcogbridge -lcogcore \
    $(pkg-config --cflags --libs wpe-webkit-1.0 wpe-1.0 glib-2.0 json-glib-1.0)
```

## Step 5: Run Your Application

```bash
./hello_cogbridge
```

You should see:

```
CogBridge initialized successfully
CogBridge instance created: hello-app
CogBridge: Bound function: say_hello
CogBridge: Loading HTML content
[C] say_hello() called from JavaScript!
Page loaded! Check the output above.
Press Ctrl+C to exit (in 5 seconds)...
```

## Next Steps

### Add More Functions

Bind multiple C functions:

```c
cogbridge_bind_function(bridge, "add", on_add, NULL, NULL);
cogbridge_bind_function(bridge, "multiply", on_multiply, NULL, NULL);
cogbridge_bind_function(bridge, "read_sensor", on_read_sensor, &sensor_data, NULL);
```

### Handle JSON Arguments

Parse arguments using json-glib:

```c
#include <json-glib/json-glib.h>

static char *
add_numbers(CogBridge *bridge, const char *function_name,
            const char *args_json, void *user_data)
{
    JsonParser *parser = json_parser_new();
    json_parser_load_from_data(parser, args_json, -1, NULL);
    
    JsonNode *root = json_parser_get_root(parser);
    JsonArray *array = json_node_get_array(root);
    
    double a = json_array_get_double_element(array, 0);
    double b = json_array_get_double_element(array, 1);
    
    g_object_unref(parser);
    
    double result = a + b;
    return g_strdup_printf("%g", result);
}
```

JavaScript:

```javascript
const sum = await window.cogbridge.add_numbers(10, 20);
console.log('Sum:', sum); // 30
```

### Emit Events from C

Send data from C to JavaScript:

```c
// In a timer or hardware interrupt handler
void on_sensor_reading(CogBridge *bridge, double temperature) {
    char *json = g_strdup_printf(
        "{\"temperature\":%.2f,\"timestamp\":%ld}",
        temperature, time(NULL));
    
    cogbridge_emit_event(bridge, "sensor_update", json);
    g_free(json);
}
```

JavaScript:

```javascript
window.cogbridge.on('sensor_update', function(data) {
    console.log('Temperature:', data.temperature);
    updateDisplay(data);
});
```

### Execute JavaScript from C

Call JavaScript functions from your C code:

```c
void update_display(CogBridge *bridge, const char *message) {
    char *script = g_strdup_printf(
        "document.getElementById('status').textContent = '%s';",
        message);
    
    cogbridge_execute_script(bridge, script, NULL, NULL);
    g_free(script);
}
```

## Common Patterns

### Hardware Integration

```c
// GPIO control example
static char *
set_gpio(CogBridge *bridge, const char *fn,
         const char *args_json, void *user_data)
{
    // Parse pin and value from args_json
    int pin = ...;
    int value = ...;
    
    // Control actual hardware
    gpio_set_value(pin, value);
    
    return g_strdup("{\"success\":true}");
}

cogbridge_bind_function(bridge, "setGPIO", set_gpio, NULL, NULL);
```

### Periodic Updates

```c
static gboolean
update_timer(gpointer user_data)
{
    CogBridge *bridge = (CogBridge *)user_data;
    
    // Read hardware state
    double value = read_sensor();
    
    // Send to JavaScript
    char *json = g_strdup_printf("{\"value\":%.2f}", value);
    cogbridge_emit_event(bridge, "update", json);
    g_free(json);
    
    return G_SOURCE_CONTINUE; // Keep running
}

// In main():
g_timeout_add_seconds(1, update_timer, bridge);
```

### Loading External Files

```c
// Load from file system
cogbridge_load_uri(bridge, "file:///path/to/index.html");

// Load from web server (if network available)
cogbridge_load_uri(bridge, "http://localhost:8000/dashboard.html");

// Load from data URI
const char *data_uri = "data:text/html,<html><body>Hello</body></html>";
cogbridge_load_uri(bridge, data_uri);
```

## Troubleshooting

### "Failed to get platform" Error

Make sure a WPE backend is available. For headless operation, install `wpebackend-fdo`:

```bash
sudo apt-get install libwpebackend-fdo-1.0-dev
```

### Page Not Loading

Enable console output to see what's happening:

```c
CogBridgeConfig config;
cogbridge_get_default_config(&config);
config.enable_console = true;
cogbridge_init(&config, NULL);
```

### Function Not Callable from JavaScript

Ensure functions are bound *before* loading HTML:

```c
// Correct order:
cogbridge_bind_function(bridge, "myFunc", callback, NULL, NULL);
cogbridge_load_html(bridge, html, NULL);

// Wrong order:
cogbridge_load_html(bridge, html, NULL);
cogbridge_bind_function(bridge, "myFunc", callback, NULL, NULL); // Too late!
```

## Resources

- [Full API Reference](README.md#api-reference)
- [Examples](examples/)
- [Cog Documentation](https://igalia.github.io/cog/)
- [WebKit WPE](https://wpewebkit.org/)

## Support

For questions or issues:
- Check the [README](README.md)
- Look at the [examples](examples/)
- Open an issue on GitHub

Happy coding with CogBridge! 🚀