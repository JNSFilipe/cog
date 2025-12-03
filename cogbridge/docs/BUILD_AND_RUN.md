# CogBridge - Build and Run Guide

## ✅ Successfully Built and Tested!

CogBridge has been successfully built and tested with both the minimal and embedded dashboard examples working perfectly!

## 🎯 Platform Selection Guide

**Choose the right platform for your needs:**

- **DRM** (Recommended for embedded) - Shows GUI on display via direct framebuffer, NO X11/Wayland needed
- **Headless** - No visual output, for testing/automation only
- **GTK4** - Desktop window with X11/Wayland, for development only (large dependencies)

## 🔨 Build Instructions

### Prerequisites for DRM Platform (Recommended)

```bash
# Ubuntu/Debian
sudo apt-get install -y \
    build-essential meson ninja-build \
    libwpewebkit-1.0-dev libwpe-1.0-dev \
    libglib2.0-dev libjson-glib-dev \
    libdrm-dev libgbm-dev libinput-dev \
    libudev-dev libepoxy-dev

# Fedora/RHEL
sudo dnf install -y \
    gcc meson ninja-build \
    wpewebkit-devel wpe-devel \
    glib2-devel json-glib-devel \
    libdrm-devel mesa-libgbm-devel libinput-devel \
    systemd-devel libepoxy-devel
```

### Prerequisites for Headless Platform (No GUI)

```bash
# Ubuntu/Debian
sudo apt-get install -y \
    build-essential meson ninja-build \
    libwpewebkit-1.0-dev libwpe-1.0-dev \
    libglib2.0-dev libjson-glib-dev

# Fedora/RHEL
sudo dnf install -y \
    gcc meson ninja-build \
    wpewebkit-devel wpe-devel \
    glib2-devel json-glib-devel
```

### Build Steps - DRM Platform (Shows GUI on Display)

```bash
# Navigate to the cog directory
cd cog

# Configure the build for DRM (direct framebuffer rendering)
meson setup build \
    -Dexamples=true \
    -Dlibportal=disabled \
    -Dprograms=false \
    -Dplatforms=drm

# Build
ninja -C build

# Verify build succeeded
ls -l build/cogbridge/examples/
```

### Build Steps - Headless Platform (No GUI Output)

```bash
# Navigate to the cog directory
cd cog

# Configure the build for headless (no display)
meson setup build \
    -Dexamples=true \
    -Dlibportal=disabled \
    -Dprograms=false \
    -Dplatforms=headless

# Build
ninja -C build

# Verify build succeeded
ls -l build/cogbridge/examples/
```

### Build Steps - GTK4 Platform (Development Only)

```bash
# Install additional GTK dependencies first
sudo apt-get install -y libgtk-4-dev libportal-gtk4-dev

# Configure the build for GTK4 (desktop window)
meson setup build \
    -Dexamples=true \
    -Dprograms=false \
    -Dplatforms=gtk4,drm,headless

# Build
ninja -C build
```

**Expected output:**
- `cogbridge-minimal` executable
- `cogbridge-dashboard` executable (or `cogbridge-minimal-gui` for GTK4)
- `libcogbridge.so.12.0.0` shared library

## 🚀 Running the Examples

### DRM Platform (Shows GUI on Display)

```bash
# Set the module directory environment variable
export COG_MODULEDIR=./build/platform/drm

# Run examples (must have DRM permissions - see troubleshooting below)
sudo ./build/cogbridge/examples/cogbridge-minimal
sudo ./build/cogbridge/examples/cogbridge-dashboard

# Or add your user to video/input groups (logout required)
sudo usermod -a -G video,input $USER
```

### Headless Platform (No Visual Output)

```bash
# Set the module directory environment variable
export COG_MODULEDIR=./build/platform/headless

# Run minimal example
./build/cogbridge/examples/cogbridge-minimal

# Run dashboard example
./build/cogbridge/examples/cogbridge-dashboard
```

### GTK4 Platform (Desktop Window)

```bash
# Set the module directory environment variable
export COG_MODULEDIR=./build/platform/gtk4

# Run GUI example (requires X11 or Wayland session)
./build/cogbridge/examples/cogbridge-minimal-gui
```

### Using the Helper Script

```bash
# The helper script auto-detects the platform
./cogbridge/examples/run-examples.sh minimal
./cogbridge/examples/run-examples.sh dashboard
./cogbridge/examples/run-examples.sh gui  # if built with GTK4
```

## 📋 Expected Output

### DRM Platform (Visual Output on Display)

When running with DRM platform, you will see:
- **Console logs** showing initialization and function calls
- **GUI displayed on screen** (full-screen web interface)
- The rendered HTML with buttons and interactive elements
- No window decorations (direct framebuffer rendering)

### Headless Platform (No Visual Output)

When running the headless platform, you should see:

```
=== CogBridge Minimal Example ===

** Message: CogBridge initialized successfully
** Message: CogBridge instance created: minimal-example
** Message: CogBridge: Bound function: add
** Message: CogBridge: Bound function: greet
** Message: CogBridge: Bound function: get_system_info
** Message: CogBridge: Bound function: trigger_event
** Message: CogBridge: Loading HTML content
** Message: CogBridge: Page loaded and ready

Binding C functions...
Loading HTML content...
Waiting for page to load...
Page loaded successfully!

Setting up timer for periodic updates...

=== Running event loop ===
[C] Timeout triggered, counter = 1
[C] Timeout triggered, counter = 2
[C] Timeout triggered, counter = 3
[C] Timeout triggered, counter = 4
[C] Timeout triggered, counter = 5
[C] Reached counter limit, quitting in 2 seconds...

=== Cleaning up ===
Done!
```

### Dashboard Example

The dashboard example runs continuously and outputs:

```
=== CogBridge Embedded Dashboard Example ===

** Message: CogBridge initialized successfully
** Message: CogBridge instance created: embedded-dashboard
** Message: CogBridge: Bound function: read_sensors
** Message: CogBridge: Bound function: get_system_stats
** Message: CogBridge: Bound function: toggle_led
** Message: CogBridge: Bound function: toggle_motor
** Message: CogBridge: Bound function: set_motor_speed
** Message: CogBridge: Bound function: emergency_stop
** Message: CogBridge: Loading HTML content
** Message: CogBridge: Page loaded and ready

Binding hardware control functions...
Loading dashboard...
Dashboard loaded successfully!

=== Dashboard is running ===
The dashboard simulates an embedded system with:
  - Sensor readings (temperature, humidity, pressure)
  - System resource monitoring (CPU, memory)
  - LED controls (4 LEDs)
  - Motor control with speed adjustment
  - Real-time event updates

Press Ctrl+C to exit.

[C] Reading sensors: T=25.3°C H=51.2% P=1013.45hPa
[C] System stats: CPU=35% MEM=48%
...
```

## 🔧 Troubleshooting

### Error: "cannot find module 'headless'"

**Solution:** You need to set the `COG_MODULEDIR` environment variable:

```bash
export COG_MODULEDIR=./build/platform/headless
```

Or use the helper script which sets this automatically.

### Error: "libcogbridge.so: cannot open shared object file"

**Solution:** The library path needs to be set:

```bash
# Add build directory to library path
export LD_LIBRARY_PATH=./build/cogbridge:./build/core:$LD_LIBRARY_PATH

# Then run
COG_MODULEDIR=./build/platform/headless ./build/cogbridge/examples/cogbridge-minimal
```

Or use the helper script which handles this.

### Error: "Build directory not found"

**Solution:** Make sure you've built the project first:

```bash
meson setup build -Dexamples=true -Dlibportal=disabled -Dprograms=false -Dplatforms=headless
ninja -C build
```

## 🧪 Testing

### Memory Leak Check

```bash
COG_MODULEDIR=./build/platform/headless \
LD_LIBRARY_PATH=./build/cogbridge:./build/core \
valgrind --leak-check=full \
    --show-leak-kinds=all \
    ./build/cogbridge/examples/cogbridge-minimal
```

### Long-Running Test

```bash
# Run dashboard for 1 hour
COG_MODULEDIR=./build/platform/headless \
timeout 3600 ./build/cogbridge/examples/cogbridge-dashboard
```

### Performance Monitoring

```bash
# Monitor CPU and memory usage
COG_MODULEDIR=./build/platform/headless \
./build/cogbridge/examples/cogbridge-dashboard &

PID=$!
top -p $PID

# Kill when done
kill $PID
```

## 📝 Creating Your Own Application

### Step 1: Create Source File

Create `my_app.c`:

```c
#include <cogbridge/cogbridge.h>

static char *my_function(CogBridge *bridge, const char *fn,
                         const char *args, void *data) {
    return g_strdup("\"Hello from my app!\"");
}

int main(void) {
    cogbridge_init(NULL, NULL);
    CogBridge *bridge = cogbridge_new("my-app");
    
    cogbridge_bind_function(bridge, "myFunction", my_function, NULL, NULL);
    
    const char *html = "<html><body><script>"
        "window.cogbridge.myFunction().then(console.log);"
        "</script></body></html>";
    
    cogbridge_load_html(bridge, html, NULL);
    cogbridge_wait_ready(bridge, 5000);
    
    g_timeout_add_seconds(5, (GSourceFunc)cogbridge_quit, bridge);
    cogbridge_run(bridge);
    
    cogbridge_free(bridge);
    cogbridge_cleanup();
    return 0;
}
```

### Step 2: Compile

```bash
# Using pkg-config (if installed system-wide)
gcc -o my_app my_app.c $(pkg-config --cflags --libs cogbridge)

# Or manually with build directory
gcc -o my_app my_app.c \
    -I./core -I./cogbridge \
    -L./build/cogbridge -L./build/core \
    -lcogbridge -lcogcore \
    $(pkg-config --cflags --libs wpe-webkit-1.1 wpe-1.0 glib-2.0 json-glib-1.0)
```

### Step 3: Run

```bash
# Set paths
export COG_MODULEDIR=./build/platform/headless
export LD_LIBRARY_PATH=./build/cogbridge:./build/core:$LD_LIBRARY_PATH

# Run
./my_app
```

## 🎯 Quick Reference

### Essential Environment Variables

```bash
# Module directory (required for platform loading)
export COG_MODULEDIR=./build/platform/headless

# Library path (if not installed system-wide)
export LD_LIBRARY_PATH=./build/cogbridge:./build/core:$LD_LIBRARY_PATH
```

### Build Commands Cheat Sheet

```bash
# Clean build
rm -rf build
meson setup build -Dexamples=true -Dlibportal=disabled -Dprograms=false -Dplatforms=headless
ninja -C build

# Rebuild after changes
ninja -C build

# Install system-wide
sudo ninja -C build install
```

### Run Commands Cheat Sheet

```bash
# Minimal example
./cogbridge/examples/run-examples.sh minimal

# Dashboard example  
./cogbridge/examples/run-examples.sh dashboard

# With custom settings
COG_MODULEDIR=./build/platform/headless \
G_MESSAGES_DEBUG=all \
./build/cogbridge/examples/cogbridge-minimal
```

## ✨ Success Checklist

- [x] Build completes without errors
- [x] `cogbridge-minimal` runs successfully
- [x] `cogbridge-dashboard` runs successfully
- [x] C functions can be called from JavaScript
- [x] JavaScript can be executed from C
- [x] Events are emitted and received
- [x] Page loads and becomes ready
- [x] Examples run for their expected duration
- [x] Clean exit with no crashes

## 📚 Next Steps

1. **Read the Documentation**: Check out the comprehensive docs in `cogbridge/README.md`
2. **Try the Examples**: Experiment with modifying the examples
3. **Build Your App**: Use the examples as templates for your own application
4. **Explore the API**: Review `cogbridge/QUICK_REFERENCE.md` for API details
5. **Check Architecture**: Read `cogbridge/OVERVIEW.md` to understand the design

## 🎉 Congratulations!

You have successfully built and run CogBridge! The framework is ready to use for building headless embedded web applications with bidirectional C ↔ JavaScript communication.

For more information, see:
- `cogbridge/README.md` - Full documentation
- `cogbridge/GETTING_STARTED.md` - Tutorial
- `cogbridge/QUICK_REFERENCE.md` - API reference
- `cogbridge/OVERVIEW.md` - Architecture details

Happy coding! 🚀