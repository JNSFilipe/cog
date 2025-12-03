# Building and Testing with Both DRM and GTK4 Platforms

This guide shows how to build CogBridge with both DRM and GTK4 platforms, allowing you to:
- Develop and debug on your desktop with GTK4
- Test embedded mode with DRM before deployment
- Switch between platforms easily

## Quick Start

```bash
# Install dependencies for both platforms
sudo apt-get install -y \
    build-essential meson ninja-build \
    libwpewebkit-1.0-dev libwpe-1.0-dev \
    libglib2.0-dev libjson-glib-dev \
    libdrm-dev libgbm-dev libinput-dev \
    libudev-dev libepoxy-dev \
    libgtk-4-dev libportal-gtk4-dev

# Build with BOTH platforms
cd cog
meson setup build \
  -Dexamples=true \
  -Dplatforms=drm,gtk4 \
  -Dprograms=false

ninja -C build
```

## Verifying the Build

Check that both platform modules were built:

```bash
# Should see cogplatform-drm.so
ls -l build/platform/drm/

# Should see cogplatform-gtk4.so
ls -l build/platform/gtk4/

# Check examples were built
ls -l build/cogbridge/examples/
```

Expected executables:
- `cogbridge-minimal`
- `cogbridge-dashboard`
- `cogbridge-minimal-gui`
- `cogbridge-platform-switch`

---

## Running with GTK4 (Desktop Development)

GTK4 is perfect for:
- Development on your desktop/laptop
- Debugging with browser DevTools
- Testing UI without hardware
- Visual inspection and iteration

### Basic Run

```bash
# Set platform module directory
export COG_MODULEDIR=./build/platform/gtk4

# Run any example
./build/cogbridge/examples/cogbridge-minimal
./build/cogbridge/examples/cogbridge-dashboard
./build/cogbridge/examples/cogbridge-minimal-gui
```

### With Platform Switcher

```bash
COG_MODULEDIR=./build/platform/gtk4 \
  ./build/cogbridge/examples/cogbridge-platform-switch gtk4
```

### What You'll See

- ✅ Desktop window with title bar and decorations
- ✅ Resizable window
- ✅ Your web interface rendered inside
- ✅ Mouse and keyboard work normally
- ✅ Can open multiple windows
- ✅ Browser DevTools available (if enabled in config)

### Tips for GTK4 Development

```bash
# Enable developer tools for debugging
# (Modify your code or use platform-switch example)
CogBridgeConfig config = {
    .platform_name = "gtk4",
    .enable_developer_extras = true  // Enable DevTools
};

# Press F12 or right-click → Inspect to open DevTools
```

---

## Running with DRM (Embedded Mode Test)

DRM mode tests your application as it will run on embedded hardware:
- Direct framebuffer rendering
- Full-screen display
- No window decorations
- Hardware-accelerated

### ⚠️ Important: DRM Requires Exclusive Display Access

DRM cannot run while X11 or Wayland is active. You have several options:

#### Option 1: SSH from Another Computer (Recommended)

```bash
# From your main desktop/laptop, SSH into the test machine
ssh user@test-machine

# On the test machine, stop display manager
sudo systemctl stop gdm       # GNOME
# or
sudo systemctl stop lightdm   # LightDM
# or
sudo systemctl stop sddm      # KDE

# Now run DRM examples
export COG_MODULEDIR=./build/platform/drm
sudo ./build/cogbridge/examples/cogbridge-minimal

# When done, restart display manager
sudo systemctl start gdm  # or lightdm, sddm
```

#### Option 2: Switch to a Virtual Terminal

```bash
# Press Ctrl+Alt+F2 (or F3, F4, etc.) to switch to text console
# Login to text console

cd /path/to/cog
export COG_MODULEDIR=./build/platform/drm
sudo ./build/cogbridge/examples/cogbridge-minimal

# The GUI will appear on the display
# Press Ctrl+C to quit

# Press Ctrl+Alt+F1 (or F7) to return to graphical desktop
```

#### Option 3: Run from Single-User Mode

```bash
# Switch to multi-user target (no GUI)
sudo systemctl isolate multi-user.target

# Run your DRM application
cd /path/to/cog
export COG_MODULEDIR=./build/platform/drm
sudo ./build/cogbridge/examples/cogbridge-minimal

# Return to graphical mode
sudo systemctl isolate graphical.target
```

### Basic DRM Run

```bash
export COG_MODULEDIR=./build/platform/drm

# Run with sudo (need DRM device access)
sudo ./build/cogbridge/examples/cogbridge-minimal
```

### With Platform Switcher

```bash
COG_MODULEDIR=./build/platform/drm \
  sudo ./build/cogbridge/examples/cogbridge-platform-switch drm
```

### Setting Up Permissions (No sudo needed)

```bash
# Add your user to video and input groups
sudo usermod -a -G video,input $USER

# Verify
groups $USER

# Logout and login again for changes to take effect
# After logout/login, you can run without sudo:
export COG_MODULEDIR=./build/platform/drm
./build/cogbridge/examples/cogbridge-minimal
```

### What You'll See

- ✅ Full-screen interface on display
- ✅ No window decorations
- ✅ No desktop environment visible
- ✅ Direct hardware rendering
- ✅ Touch/keyboard/mouse input works
- ✅ Smooth, hardware-accelerated graphics

---

## Development Workflow: GTK4 → DRM

Recommended workflow for embedded development:

### 1. Develop with GTK4

```bash
# Set up for GTK4 development
export COG_MODULEDIR=./build/platform/gtk4

# Edit your HTML/CSS/JavaScript
nano your_app_ui.html

# Run and see changes immediately
./your-cogbridge-app

# Use browser DevTools for debugging (F12)
# Iterate until UI looks good
```

### 2. Test with DRM

```bash
# SSH into your machine or switch to VT
# Stop display manager
sudo systemctl stop gdm

# Test with DRM
export COG_MODULEDIR=./build/platform/drm
sudo ./your-cogbridge-app

# Verify it works full-screen
# Check touch/input
# Verify performance

# When satisfied, restart display
sudo systemctl start gdm
```

### 3. Deploy to Embedded Target

```bash
# Build release version with DRM only
meson configure build -Dplatforms=drm -Dbuildtype=release
ninja -C build

# Copy to target device
scp build/your-app target:/usr/local/bin/
scp build/libcogbridge.so* target:/usr/local/lib/
scp -r build/platform/drm target:/usr/local/lib/cog/modules/

# On target, run your app
./usr/local/bin/your-app
```

---

## Platform Switcher Example

The `platform-switch` example demonstrates runtime platform selection:

```bash
# Build with all platforms
meson setup build -Dplatforms=drm,gtk4,headless -Dexamples=true
ninja -C build

# Run with GTK4 (desktop window)
COG_MODULEDIR=./build/platform/gtk4 \
  ./build/cogbridge/examples/cogbridge-platform-switch gtk4

# Run with DRM (framebuffer - need to stop X11/Wayland first)
COG_MODULEDIR=./build/platform/drm \
  sudo ./build/cogbridge/examples/cogbridge-platform-switch drm

# Run with headless (no GUI)
COG_MODULEDIR=./build/platform/headless \
  ./build/cogbridge/examples/cogbridge-platform-switch headless
```

### Platform Switcher Usage

```bash
# Get help
./build/cogbridge/examples/cogbridge-platform-switch --help

# Specify platform
./build/cogbridge/examples/cogbridge-platform-switch [drm|gtk4|headless]
```

---

## Switching Platforms in Your Code

### Method 1: Command-Line Argument

```c
#include <cogbridge/cogbridge.h>

int main(int argc, char *argv[]) {
    const char *platform = "drm";  // Default
    
    // Parse arguments
    if (argc > 1) {
        platform = argv[1];
    }
    
    // Configure with selected platform
    CogBridgeConfig config = {
        .platform_name = platform,
        .width = 1920,
        .height = 1080
    };
    
    cogbridge_init(&config, NULL);
    // ... rest of your code
}
```

Usage:
```bash
./your-app gtk4    # Use GTK4
./your-app drm     # Use DRM
```

### Method 2: Environment Variable

```c
#include <cogbridge/cogbridge.h>
#include <stdlib.h>

int main() {
    const char *platform = getenv("COGBRIDGE_PLATFORM");
    if (!platform) {
        platform = "drm";  // Default
    }
    
    CogBridgeConfig config = {
        .platform_name = platform
    };
    
    cogbridge_init(&config, NULL);
    // ... rest of your code
}
```

Usage:
```bash
COGBRIDGE_PLATFORM=gtk4 ./your-app
COGBRIDGE_PLATFORM=drm ./your-app
```

### Method 3: Compile-Time Selection

```c
#include <cogbridge/cogbridge.h>

int main() {
#ifdef EMBEDDED_BUILD
    const char *platform = "drm";
#else
    const char *platform = "gtk4";
#endif
    
    CogBridgeConfig config = {
        .platform_name = platform
    };
    
    cogbridge_init(&config, NULL);
    // ... rest of your code
}
```

Build:
```bash
# For development
meson setup build -Dplatforms=gtk4,drm
ninja -C build

# For embedded deployment
meson setup build -Dplatforms=drm -Dc_args="-DEMBEDDED_BUILD"
ninja -C build
```

---

## Comparison: GTK4 vs DRM

| Aspect | GTK4 | DRM |
|--------|------|-----|
| **Display** | Window with decorations | Full-screen, no decorations |
| **Environment** | X11/Wayland desktop | Direct to framebuffer |
| **Dependencies** | Heavy (~100+ MB) | Light (~15 MB) |
| **Setup** | Just run | May need permissions |
| **Use During** | Development | Testing & deployment |
| **DevTools** | Yes (F12) | No |
| **Multi-window** | Yes | No |
| **Resize** | Yes | No (fixed resolution) |
| **Performance** | Desktop OpenGL | OpenGL ES (optimized) |

---

## Troubleshooting

### GTK4 Issues

**Problem:** "Could not load platform module"
```bash
# Solution: Check GTK4 module exists
ls build/platform/gtk4/
# Ensure COG_MODULEDIR is set correctly
export COG_MODULEDIR=./build/platform/gtk4
```

**Problem:** "No display found" or "$DISPLAY not set"
```bash
# Solution: Ensure X11 or Wayland is running
echo $DISPLAY          # Should show ":0" or similar
echo $WAYLAND_DISPLAY  # Should show "wayland-0" or similar

# Start a display server if needed
startx
```

**Problem:** Window doesn't appear
```bash
# Check if process is running
ps aux | grep cogbridge

# Check logs
G_MESSAGES_DEBUG=all ./your-app
```

### DRM Issues

**Problem:** "Permission denied" accessing /dev/dri/card0
```bash
# Solution: Add user to video group
sudo usermod -a -G video,input $USER
# Logout and login

# Or run with sudo
sudo ./your-app
```

**Problem:** "Device or resource busy"
```bash
# Solution: Stop X11/Wayland
sudo systemctl stop gdm  # or lightdm, sddm

# Check nothing is using DRM
sudo fuser /dev/dri/card0
```

**Problem:** "No display found"
```bash
# Check DRM devices exist
ls -l /dev/dri/

# Verify DRM driver loaded
lsmod | grep drm

# Check connected displays
sudo modetest -c
```

For more troubleshooting: See [DRM_TROUBLESHOOTING.md](DRM_TROUBLESHOOTING.md)

---

## Best Practices

### ✅ Do This

- **Develop with GTK4** - Fast iteration, easy debugging
- **Test with DRM** - Verify before deployment
- **Build both platforms** during development
- **Deploy with DRM only** to embedded targets
- **Use version control** to track platform-specific changes
- **Test input devices** (touch, keyboard) in DRM mode

### ❌ Avoid This

- **Don't test GTK4 and assume DRM will work** - Always test DRM!
- **Don't run DRM on your main desktop** - Use a test machine or VM
- **Don't deploy GTK4 to embedded** - Too heavy
- **Don't forget permissions** - Set up video/input groups

---

## Example: Complete Development Cycle

```bash
# 1. Build with both platforms
meson setup build -Dplatforms=drm,gtk4 -Dexamples=true
ninja -C build

# 2. Develop with GTK4 on desktop
export COG_MODULEDIR=./build/platform/gtk4
./build/cogbridge/examples/cogbridge-minimal
# Edit HTML, refresh, iterate...

# 3. Test with DRM (SSH to test machine or use VT)
ssh testmachine
cd /path/to/cog
sudo systemctl stop gdm
export COG_MODULEDIR=./build/platform/drm
sudo ./build/cogbridge/examples/cogbridge-minimal
# Verify full-screen display works
# Test touch/input
# Check performance
sudo systemctl start gdm
exit

# 4. Deploy to embedded target
meson configure build -Dplatforms=drm -Dbuildtype=release
ninja -C build
rsync -av build/your-app embedded-target:/app/
rsync -av build/libcogbridge.so* embedded-target:/usr/local/lib/
rsync -av build/platform/drm embedded-target:/usr/local/lib/cog/modules/

# 5. Run on embedded target
ssh embedded-target
/app/your-app
```

---

## Summary

**For development:**
```bash
# GTK4 - Easy, fast, desktop window
meson setup build -Dplatforms=drm,gtk4
COG_MODULEDIR=./build/platform/gtk4 ./your-app
```

**For testing embedded mode:**
```bash
# DRM - Full-screen, direct framebuffer
sudo systemctl stop gdm
COG_MODULEDIR=./build/platform/drm sudo ./your-app
```

**For production deployment:**
```bash
# DRM only - Minimal, optimized
meson setup build -Dplatforms=drm -Dbuildtype=release
# Deploy to embedded target
```

Building with both platforms gives you the best of both worlds:
- **Fast development** with GTK4
- **Accurate testing** with DRM
- **Production-ready** embedded deployment

🚀 Happy developing!