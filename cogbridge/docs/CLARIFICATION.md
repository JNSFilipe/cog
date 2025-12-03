# CogBridge Platform Clarification

## Executive Summary

**CogBridge DOES support GUI display on embedded systems!**

The confusion arose from the term "headless" which has different meanings:

- **In embedded systems jargon:** "headless" often means "no X11/Wayland/desktop environment" but GUI still works
- **In CogBridge terminology:** "headless" literally means "no visual output whatsoever"

## The Solution: DRM Platform

For embedded systems with displays, use the **DRM (Direct Rendering Manager) platform**:

```c
CogBridgeConfig config = {
    .platform_name = "drm",  // ← This shows GUI on display!
    .width = 1920,
    .height = 1080
};
cogbridge_init(&config, NULL);
```

### What DRM Platform Provides

✅ **Full GUI rendered on display**
✅ **No X11 or Wayland server needed**
✅ **No GTK or Qt frameworks needed**
✅ **Direct rendering to framebuffer via DRM/KMS**
✅ **Hardware-accelerated OpenGL ES rendering**
✅ **Native input handling (keyboard, mouse, touch)**
✅ **Perfect for embedded Linux systems**

This is exactly what Cog was designed for and what you want!

## Platform Comparison

| Platform | Visual Output | Use Case | Dependencies |
|----------|---------------|----------|--------------|
| **DRM** | ✅ YES (Framebuffer) | Embedded systems with displays | libdrm, libgbm, libinput |
| **Headless** | ❌ NO | Testing, automation, server-side | Minimal (no display libs) |
| **GTK4** | ✅ YES (Window) | Desktop development | GTK4, X11/Wayland |

## How to Build and Run

### Build with DRM Platform

```bash
cd cog

# Configure for DRM (direct framebuffer rendering)
meson setup build \
  -Dexamples=true \
  -Dplatforms=drm \
  -Dprograms=false \
  -Dlibportal=disabled

# Build
ninja -C build
```

### Install Dependencies (Ubuntu/Debian)

```bash
sudo apt-get install -y \
    build-essential meson ninja-build \
    libwpewebkit-1.0-dev libwpe-1.0-dev \
    libglib2.0-dev libjson-glib-dev \
    libdrm-dev libgbm-dev libinput-dev \
    libudev-dev libepoxy-dev
```

### Run with DRM Platform

```bash
# Set platform module path
export COG_MODULEDIR=./build/platform/drm

# Run (need DRM device access)
sudo ./build/cogbridge/examples/cogbridge-minimal

# Or add user to video/input groups (logout required)
sudo usermod -a -G video,input $USER
```

## What You'll See

When running with DRM platform:

1. **Console output** showing initialization and function calls
2. **Full GUI displayed on screen** (your HTML/CSS/JavaScript rendered)
3. **Interactive interface** (buttons, forms, etc. all work)
4. **Smooth rendering** with hardware acceleration

Example output:
```
=== CogBridge Minimal Example ===

** Message: CogBridge initialized successfully
** Message: CogBridge instance created: minimal-example
** Message: DRM: Using device /dev/dri/card0
** Message: DRM: Display mode: 1920x1080@60Hz
** Message: Page loaded and ready
[C] add(42, 58) = 100
[C] greet('World')
```

**AND** you'll see your web interface rendered on the display!

## Raspberry Pi Example

Perfect use case - show GUI on HDMI without X11/Wayland:

```bash
# 1. Enable DRM/KMS on Raspberry Pi
sudo nano /boot/firmware/config.txt
# Add: dtoverlay=vc4-kms-v3d

# 2. Reboot
sudo reboot

# 3. Add user to groups
sudo usermod -a -G video,input pi

# 4. Build with DRM platform
meson setup build -Dplatforms=drm -Dlibportal=disabled -Dexamples=true
ninja -C build

# 5. Run and see GUI on HDMI!
COG_MODULEDIR=./build/platform/drm ./build/cogbridge/examples/cogbridge-minimal
```

## Key Differences from GTK/Qt

### Traditional Approach (Heavy)
```
Your App → GTK/Qt → X11/Wayland → GPU → Display
           └─ 100+ MB dependencies
```

### CogBridge DRM Approach (Lightweight)
```
Your App → CogBridge → WPE WebKit → OpenGL ES → DRM/KMS → GPU → Display
           └─ 15 MB dependencies, no desktop stack
```

## When to Use Each Platform

### Use DRM Platform When:
- ✅ Building for embedded Linux (Raspberry Pi, BeagleBone, industrial boards)
- ✅ Creating kiosks or digital signage
- ✅ Developing industrial HMI panels
- ✅ Building automotive displays
- ✅ IoT devices with touchscreens
- ✅ You want GUI **without** X11/Wayland/GTK/Qt bloat

### Use Headless Platform When:
- ✅ Running automated tests (CI/CD)
- ✅ Server-side web rendering
- ✅ Headless browser automation
- ✅ Web scraping
- ✅ You truly **don't want** any visual output

### Use GTK4 Platform When:
- ✅ Developing on your desktop/laptop
- ✅ Debugging with browser DevTools
- ✅ Prototyping before deploying to hardware
- ✅ Building traditional desktop applications

## Code Example

```c
#include <cogbridge/cogbridge.h>

int main() {
    // Configure for DRM platform (shows GUI on display)
    CogBridgeConfig config = {
        .width = 1920,
        .height = 1080,
        .platform_name = "drm",      // Direct framebuffer rendering
        .enable_console = true
    };
    
    if (!cogbridge_init(&config, NULL)) {
        fprintf(stderr, "Failed to initialize\n");
        return 1;
    }
    
    // Create bridge and load your web interface
    CogBridge *bridge = cogbridge_new("my-app");
    cogbridge_load_uri(bridge, "file:///app/index.html");
    
    // Your HTML/CSS/JavaScript will render on the display!
    cogbridge_run(bridge);
    
    // Cleanup
    cogbridge_free(bridge);
    cogbridge_cleanup();
    
    return 0;
}
```

## Troubleshooting

### "Failed to open DRM device"
```bash
# Add user to video group
sudo usermod -a -G video,input $USER
# Then logout and login again
```

### "No display found"
```bash
# Check DRM devices exist
ls -l /dev/dri/

# Verify DRM driver loaded
lsmod | grep drm

# Check connected displays
sudo modetest -c
```

### Still issues?
See detailed troubleshooting: [DRM_TROUBLESHOOTING.md](DRM_TROUBLESHOOTING.md)

## Summary

**The Bottom Line:**

- ❌ **Don't use** "headless" platform if you want to show GUI
- ✅ **DO use** "drm" platform for embedded systems with displays
- 🎯 **DRM = GUI without desktop bloat** (exactly what you want!)

CogBridge with DRM platform provides the same functionality as Cog - direct framebuffer rendering without X11/Wayland/GTK/Qt dependencies. This is perfect for embedded systems!

## Documentation Links

- **[PLATFORM_GUIDE.md](PLATFORM_GUIDE.md)** - Complete platform comparison and selection guide
- **[DRM_TROUBLESHOOTING.md](DRM_TROUBLESHOOTING.md)** - Detailed DRM troubleshooting
- **[BUILD_AND_RUN.md](BUILD_AND_RUN.md)** - Build instructions for all platforms
- **[GETTING_STARTED.md](GETTING_STARTED.md)** - Step-by-step tutorial
- **[OVERVIEW.md](OVERVIEW.md)** - Architecture overview

---

**For 99% of embedded systems: Use DRM platform!** 🚀