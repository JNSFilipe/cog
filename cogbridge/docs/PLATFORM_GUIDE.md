# CogBridge Platform Guide: Understanding Display Modes

## TL;DR - What You Need to Know

**If you want to show a GUI on an embedded system WITHOUT X11/Wayland/GTK/Qt:**
→ **Use DRM platform** (`config.platform_name = "drm"`)

**If you truly want NO visual output (testing/automation):**
→ **Use headless platform** (`config.platform_name = "headless"`)

**If you're developing on a desktop with X11/Wayland:**
→ **Use GTK4 platform** (`config.platform_name = "gtk4"`)

---

## The Terminology Confusion

### What "Headless" Usually Means in Embedded Systems

In the embedded systems world, "headless" often means:
- **No desktop environment** (no GNOME, KDE, etc.)
- **No display server** (no X11, Wayland)
- **No heavy GUI frameworks** (no GTK, Qt)

But the display **still works** - it renders directly to the framebuffer!

### What "Headless" Means in CogBridge

In CogBridge (and Cog), the term "headless" is more literal:
- **No visual output whatsoever**
- Nothing is displayed on any screen
- Rendering happens in memory only
- Used for testing, screenshots, automation

### The Right Term for Embedded with Display

What you want for embedded systems with displays is:
- **DRM platform** = Direct Rendering Manager
- Renders **directly to framebuffer**
- Shows **full GUI on screen**
- **NO X11, Wayland, GTK, or Qt dependencies**
- This is what Cog was designed for!

---

## Platform Comparison Table

| Aspect | DRM | Headless | GTK4 |
|--------|-----|----------|------|
| **Shows GUI on screen?** | ✅ YES | ❌ NO | ✅ YES |
| **Requires X11/Wayland?** | ❌ NO | ❌ NO | ✅ YES |
| **Requires GTK/Qt?** | ❌ NO | ❌ NO | ✅ YES |
| **Requires display server?** | ❌ NO | ❌ NO | ✅ YES |
| **How it renders** | Direct to GPU/FB | Memory buffers | Desktop window |
| **Dependencies** | DRM, GBM, libinput | Minimal | Full desktop stack |
| **Binary size** | ~10-15 MB | ~8-10 MB | ~50-80 MB |
| **RAM usage (typical)** | 40-60 MB | 30-50 MB | 80-150 MB |
| **Best for** | Embedded, kiosks, HMI | Testing, CI/CD | Development |
| **User interaction** | Touch, KB, mouse | None (code only) | Desktop KB/mouse |
| **Window decorations** | None (fullscreen) | N/A | Yes (title bar, etc) |
| **Multi-window support** | No | No | Yes |
| **Hardware acceleration** | ✅ YES (OpenGL ES) | Limited | ✅ YES (OpenGL) |
| **Production ready** | ✅ YES | ✅ YES | ⚠️ Development only |

---

## Detailed Platform Descriptions

### 🖥️ DRM Platform - The Embedded Choice

**What it does:** Renders HTML/CSS/JavaScript GUI directly to the display framebuffer using the kernel's Direct Rendering Manager (DRM) subsystem.

**How it works:**
```
Your C App → CogBridge → WebKit WPE → OpenGL ES → GBM/EGL → DRM/KMS → GPU → Display
```

**No intermediate layers like:**
- ❌ X11 server
- ❌ Wayland compositor
- ❌ GTK toolkit
- ❌ Window manager
- ❌ Desktop environment

**Perfect for:**
- Raspberry Pi projects with displays
- Industrial HMI panels
- Automotive infotainment systems
- Kiosks and digital signage
- Medical devices with touchscreens
- IoT devices with displays
- Embedded Linux appliances

**Hardware requirements:**
- DRM-capable GPU driver (Intel, AMD, NVIDIA, ARM Mali, VideoCore, etc.)
- `/dev/dri/card0` device
- Kernel with DRM/KMS support
- OpenGL ES 2.0+ support

**Software dependencies:**
```bash
libdrm >= 2.4.71          # Kernel DRM interface
libgbm                    # Buffer management
libinput                  # Keyboard/mouse/touch input
libudev                   # Device management
libepoxy                  # OpenGL loader
wpewebkit >= 2.28         # WebKit engine
wpebackend-fdo >= 1.14    # WPE backend
```

**Configuration:**
```c
CogBridgeConfig config = {
    .width = 1920,
    .height = 1080,
    .platform_name = "drm",  // Direct framebuffer rendering
    .enable_console = true
};
```

**Build command:**
```bash
meson setup build -Dplatforms=drm -Dlibportal=disabled -Dprograms=false
```

**Run command:**
```bash
# Must have DRM device access (video group or root)
sudo ./your-app
# or
sudo usermod -a -G video,input $USER  # then logout/login
./your-app
```

---

### 🚫 Headless Platform - Truly No Display

**What it does:** Runs WebKit engine with **NO visual output**. Rendering happens in shared memory buffers that are never displayed.

**How it works:**
```
Your C App → CogBridge → WebKit WPE → SHM buffers → /dev/null
                                                    ↓
                                            (nothing displayed)
```

**Perfect for:**
- Automated testing (no display needed)
- CI/CD pipelines
- Server-side rendering (generate screenshots/PDFs)
- Web scraping
- Headless browser automation
- Backend processing

**NOT suitable for:**
- ❌ Showing GUI to users
- ❌ Interactive kiosks
- ❌ Displays of any kind

**Software dependencies:**
```bash
wpewebkit >= 2.28         # WebKit engine
wpebackend-fdo >= 1.14    # WPE backend
glib2                     # GLib utilities
json-glib                 # JSON parsing
```

**Configuration:**
```c
CogBridgeConfig config = {
    .width = 1920,
    .height = 1080,
    .platform_name = "headless",  // No visual output!
    .enable_console = true
};
```

**Build command:**
```bash
meson setup build -Dplatforms=headless -Dlibportal=disabled -Dprograms=false
```

**What you'll see:**
- ✅ Console log messages
- ✅ JavaScript executes normally
- ✅ Your C callbacks get called
- ✅ Events are emitted and received
- ❌ **NO window anywhere**
- ❌ **NO display output**

---

### 🪟 GTK4 Platform - Desktop Development

**What it does:** Shows a traditional desktop window using GTK4 toolkit. Requires full desktop environment.

**How it works:**
```
Your C App → CogBridge → WebKit GTK → GTK4 → Wayland/X11 → Desktop
```

**Perfect for:**
- Developing on your laptop/workstation
- Debugging with browser DevTools
- Prototyping before deploying to hardware
- Desktop applications (if that's your goal)

**NOT suitable for:**
- ❌ Embedded systems (too heavy)
- ❌ Minimal installations
- ❌ Headless servers

**Software dependencies:**
```bash
gtk4                      # GTK toolkit (~100+ MB)
libportal-gtk4           # Portal support
adwaita-icon-theme       # Icons
wpewebkit >= 2.28        # WebKit
+ many GTK/desktop deps
```

**Configuration:**
```c
CogBridgeConfig config = {
    .width = 1280,
    .height = 720,
    .platform_name = "gtk4",  // Desktop window
    .enable_console = true,
    .enable_developer_extras = true  // Enable DevTools
};
```

**Build command:**
```bash
meson setup build -Dplatforms=gtk4 -Dprograms=false
```

---

## Decision Tree

```
Do you want to show a GUI?
│
├─ NO → Use "headless" platform
│        (testing, automation, server-side)
│
└─ YES → Is this for embedded/production?
         │
         ├─ YES → Use "drm" platform
         │         (Raspberry Pi, industrial, kiosks)
         │
         └─ NO → Use "gtk4" platform
                  (desktop development)
```

---

## Common Scenarios

### Scenario 1: Raspberry Pi Kiosk

**Goal:** Show web-based UI on HDMI display, no desktop environment

**Solution:** DRM platform
```bash
# Build
meson setup build -Dplatforms=drm -Dlibportal=disabled
ninja -C build

# Configure Raspberry Pi for DRM
sudo nano /boot/firmware/config.txt
# Ensure: dtoverlay=vc4-kms-v3d

# Add user to groups
sudo usermod -a -G video,input pi

# Run
COG_MODULEDIR=./build/platform/drm ./your-app
```

### Scenario 2: Automated UI Testing

**Goal:** Test web UI without showing windows in CI/CD

**Solution:** Headless platform
```bash
# Build
meson setup build -Dplatforms=headless -Dlibportal=disabled
ninja -C build

# Run tests (no display needed)
COG_MODULEDIR=./build/platform/headless ./run-tests
```

### Scenario 3: Desktop Development

**Goal:** Develop UI on Ubuntu desktop before deploying to hardware

**Solution:** GTK4 platform
```bash
# Build
meson setup build -Dplatforms=gtk4,drm
ninja -C build

# Develop with GTK4 (easy debugging)
COG_MODULEDIR=./build/platform/gtk4 ./your-app

# Test with DRM (close to production)
sudo COG_MODULEDIR=./build/platform/drm ./your-app
```

### Scenario 4: Industrial HMI Panel

**Goal:** Touchscreen interface for factory equipment

**Solution:** DRM platform
```bash
# Build for production
meson setup build -Dplatforms=drm -Dbuildtype=release
ninja -C build

# Install to embedded target
ninja -C build install

# Configure for auto-start
sudo systemctl enable your-hmi-service
```

---

## Migration Examples

### From GTK/Qt to CogBridge DRM

**Before (heavy GTK application):**
```c
// Requires: gtk3-dev, webkit2gtk-4.0-dev (100+ MB)
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

int main() {
    gtk_init(NULL, NULL);
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *webview = webkit_web_view_new();
    gtk_container_add(GTK_CONTAINER(window), webview);
    gtk_widget_show_all(window);
    gtk_main();
}
```

**After (minimal CogBridge DRM):**
```c
// Requires: libdrm, libgbm, libinput, wpewebkit (15 MB)
#include <cogbridge/cogbridge.h>

int main() {
    CogBridgeConfig config = {
        .platform_name = "drm"  // Direct framebuffer
    };
    cogbridge_init(&config, NULL);
    
    CogBridge *bridge = cogbridge_new("app");
    cogbridge_load_uri(bridge, "file:///app/index.html");
    cogbridge_run(bridge);
}
```

**Benefits:**
- 85% smaller dependency footprint
- 50% lower RAM usage
- No X11/Wayland dependency
- Faster boot time
- More secure (smaller attack surface)

---

## Platform Selection Best Practices

### For Embedded Systems
1. **Always start with DRM** if you need display output
2. Use headless **only** for backend processing
3. Use GTK4 **only** during development phase
4. Build both DRM and headless: `meson setup build -Dplatforms=drm,headless`

### For Desktop Applications
1. Use GTK4 for traditional desktop apps
2. Consider Electron/Tauri if you need cross-platform
3. DRM mode won't work well on desktop (exclusive access issues)

### For Testing
1. Use headless for CI/CD pipelines
2. Use GTK4 for interactive test debugging
3. Use DRM for hardware-in-the-loop testing

---

## FAQ

**Q: I want GUI on my embedded device. Which platform?**
A: DRM platform. It's what you want!

**Q: Does DRM platform work without a display server?**
A: Yes! That's the whole point. No X11, no Wayland needed.

**Q: Why would I use headless if I have a display?**
A: You wouldn't. Headless is for servers, testing, automation.

**Q: Can I switch platforms at runtime?**
A: No, but you can rebuild with multiple platforms and choose via `config.platform_name`.

**Q: Does DRM work on Raspberry Pi?**
A: Yes! Perfect match. Just enable vc4-kms-v3d in config.txt.

**Q: What about Wayland platform?**
A: Exists but requires Wayland compositor. Use DRM for embedded instead.

**Q: Can I use DRM on my Ubuntu desktop?**
A: Only if you stop X11/Wayland first. Not recommended for desktop.

**Q: What's the smallest platform?**
A: Headless (minimal deps), then DRM, then GTK4 (huge deps).

---

## Summary

| You want... | Use this platform | Build with |
|-------------|-------------------|------------|
| GUI on embedded system | **drm** | `-Dplatforms=drm` |
| No GUI (testing) | **headless** | `-Dplatforms=headless` |
| Desktop window (dev) | **gtk4** | `-Dplatforms=gtk4` |

**The key insight:** "Headless" in embedded usually means "no desktop bloat, but GUI still works" - that's **DRM platform** in CogBridge!

True "headless" (no display at all) is for testing and automation only.

---

## Quick Start Commands

**Embedded system with display (most common):**
```bash
meson setup build -Dplatforms=drm -Dlibportal=disabled -Dexamples=true
ninja -C build
sudo ./build/cogbridge/examples/cogbridge-minimal
```

**Testing/automation (no display):**
```bash
meson setup build -Dplatforms=headless -Dlibportal=disabled -Dexamples=true
ninja -C build
./build/cogbridge/examples/cogbridge-minimal
```

**Desktop development:**
```bash
meson setup build -Dplatforms=gtk4 -Dexamples=true
ninja -C build
./build/cogbridge/examples/cogbridge-minimal-gui
```

---

**For 99% of embedded systems with displays: use DRM platform!** 🚀