# Platform Modes: DRM vs Headless vs GTK4

CogBridge supports multiple platform modes, each designed for different use cases. Understanding the difference is crucial for choosing the right mode for your application.

## Platform Comparison

| Feature | DRM | Headless | GTK4 |
|---------|-----|----------|------|
| **Visual Output** | ✅ Yes (framebuffer) | ❌ No | ✅ Yes (windowed) |
| **X11/Wayland Required** | ❌ No | ❌ No | ✅ Yes |
| **GTK/Qt Required** | ❌ No | ❌ No | ✅ Yes (GTK4) |
| **Display Server** | None (direct to hardware) | None | X11 or Wayland |
| **Best For** | Embedded systems, kiosks | Testing, server-side | Development |
| **Memory Usage** | Low | Very Low | High |
| **Dependencies** | libdrm, libinput, gbm | Minimal | Full desktop stack |

## DRM Platform (Recommended for Embedded)

**What it is:** Direct Rendering Manager platform renders directly to the framebuffer without any display server.

**Visual Output:** ✅ **YES** - Full GUI is displayed on screen

### Key Features
- Renders directly to `/dev/dri/card0` (or other DRM device)
- Uses KMS (Kernel Mode Setting) for display configuration
- OpenGL ES hardware acceleration via GBM/EGL
- Native input handling via libinput (keyboard, mouse, touch)
- **No X11, Wayland, GTK, or Qt dependencies**
- Full-screen display (no window decorations)
- Ideal for:
  - Embedded Linux systems (Raspberry Pi, BeagleBone, industrial boards)
  - Kiosks and digital signage
  - Industrial HMI (Human-Machine Interface)
  - Automotive displays
  - IoT devices with displays

### Dependencies
```bash
# Runtime dependencies
libdrm >= 2.4.71
libgbm (usually part of Mesa)
libinput
libudev
libepoxy
wpewebkit >= 2.28
wpebackend-fdo >= 1.14
```

### Building
```bash
meson setup build \
  -Dexamples=true \
  -Dplatforms=drm \
  -Dprograms=false \
  -Dlibportal=disabled

ninja -C build
```

### Running
```bash
# Set platform module path
export COG_MODULEDIR=./build/platform/drm

# Run your application (must run as root or with DRM permissions)
sudo ./build/cogbridge/examples/cogbridge-minimal

# Or add your user to the video/input groups
sudo usermod -a -G video,input $USER
```

### Configuration in Code
```c
CogBridgeConfig config = {
    .width = 1920,
    .height = 1080,
    .platform_name = "drm",  // Direct framebuffer rendering
    .enable_console = true
};

cogbridge_init(&config, NULL);
```

### Hardware Requirements
- DRM-capable graphics driver (most modern Linux systems)
- GPU with OpenGL ES support (for hardware acceleration)
- Access to `/dev/dri/card0` device
- Access to `/dev/input/*` devices for input
- Framebuffer support in kernel

### Troubleshooting DRM Mode

**Problem:** `Failed to open DRM device`
**Solution:** 
```bash
# Check DRM device exists
ls -l /dev/dri/

# Check permissions
sudo chmod 666 /dev/dri/card0

# Or run as root
sudo ./your-app
```

**Problem:** `No suitable display found`
**Solution:**
```bash
# List available displays
modetest -c

# Check kernel has DRM enabled
lsmod | grep drm
```

**Problem:** `Input devices not working`
**Solution:**
```bash
# Check input permissions
ls -l /dev/input/

# Add user to input group
sudo usermod -a -G input $USER
```

---

## Headless Platform

**What it is:** True headless mode with **NO visual output** whatsoever.

**Visual Output:** ❌ **NO** - Nothing is displayed on screen

### Key Features
- Web rendering happens in memory only
- Outputs to shared memory buffers (not displayed)
- Useful for:
  - Automated testing
  - Server-side rendering (screenshots, PDFs)
  - Web scraping
  - Headless automation
  - CI/CD pipelines
  - Backend processing where no display is needed

### Use Cases
- **Testing:** Run automated UI tests without a display
- **Server-side:** Generate screenshots or PDFs on a server
- **Validation:** Verify web content loads and JS executes correctly
- **Batch processing:** Process web content in the background

### Dependencies
```bash
# Minimal dependencies (no display libraries needed)
wpewebkit >= 2.28
wpebackend-fdo >= 1.14
glib >= 2.44
json-glib >= 1.0
```

### Building
```bash
meson setup build \
  -Dexamples=true \
  -Dplatforms=headless \
  -Dprograms=false \
  -Dlibportal=disabled

ninja -C build
```

### Running
```bash
export COG_MODULEDIR=./build/platform/headless
./build/cogbridge/examples/cogbridge-minimal

# You'll see console output but NO GUI window
```

### Configuration in Code
```c
CogBridgeConfig config = {
    .width = 1920,
    .height = 1080,
    .platform_name = "headless",  // No visual output
    .enable_console = true
};

cogbridge_init(&config, NULL);
```

### What You'll See
When running in headless mode:
- ✅ Console log messages appear
- ✅ JavaScript executes normally
- ✅ C callbacks are invoked
- ✅ Events are emitted and received
- ❌ **NO window appears**
- ❌ **NO visual output on any display**

---

## GTK4 Platform (Development Only)

**What it is:** Traditional desktop GUI using GTK4 toolkit with window decorations.

**Visual Output:** ✅ **YES** - GUI window on desktop

### Key Features
- Runs as a normal desktop application
- Window with title bar, minimize/maximize buttons
- Requires X11 or Wayland display server
- Requires full GTK4 stack and dependencies
- **Large dependency footprint (100+ MB)**
- Useful for:
  - Development and debugging
  - Prototyping on desktop before deploying to embedded
  - Testing UI without physical hardware

### Dependencies
```bash
# Heavy desktop dependencies
gtk4
libportal-gtk4
adwaita-icon-theme
wpewebkit >= 2.28
wpebackend-fdo >= 1.14
+ many GTK/desktop dependencies
```

### Building
```bash
meson setup build \
  -Dexamples=true \
  -Dplatforms=gtk4,drm,headless \
  -Dprograms=false

ninja -C build
```

### Running
```bash
export COG_MODULEDIR=./build/platform/gtk4

# Must have X11 or Wayland session running
./build/cogbridge/examples/cogbridge-minimal-gui
```

### Configuration in Code
```c
CogBridgeConfig config = {
    .width = 1280,
    .height = 720,
    .platform_name = "gtk4",  // Desktop window
    .enable_console = true,
    .enable_developer_extras = true  // DevTools available
};

cogbridge_init(&config, NULL);
```

---

## Choosing the Right Platform

### For Production Embedded Systems
→ **Use DRM Platform**
- Full GUI without bloated dependencies
- Direct hardware rendering
- No display server overhead
- Perfect for embedded Linux

### For Backend/Server Applications
→ **Use Headless Platform**
- No display needed
- Minimal resource usage
- Testing and automation

### For Desktop Development
→ **Use GTK4 Platform**
- Easy debugging with DevTools
- Familiar desktop window
- Prototype before deploying to hardware

---

## Migration Guide

### From Headless to DRM (Add Visual Output)

```c
// Before (no visual output)
CogBridgeConfig config = {
    .platform_name = "headless"
};

// After (show GUI on framebuffer)
CogBridgeConfig config = {
    .platform_name = "drm"
};

// Rebuild with DRM platform
// meson setup build -Dplatforms=drm
```

### From GTK4 to DRM (Remove Desktop Dependencies)

```c
// Before (desktop window)
CogBridgeConfig config = {
    .platform_name = "gtk4"
};

// After (embedded framebuffer)
CogBridgeConfig config = {
    .platform_name = "drm"
};

// Rebuild with DRM platform (much smaller!)
// meson setup build -Dplatforms=drm -Dlibportal=disabled
```

---

## Platform Environment Variables

### DRM Platform
```bash
# Required: Platform module directory
export COG_MODULEDIR=/usr/lib/cog/modules  # or ./build/platform/drm

# Optional: Force specific DRM device
export COG_PLATFORM_DRM_DEVICE=/dev/dri/card1

# Optional: Set rotation (0, 90, 180, 270)
export COG_PLATFORM_DRM_ROTATION=0
```

### Headless Platform
```bash
export COG_MODULEDIR=/usr/lib/cog/modules  # or ./build/platform/headless

# Optional: Set frame rate (for internal rendering loop)
export COG_PLATFORM_HEADLESS_MAX_FPS=60
```

### GTK4 Platform
```bash
export COG_MODULEDIR=/usr/lib/cog/modules  # or ./build/platform/gtk4

# Standard GTK/Wayland/X11 environment variables apply
export WAYLAND_DISPLAY=wayland-0
export DISPLAY=:0
```

---

## Summary

- **DRM = GUI without X11/Wayland** (Embedded systems)
- **Headless = No GUI at all** (Testing/automation)
- **GTK4 = Traditional desktop GUI** (Development)

The term "headless" in embedded systems often means "without desktop dependencies", but in CogBridge:
- **DRM provides GUI without desktop dependencies** ← This is what you want!
- **Headless truly means no visual output at all**

For embedded systems with displays, **always use DRM platform**.