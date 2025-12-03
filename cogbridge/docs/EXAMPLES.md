# CogBridge Examples

This directory contains streamlined, focused examples demonstrating CogBridge functionality.

## Overview

We provide **two distinct examples** with **no code duplication**:

1. **minimal.c** - Truly minimal example (~180 lines)
2. **dashboard.c** - Advanced embedded system simulation (~540 lines)

Each example serves a specific purpose and demonstrates different aspects of CogBridge.

---

## Build Script and Platform Selection

CogBridge examples use **compile-time platform selection** - each example is built as a separate executable for each platform. No environment variables or wrapper scripts needed!

### How It Works

When you build examples, the build system creates platform-specific binaries:
- `cogbridge-minimal-drm` - Direct framebuffer rendering
- `cogbridge-minimal-headless` - No visual output (testing)
- `cogbridge-minimal-gtk4` - GTK4 desktop window

Each binary has the platform and module directory **compiled in**, so they're truly standalone executables.

### `build_examples.sh` - Build Script

Builds CogBridge examples for one or all platforms (DRM, Headless, GTK4).

**Usage:**
```bash
# Build for all platforms
./build_examples.sh

# Build for specific platform(s)
./build_examples.sh drm
./build_examples.sh headless
./build_examples.sh gtk4
./build_examples.sh drm headless  # Multiple platforms
```

**What it does:**
- Configures Meson with selected platform(s)
- Compiles platform-specific binaries with compile-time defines:
  - `COGBRIDGE_PLATFORM_DEFAULT` - Platform enum value
  - `COGBRIDGE_MODULE_DIR` - Absolute path to platform module
- Creates standalone executables in `build/cogbridge/examples/`

**Example output:**
```bash
Built platforms: drm headless

To run examples:
  /path/to/cog/build/cogbridge/examples/cogbridge-minimal-drm
  /path/to/cog/build/cogbridge/examples/cogbridge-dashboard-drm
  /path/to/cog/build/cogbridge/examples/cogbridge-minimal-headless
  /path/to/cog/build/cogbridge/examples/cogbridge-dashboard-headless

Examples are standalone executables - no environment variables needed!
```

### Quick Start Workflow

```bash
# 1. Build all platforms
cd cogbridge/examples
./build_examples.sh

# 2. Run examples directly - no scripts needed!
../build/cogbridge/examples/cogbridge-minimal-drm       # Framebuffer (may need sudo)
../build/cogbridge/examples/cogbridge-dashboard-gtk4    # Desktop window
../build/cogbridge/examples/cogbridge-minimal-headless  # No display (testing)
```

### Platform-specific notes

- **DRM:** May require `sudo` or being in the `video` group; should be run outside X11/Wayland
- **Headless:** No visual output, useful for testing/CI
- **GTK4:** Requires running X11 or Wayland display server

### Why This Approach?

**Old way (environment variables):**
```bash
COG_MODULEDIR=./build/platform/drm ./build/cogbridge/examples/cogbridge-minimal
```

**New way (standalone executables):**
```bash
./build/cogbridge/examples/cogbridge-minimal-drm
```

Benefits:
- ✅ No environment variables needed
- ✅ No wrapper scripts required
- ✅ Platform selected at compile time (clear and explicit)
- ✅ Module paths compiled in (always correct)
- ✅ True standalone executables

---

## Examples

### 1. Minimal Example (`minimal.c`)

**Purpose:** Demonstrate the absolute essentials of CogBridge in the simplest way possible.

**What it shows:**
- Basic initialization and configuration
- Loading HTML content
- Binding C functions to JavaScript
- Calling C functions from JavaScript with arguments
- Emitting events from C to JavaScript
- Running the event loop

**What it does NOT include:**
- Complex UI styling
- Real-world hardware simulation
- Multiple data types
- Periodic updates
- State management

**Features demonstrated:**
```c
// Function binding (JavaScript → C)
cogbridge_bind_function(bridge, "add", on_add, NULL, NULL);
cogbridge_bind_function(bridge, "greet", on_greet, NULL, NULL);

// Event emission (C → JavaScript)
cogbridge_emit_event(bridge, "notification", "{\"message\":\"Event from C!\"}");
```

**JavaScript API:**
```javascript
// Call C function
const result = await window.cogbridge.add(5, 7);

// Listen for events
window.cogbridge.on('notification', (data) => {
    console.log(data.message);
});
```

**Build:**
```bash
cd cogbridge/examples
./build_examples.sh drm
```

**Run:**
```bash
sudo ./build/cogbridge/examples/cogbridge-minimal-drm
```

**Perfect for:**
- Learning CogBridge basics
- Quick testing
- Starting point for new projects
- Understanding the core API

---

### 2. Dashboard Example (`dashboard.c`)

**Purpose:** Demonstrate a realistic embedded system application with rich UI and hardware interaction.

**What it shows:**
- Complex state management (sensors, LEDs, motor)
- Multiple bound functions (6 different functions)
- Real-time sensor data updates (temperature, humidity, pressure, CPU, memory)
- Hardware control (LED on/off, motor speed)
- Periodic updates via GLib timers
- Rich HTML/CSS dashboard interface
- JSON data interchange for complex structures
- Event-driven architecture

**Simulated Hardware:**
- **Sensors:** Temperature, humidity, pressure, CPU usage, memory usage
- **Controls:** 4 LEDs (red, green, blue, yellow)
- **Motor:** On/off control with variable speed
- **System monitoring:** Real-time system stats

**Features demonstrated:**
```c
// Complex state structure
typedef struct {
    double temperature;
    double humidity;
    double pressure;
    int cpu_usage;
    int memory_usage;
    gboolean led_state[4];
    gboolean motor_running;
    int motor_speed;
} HardwareState;

// Multiple function bindings
cogbridge_bind_function(bridge, "read_sensors", on_read_sensors, &hw_state, NULL);
cogbridge_bind_function(bridge, "set_led", on_set_led, &hw_state, NULL);
cogbridge_bind_function(bridge, "set_motor", on_set_motor, &hw_state, NULL);

// Periodic updates
g_timeout_add_seconds(2, update_sensors_timer, bridge);
```

**JavaScript API:**
```javascript
// Read all sensors
const data = await window.cogbridge.read_sensors();
console.log(`Temp: ${data.temperature}°C`);

// Control hardware
await window.cogbridge.set_led(0, true);  // LED 0 ON
await window.cogbridge.set_motor(true, 75); // Motor 75% speed

// Listen for updates
window.cogbridge.on('sensor_update', (data) => {
    updateDashboard(data);
});
```

**Build:**
```bash
cd cogbridge/examples
./build_examples.sh drm
```

**Run:**
```bash
sudo ./build/cogbridge/examples/cogbridge-dashboard-drm
```

**Perfect for:**
- Understanding real-world embedded applications
- Learning state management patterns
- Seeing rich UI possibilities
- Hardware integration examples
- Dashboard/HMI inspiration

---

## Comparison Table

| Aspect | Minimal | Dashboard |
|--------|---------|-----------|
| **Lines of code** | ~180 | ~540 |
| **Complexity** | Very simple | Realistic |
| **Bound functions** | 3 | 6 |
| **HTML/CSS** | Basic | Rich dashboard |
| **State management** | None | Complex structure |
| **Periodic updates** | No | Yes (2-second timer) |
| **Hardware simulation** | No | Yes (sensors, LEDs, motor) |
| **Best for** | Learning basics | Real-world inspiration |

---

## What Was Removed

To eliminate redundancy, we removed:

### ❌ `minimal_gui.c` (deleted)
**Reason:** Identical functionality to `minimal.c`. Platform selection can be done via:
```c
config.platform_name = "gtk4";  // Desktop window
```
No need for separate file.

### ❌ `platform-switch.c` (deleted)
**Reason:** Platform selection is now done at compile time via the build system. No need for 400+ lines demonstrating runtime selection. Each example is built as a separate binary per platform.

### What we kept
- **Distinct functionality** - Each example shows different things
- **No code duplication** - No repeated `on_add`, `on_greet` functions
- **Clear purpose** - Each example has a specific learning goal

---

## Common Patterns

### Platform Selection (compile-time)

Platform is selected at **build time** via the build system, not at runtime. Each binary is built for a specific platform.

In your code, the platform is automatically configured:
```c
CogBridgeConfig config;
cogbridge_get_default_config(&config);

// Platform is set at compile time via build system
#ifdef COGBRIDGE_PLATFORM_DEFAULT
config.platform = COGBRIDGE_PLATFORM_DEFAULT;
#endif
#ifdef COGBRIDGE_MODULE_DIR
config.module_dir = COGBRIDGE_MODULE_DIR;
#endif

cogbridge_init(&config, NULL);
```

The build system defines these for you based on which platform you're building.

### Function Binding Pattern

```c
static char *
my_function(CogBridge *bridge, const char *fn, const char *args_json, void *user_data) {
    // Parse JSON args
    JsonParser *parser = json_parser_new();
    json_parser_load_from_data(parser, args_json, -1, NULL);
    JsonArray *array = json_node_get_array(json_parser_get_root(parser));
    
    // Get arguments
    int arg1 = json_array_get_int_element(array, 0);
    
    g_object_unref(parser);
    
    // Do work, return JSON
    return g_strdup_printf("%d", result);
}

// Bind it
cogbridge_bind_function(bridge, "my_function", my_function, user_data, NULL);
```

### Event Emission Pattern

```c
// From C
cogbridge_emit_event(bridge, "event_name", "{\"key\":\"value\"}");

// In JavaScript
window.cogbridge.on('event_name', (data) => {
    console.log(data.key);
});
```

---

## Building and Running

### Using the build script (recommended)

```bash
# Build for all platforms
cd cogbridge/examples
./build_examples.sh

# Build for specific platform(s)
./build_examples.sh drm
./build_examples.sh drm headless gtk4

# Run examples directly (standalone executables)
./build/cogbridge/examples/cogbridge-minimal-drm
./build/cogbridge/examples/cogbridge-dashboard-headless
./build/cogbridge/examples/cogbridge-minimal-gtk4
```

See the [Build Script and Platform Selection](#build-script-and-platform-selection) section above for details.

### Manual build (if needed)

```bash
# Build manually
cd cogbridge/examples
meson setup build -Dplatforms=drm,headless,gtk4 -Dexamples=true -Dprograms=false
ninja -C build

# Run the platform-specific binaries
sudo ./build/cogbridge/examples/cogbridge-minimal-drm
./build/cogbridge/examples/cogbridge-minimal-headless
./build/cogbridge/examples/cogbridge-minimal-gtk4
```

**Note:** No environment variables needed! Each binary has its platform and module directory compiled in.

---

## Next Steps

After exploring these examples:

1. **Start with minimal.c** to understand the basics
2. **Study dashboard.c** to see realistic patterns
3. **Read the API docs:** [QUICK_REFERENCE.md](QUICK_REFERENCE.md)
4. **Build your own application** using these as templates

---

## Example Selection Guide

**Choose minimal.c if you want to:**
- ✅ Learn CogBridge basics quickly
- ✅ Understand core API in < 200 lines
- ✅ Start a new simple project
- ✅ Test CogBridge installation

**Choose dashboard.c if you want to:**
- ✅ See realistic embedded system patterns
- ✅ Learn state management
- ✅ Build an HMI or dashboard
- ✅ Understand hardware integration
- ✅ Get UI/UX inspiration

**Don't look for:**
- ❌ Platform switching examples (platform is selected at build time)
- ❌ GUI vs non-GUI examples (same source code, different binaries per platform)
- ❌ Multiple minimal examples (we have one, truly minimal)
- ❌ Environment variable setup scripts (not needed - everything compiled in)

---

## Code Statistics

```
Total examples:     2 files
Total lines:        718 lines
Average per file:   359 lines
Code duplication:   0% (no shared function implementations)
```

**Before streamlining:** 4 examples, 1710 lines, significant duplication

**After streamlining:** 2 examples, 718 lines, zero duplication

**Savings:** 58% reduction in code, 100% elimination of redundancy

---

## Contributing

If you want to add a new example:

1. **Ensure it's truly distinct** - Does it show something neither existing example covers?
2. **Keep it focused** - One clear purpose, no feature creep
3. **No duplication** - Don't copy existing functions unless absolutely necessary
4. **Document well** - Clear header comments explaining what it demonstrates
5. **Update this file** - Add your example to the table above

---

**Questions?** See [FAQ.md](FAQ.md) or [QUICK_REFERENCE.md](QUICK_REFERENCE.md)