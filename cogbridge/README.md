# CogBridge

**An embedded WebView framework for C based on WebKit WPE**

CogBridge is a lightweight library that enables bidirectional communication between C code and JavaScript in embedded systems. Built on top of the [Cog](https://github.com/Igalia/cog) WebKit WPE launcher, CogBridge provides a simple API similar to [Tauri](https://tauri.app/) and [Saucer](https://github.com/saucer/saucer) but specifically designed for embedded Linux environments without X11 or Wayland dependencies.

> **📺 IMPORTANT: GUI Display Support**
>
> **CogBridge DOES show GUI on displays!** Use the **DRM platform** for direct framebuffer rendering (no X11/Wayland needed).
>
> - **DRM platform** = Full GUI on display via framebuffer (embedded systems) ← **Use this for embedded!**
> - **Headless platform** = No visual output (testing/automation only)
> - **GTK4 platform** = Desktop window (development only)
>
> 📖 **See [PLATFORM_GUIDE.md](PLATFORM_GUIDE.md) for detailed explanation**

## Quick Links

**Common Tasks:**
- 🔨 **[Build with both DRM & GTK4](DUAL_PLATFORM_GUIDE.md)** - Develop on desktop, test embedded mode
- 🚀 **[Quick Reference Card](QUICK_PLATFORM_REFERENCE.md)** - All commands in one place
- 🐛 **[DRM Troubleshooting](DRM_TROUBLESHOOTING.md)** - Fix permissions, device access issues
- 📖 **[Complete Platform Guide](PLATFORM_GUIDE.md)** - Which platform to choose?

**Getting Started:**
1. **[Install dependencies & build](BUILD_AND_RUN.md#build-instructions)**
2. **[Run examples](BUILD_AND_RUN.md#running-the-examples)**
3. **[Write your first app](GETTING_STARTED.md)**

## Features

- 🚀 **No X11/Wayland Required** - Direct framebuffer rendering via DRM/KMS, or truly headless operation
- 🔄 **Bidirectional Communication** - Call C functions from JavaScript and vice versa
- 📡 **Event System** - Emit events from C to JavaScript listeners
- 🎯 **Simple API** - Easy-to-use C API with minimal boilerplate
- ⚡ **Lightweight** - Built on WebKit WPE, optimized for embedded systems
- 🔧 **Hardware Integration** - Perfect for IoT dashboards, kiosks, and embedded UIs
- 📦 **JSON-based** - Simple JSON data interchange between C and JavaScript
- 🛡️ **Type-safe** - Compile-time type checking for bound functions
- 🖥️ **Multiple Display Modes** - DRM (framebuffer), headless (no display), or GTK4 for development

## Use Cases

CogBridge is ideal for:

- **Embedded System Dashboards** - Create rich web-based interfaces for hardware monitoring
- **IoT Devices** - Build interactive control panels for IoT applications
- **Industrial HMI** - Develop human-machine interfaces with web technologies
- **Digital Signage** - Dynamic content with hardware integration
- **Kiosk Applications** - Interactive kiosks with custom hardware control
- **Test Equipment** - Web-based interfaces for test and measurement devices

## Architecture

```
┌─────────────────────────────────────────┐
│         JavaScript (WebView)            │
│  ┌───────────────────────────────────┐  │
│  │  window.cogbridge.functionName()  │  │
│  │  window.cogbridge.on('event',fn)  │  │
│  └───────────────────────────────────┘  │
└──────────────┬──────────────────────────┘
               │ Message Passing
               │ (WebKit User Messages)
┌──────────────▼──────────────────────────┐
│         CogBridge Library (C)           │
│  ┌───────────────────────────────────┐  │
│  │  cogbridge_bind_function()        │  │
│  │  cogbridge_execute_script()       │  │
│  │  cogbridge_emit_event()           │  │
│  └───────────────────────────────────┘  │
└──────────────┬──────────────────────────┘
               │
┌──────────────▼──────────────────────────┐
│      Your C Application Code           │
│  (Hardware control, sensor reading,     │
│   business logic, etc.)                 │
└─────────────────────────────────────────┘
```

## Platform Support

CogBridge supports multiple display platforms. **For embedded systems with displays, use DRM platform!**

📖 **Complete guide:** [PLATFORM_GUIDE.md](PLATFORM_GUIDE.md) | **Troubleshooting:** [DRM_TROUBLESHOOTING.md](DRM_TROUBLESHOOTING.md)

### DRM Platform (Recommended for Embedded) ✅ SHOWS GUI
**Direct framebuffer rendering without X11/Wayland** - Shows GUI directly on display hardware.

> **✅ Raspberry Pi 5 Fully Supported!**
> DRM platform works perfectly on Raspberry Pi 5 with automatic shadow buffer conversion for the VC7 GPU.
> See [DRM_FRAMEBUFFER_ISSUE.md](docs/DRM_FRAMEBUFFER_ISSUE.md) for technical details.

**Tested Hardware:**
- ✅ Raspberry Pi 5 (VC7 GPU) - Production ready
- ✅ Raspberry Pi 4 (VC6 GPU) - Expected to work
- ✅ Intel/AMD/ARM Mali GPUs with DRM support

**Dependencies:**
- **WPE WebKit** >= 2.28.0 (2.0, 1.1, or 1.0 API)
- **WPE backend FDO** >= 1.14.0
- **GLib** >= 2.44
- **json-glib** >= 1.0
- **libdrm** >= 2.4.71
- **libinput**
- **libudev**
- **libgbm** (usually part of Mesa)
- **libepoxy**

### Headless Platform
**No display output** - For server-side rendering, testing, or backend processing.

**Dependencies:**
- **WPE WebKit** >= 2.28.0
- **WPE backend FDO** >= 1.14.0
- **GLib** >= 2.44
- **json-glib** >= 1.0

### GTK4 Platform (Development Only)
**Traditional GUI** - Requires full desktop environment with X11 or Wayland.

**Dependencies:** All of the above plus GTK4, libportal, and desktop libraries.

## Building

CogBridge uses the Meson build system. Choose the platform based on your needs:

### For Embedded Systems (DRM - Visual Output)

```bash
# Configure with DRM platform for direct framebuffer rendering
meson setup build \
  -Dexamples=true \
  -Dplatforms=drm \
  -Dprograms=false \
  -Dlibportal=disabled

# Build
ninja -C build

# Install
sudo ninja -C build install
```

### For Headless Operation (No Display)

```bash
# Configure with headless platform (no visual output)
meson setup build \
  -Dexamples=true \
  -Dplatforms=headless \
  -Dprograms=false \
  -Dlibportal=disabled

# Build
ninja -C build
```

### For Development (GTK4 - Desktop GUI)

```bash
# Configure with GTK4 platform (requires desktop environment)
meson setup build \
  -Dexamples=true \
  -Dplatforms=gtk4,drm,headless \
  -Dprograms=false

# Build
ninja -C build
```

### Build Options

- `-Dexamples=true` - Build example programs
- `-Dplatforms=drm,headless,gtk4` - Comma-separated list of platforms to build
- `-Dwpe_api=2.0` - Specify WPE WebKit API version (2.0, 1.1, or 1.0)
- `-Dprograms=false` - Don't build the cog launcher binary
- `-Dlibportal=disabled` - Disable portal support (not needed for embedded)
- `--prefix=/usr/local` - Custom installation prefix

## Documentation

### Platform Selection & Setup
- **[PLATFORM_GUIDE.md](docs/PLATFORM_GUIDE.md)** - Complete guide: DRM vs Headless vs GTK4
- **[DUAL_PLATFORM_GUIDE.md](docs/DUAL_PLATFORM_GUIDE.md)** - Build & test with both DRM and GTK4
- **[QUICK_PLATFORM_REFERENCE.md](docs/QUICK_PLATFORM_REFERENCE.md)** - Quick reference card with all commands
- **[DRM_TROUBLESHOOTING.md](docs/DRM_TROUBLESHOOTING.md)** - DRM platform troubleshooting
- **[DRM_FRAMEBUFFER_ISSUE.md](docs/DRM_FRAMEBUFFER_ISSUE.md)** - Raspberry Pi 5 framebuffer fix (RESOLVED)

### Getting Started
- **[GETTING_STARTED.md](docs/GETTING_STARTED.md)** - Step-by-step tutorial
- **[BUILD_AND_RUN.md](docs/BUILD_AND_RUN.md)** - Build instructions and running examples

### Reference
- **[OVERVIEW.md](docs/OVERVIEW.md)** - Architecture and design overview
- **[QUICK_REFERENCE.md](docs/QUICK_REFERENCE.md)** - API reference
- **[FAQ.md](docs/FAQ.md)** - Frequently asked questions
- **[TESTING.md](docs/TESTING.md)** - Testing guide

## Quick Start

### 1. Basic Example

```c
#include <cogbridge/cogbridge.h>

// C function callable from JavaScript
static char *
on_greet(CogBridge *bridge, const char *function_name, 
         const char *args_json, void *user_data)
{
    // Parse arguments from JSON
    // args_json = "[\"World\"]"
    
    return g_strdup("\"Hello, World from C!\"");
}

int main(int argc, char *argv[])
{
    // Initialize CogBridge
    cogbridge_init(NULL, NULL);
    
    // Create bridge instance
    CogBridge *bridge = cogbridge_new("my-app");
    
    // Bind C function to JavaScript
    cogbridge_bind_function(bridge, "greet", on_greet, NULL, NULL);
    
    // Load HTML
    const char *html = 
        "<html><body>"
        "<script>"
        "  window.cogbridge.greet('World').then(result => {"
        "    console.log('Result:', result);"
        "  });"
        "</script>"
        "</body></html>";
    
    cogbridge_load_html(bridge, html, NULL);
    
    // Wait for page to load
    cogbridge_wait_ready(bridge, 5000);
    
    // Run event loop
    cogbridge_run(bridge);
    
    // Cleanup
    cogbridge_free(bridge);
    cogbridge_cleanup();
    
    return 0;
}
```

### 2. Compile and Run

```bash
gcc -o myapp myapp.c `pkg-config --cflags --libs cogbridge`
./myapp
```

## API Reference

### Initialization

```c
// Get default configuration
CogBridgeConfig config;
cogbridge_get_default_config(&config);

// Customize configuration
config.width = 1920;
config.height = 1080;
config.enable_console = true;
config.enable_developer_extras = false;

// Initialize library
GError *error = NULL;
if (!cogbridge_init(&config, &error)) {
    fprintf(stderr, "Init failed: %s\n", error->message);
    g_error_free(error);
    return 1;
}
```

### Creating a Bridge

```c
// Create new bridge instance
CogBridge *bridge = cogbridge_new("my-application");

// Load content
cogbridge_load_uri(bridge, "https://example.com");
// or
cogbridge_load_html(bridge, "<html>...</html>", "file:///");

// Wait for ready
cogbridge_wait_ready(bridge, 10000); // 10 second timeout
```

### Binding C Functions

```c
// Callback signature
typedef char *(*CogBridgeCallbackFunc)(
    CogBridge *bridge,
    const char *function_name,
    const char *args_json,
    void *user_data
);

// Bind function
cogbridge_bind_function(bridge, "myFunction", callback, user_data, destroy_notify);

// Unbind function
cogbridge_unbind_function(bridge, "myFunction");
```

**JavaScript side:**

```javascript
// Call bound function (returns Promise)
const result = await window.cogbridge.myFunction(arg1, arg2, arg3);
```

### Executing JavaScript from C

```c
// Asynchronous execution with callback
void on_script_result(CogBridge *bridge, const char *name, 
                      const char *result, void *user_data) {
    printf("Result: %s\n", result);
}

cogbridge_execute_script(bridge, 
    "document.title = 'New Title'; 42;",
    on_script_result, 
    NULL);

// Synchronous execution
GError *error = NULL;
char *result = cogbridge_execute_script_sync(bridge, "2 + 2", &error);
if (result) {
    printf("Result: %s\n", result);
    g_free(result);
}
```

### Event System

**C side (emit events):**

```c
// Emit event with JSON data
const char *data = "{\"temperature\":25.5,\"unit\":\"celsius\"}";
cogbridge_emit_event(bridge, "sensor_update", data);
```

**JavaScript side (listen for events):**

```javascript
window.cogbridge.on('sensor_update', function(data) {
    console.log('Temperature:', data.temperature, data.unit);
});
```

### Running the Event Loop

```c
// Start event loop (blocking)
cogbridge_run(bridge);

// From another thread or signal handler:
cogbridge_quit(bridge);
```

### Console Message Handling

```c
// Custom console handler
char *console_handler(CogBridge *bridge, const char *name,
                      const char *json, void *user_data) {
    // json contains: {"level":"ERROR","message":"...","source":"...","line":42}
    printf("Console: %s\n", json);
    return NULL;
}

cogbridge_set_console_handler(bridge, console_handler, NULL);
```

### Cleanup

```c
// Free bridge instance
cogbridge_free(bridge);

// Cleanup library
cogbridge_cleanup();
```

## JavaScript API

CogBridge automatically injects a `window.cogbridge` object with the following API:

### Calling C Functions

```javascript
// All bound C functions are available as methods
const result = await window.cogbridge.functionName(arg1, arg2, ...);

// Example:
const sum = await window.cogbridge.add(10, 20);
console.log('Sum:', sum); // 30
```

### Listening for Events

```javascript
// Register event listener
window.cogbridge.on('event_name', function(data) {
    console.log('Received:', data);
});

// Multiple listeners supported
window.cogbridge.on('sensor_data', handleSensorData);
window.cogbridge.on('sensor_data', logSensorData);
```

## JSON Data Interchange

All data between C and JavaScript is exchanged as JSON:

### JavaScript → C

Arguments are automatically serialized to JSON array:

```javascript
window.cogbridge.myFunction("hello", 42, true, {key: "value"})
```

C receives:
```json
["hello", 42, true, {"key":"value"}]
```

### C → JavaScript

Return JSON strings from C callbacks:

```c
// Return primitive
return g_strdup("42");
return g_strdup("\"string value\"");
return g_strdup("true");

// Return object
return g_strdup("{\"key\":\"value\",\"number\":123}");

// Return array
return g_strdup("[1,2,3,4,5]");

// Return null
return g_strdup("null");
```

Use **json-glib** for complex JSON handling:

```c
#include <json-glib/json-glib.h>

static char *
my_callback(CogBridge *bridge, const char *fn, 
            const char *args_json, void *data)
{
    // Parse arguments
    JsonParser *parser = json_parser_new();
    json_parser_load_from_data(parser, args_json, -1, NULL);
    JsonNode *root = json_parser_get_root(parser);
    JsonArray *array = json_node_get_array(root);
    
    // Get first argument
    const char *name = json_array_get_string_element(array, 0);
    
    g_object_unref(parser);
    
    // Build response
    JsonBuilder *builder = json_builder_new();
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "greeting");
    json_builder_add_string_value(builder, "Hello");
    json_builder_set_member_name(builder, "name");
    json_builder_add_string_value(builder, name);
    json_builder_end_object(builder);
    
    JsonNode *result = json_builder_get_root(builder);
    JsonGenerator *gen = json_generator_new();
    json_generator_set_root(gen, result);
    char *json = json_generator_to_data(gen, NULL);
    
    json_node_free(result);
    g_object_unref(gen);
    g_object_unref(builder);
    
    return json;
}
```

## Examples

### Minimal Example

A basic example demonstrating core functionality:

```bash
# Build and run
cd cog
meson setup build -Dexamples=true
ninja -C build
./build/cogbridge/examples/cogbridge-minimal
```

### Embedded Dashboard

A comprehensive example simulating an embedded system dashboard:

```bash
./build/cogbridge/examples/cogbridge-dashboard
```

Features demonstrated:
- Real-time sensor reading (temperature, humidity, pressure)
- System resource monitoring (CPU, memory)
- Hardware control (LED toggle, motor control)
- Event emission and handling
- Rich web-based UI

## Best Practices

### 1. Error Handling

Always check return values and handle errors:

```c
GError *error = NULL;
if (!cogbridge_init(&config, &error)) {
    fprintf(stderr, "Error: %s\n", error->message);
    g_error_free(error);
    return 1;
}
```

### 2. Memory Management

Free returned strings:

```c
char *result = cogbridge_execute_script_sync(bridge, script, NULL);
if (result) {
    process_result(result);
    g_free(result);  // Don't forget!
}
```

Return allocated strings from callbacks:

```c
static char *my_callback(...) {
    return g_strdup("result");  // Caller will free
}
```

### 3. Thread Safety

CogBridge uses GLib's main loop. For thread-safe operation:

```c
// From worker thread, use g_idle_add
void *worker_thread(void *data) {
    CogBridge *bridge = (CogBridge *)data;
    
    // Do work...
    
    // Update UI from main thread
    g_idle_add((GSourceFunc)update_ui, result_data);
    
    return NULL;
}
```

### 4. Validation

Always validate arguments from JavaScript:

```c
static char *on_set_speed(CogBridge *bridge, const char *fn,
                          const char *args_json, void *data)
{
    JsonParser *parser = json_parser_new();
    if (!json_parser_load_from_data(parser, args_json, -1, NULL)) {
        g_object_unref(parser);
        return g_strdup("{\"error\":\"Invalid JSON\"}");
    }
    
    JsonArray *array = json_node_get_array(json_parser_get_root(parser));
    if (json_array_get_length(array) < 1) {
        g_object_unref(parser);
        return g_strdup("{\"error\":\"Missing argument\"}");
    }
    
    int speed = json_array_get_int_element(array, 0);
    g_object_unref(parser);
    
    // Validate range
    if (speed < 0 || speed > 100) {
        return g_strdup("{\"error\":\"Speed out of range\"}");
    }
    
    // Process...
    set_motor_speed(speed);
    
    return g_strdup("{\"success\":true}");
}
```

## Platform Support

CogBridge is designed for embedded Linux systems with:

- **ARM/ARM64** - Raspberry Pi, BeagleBone, i.MX6/7/8, etc.
- **x86/x86_64** - Industrial PCs, embedded x86 systems
- **MIPS** - Router platforms, network devices

Tested on:
- Raspberry Pi 3/4
- BeagleBone Black
- i.MX6 Quad
- Generic x86_64 Linux

## Performance Considerations

### Memory Usage

CogBridge is designed for resource-constrained systems:

- Minimal overhead over base WebKit WPE
- ~50-100 KB additional memory for bridge infrastructure
- Scales with number of bound functions and active web content

### Optimization Tips

1. **Minimize JavaScript execution frequency** - Batch updates when possible
2. **Use events for high-frequency data** - More efficient than polling
3. **Cache DOM references** - Store frequently accessed elements
4. **Lazy load resources** - Only load what's needed
5. **Use CSS animations** - More efficient than JavaScript animations

## Troubleshooting

### Page Not Loading

```c
// Increase timeout
if (!cogbridge_wait_ready(bridge, 30000)) {  // 30 seconds
    fprintf(stderr, "Timeout\n");
}

// Enable console output
config.enable_console = true;
```

### Function Not Found

Check that functions are bound *before* loading content:

```c
cogbridge_bind_function(bridge, "myFunc", callback, NULL, NULL);
cogbridge_load_html(bridge, html, NULL);  // myFunc now available
```

### JSON Parse Errors

Validate JSON strings:

```bash
# Use json-glib's validation
JsonParser *parser = json_parser_new();
GError *error = NULL;
if (!json_parser_load_from_data(parser, json_str, -1, &error)) {
    g_warning("JSON error: %s", error->message);
    g_error_free(error);
}
g_object_unref(parser);
```

## License

CogBridge is licensed under the MIT License. See [COPYING](../COPYING) for details.

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## Acknowledgments

CogBridge is built on top of:

- [Cog](https://github.com/Igalia/cog) - WebKit WPE launcher by Igalia
- [WebKit](https://webkit.org/) - The WebKit browser engine
- [WPE](https://wpewebkit.org/) - WebKit for embedded platforms

Inspired by:
- [Tauri](https://tauri.app/) - Desktop application framework
- [Saucer](https://github.com/saucer/saucer) - Cross-platform webview library

## Contact

For questions, issues, or feature requests, please open an issue on GitHub.

---

**CogBridge** - Bridging the gap between embedded hardware and modern web UIs. 🌉