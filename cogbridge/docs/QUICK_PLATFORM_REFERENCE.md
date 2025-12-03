# CogBridge Platform Quick Reference Card

## TL;DR Commands

### Build Both Platforms
```bash
meson setup build -Dplatforms=drm,gtk4 -Dexamples=true -Dprograms=false
ninja -C build
```

### Run GTK4 (Desktop - Easy)
```bash
COG_MODULEDIR=./build/platform/gtk4 ./build/cogbridge/examples/cogbridge-minimal
```

### Run DRM (Embedded - Full Screen)
```bash
# Stop desktop first: sudo systemctl stop gdm
COG_MODULEDIR=./build/platform/drm sudo ./build/cogbridge/examples/cogbridge-minimal
```

---

## Platform Comparison

| Feature | GTK4 | DRM |
|---------|------|-----|
| **Shows GUI?** | ✅ Yes (window) | ✅ Yes (fullscreen) |
| **Needs X11/Wayland?** | ✅ Yes | ❌ No |
| **Use for** | Development | Production |
| **Best on** | Desktop/laptop | Embedded systems |
| **Dependencies** | Heavy | Light |
| **Setup** | Easy | Need permissions |

---

## Installation

### Ubuntu/Debian - Both Platforms
```bash
sudo apt-get install -y \
    build-essential meson ninja-build \
    libwpewebkit-1.0-dev libwpe-1.0-dev \
    libglib2.0-dev libjson-glib-dev \
    libdrm-dev libgbm-dev libinput-dev libudev-dev libepoxy-dev \
    libgtk-4-dev libportal-gtk4-dev
```

### Fedora/RHEL - Both Platforms
```bash
sudo dnf install -y \
    gcc meson ninja-build \
    wpewebkit-devel wpe-devel glib2-devel json-glib-devel \
    libdrm-devel mesa-libgbm-devel libinput-devel systemd-devel libepoxy-devel \
    gtk4-devel libportal-gtk4-devel
```

---

## Build Configurations

### Both Platforms (Development)
```bash
meson setup build -Dplatforms=drm,gtk4 -Dexamples=true
ninja -C build
```

### DRM Only (Production)
```bash
meson setup build -Dplatforms=drm -Dexamples=true -Dlibportal=disabled
ninja -C build
```

### GTK4 Only (Desktop App)
```bash
meson setup build -Dplatforms=gtk4 -Dexamples=true
ninja -C build
```

---

## Running Examples

### With GTK4
```bash
export COG_MODULEDIR=./build/platform/gtk4
./build/cogbridge/examples/cogbridge-minimal
./build/cogbridge/examples/cogbridge-dashboard
./build/cogbridge/examples/cogbridge-minimal-gui
```

### With DRM
```bash
# Setup (one-time)
sudo usermod -a -G video,input $USER  # Then logout/login

# Run
export COG_MODULEDIR=./build/platform/drm
./build/cogbridge/examples/cogbridge-minimal

# Or with sudo if group not set
sudo COG_MODULEDIR=./build/platform/drm ./build/cogbridge/examples/cogbridge-minimal
```

### Platform Switcher
```bash
# Try all three platforms with one example
COG_MODULEDIR=./build/platform/gtk4 \
  ./build/cogbridge/examples/cogbridge-platform-switch gtk4

COG_MODULEDIR=./build/platform/drm \
  sudo ./build/cogbridge/examples/cogbridge-platform-switch drm

COG_MODULEDIR=./build/platform/headless \
  ./build/cogbridge/examples/cogbridge-platform-switch headless
```

---

## Code Examples

### Basic Setup (Default = DRM)
```c
#include <cogbridge/cogbridge.h>

int main() {
    CogBridgeConfig config = {
        .width = 1920,
        .height = 1080,
        // platform_name defaults to "drm"
    };
    
    cogbridge_init(&config, NULL);
    CogBridge *bridge = cogbridge_new("app");
    cogbridge_load_uri(bridge, "file:///app/index.html");
    cogbridge_run(bridge);
    cogbridge_free(bridge);
    cogbridge_cleanup();
}
```

### Specify Platform
```c
CogBridgeConfig config = {
    .platform_name = "gtk4",  // or "drm", "headless"
    .width = 1920,
    .height = 1080
};
```

### Runtime Selection
```c
int main(int argc, char *argv[]) {
    const char *platform = (argc > 1) ? argv[1] : "drm";
    
    CogBridgeConfig config = {
        .platform_name = platform,
        .width = 1920,
        .height = 1080
    };
    
    cogbridge_init(&config, NULL);
    // ...
}
```

---

## Troubleshooting

### GTK4: "No display found"
```bash
# Ensure X11/Wayland is running
echo $DISPLAY  # Should show something
```

### DRM: "Permission denied"
```bash
# Fix permissions
sudo usermod -a -G video,input $USER
# Logout and login
```

### DRM: "Device busy"
```bash
# Stop desktop
sudo systemctl stop gdm  # or lightdm, sddm

# Or SSH from another machine
ssh user@test-machine
```

### DRM: "No display found"
```bash
# Check DRM device exists
ls -l /dev/dri/

# Check driver loaded
lsmod | grep drm
```

### "Platform module not found"
```bash
# Ensure COG_MODULEDIR is set
export COG_MODULEDIR=./build/platform/drm  # or gtk4
```

---

## Workflow

### Development (Desktop)
1. Build with GTK4: `meson setup build -Dplatforms=gtk4`
2. Run: `COG_MODULEDIR=./build/platform/gtk4 ./your-app`
3. Edit code, rebuild, test, repeat

### Testing (Embedded Mode)
1. Build with DRM: `meson setup build -Dplatforms=drm`
2. Stop desktop: `sudo systemctl stop gdm`
3. Run: `COG_MODULEDIR=./build/platform/drm sudo ./your-app`
4. Restart desktop: `sudo systemctl start gdm`

### Production (Embedded Device)
1. Build release: `meson setup build -Dplatforms=drm -Dbuildtype=release`
2. Deploy to target device
3. Run on target (no desktop needed)

---

## Environment Variables

| Variable | Purpose | Example |
|----------|---------|---------|
| `COG_MODULEDIR` | Platform plugin location | `./build/platform/drm` |
| `G_MESSAGES_DEBUG` | Enable debug logging | `all` |
| `DISPLAY` | X11 display (GTK4) | `:0` |
| `WAYLAND_DISPLAY` | Wayland display (GTK4) | `wayland-0` |

---

## Key Files

| File | Purpose |
|------|---------|
| `PLATFORM_GUIDE.md` | Complete platform comparison |
| `DUAL_PLATFORM_GUIDE.md` | Build & test with both platforms |
| `DRM_TROUBLESHOOTING.md` | DRM-specific issues |
| `BUILD_AND_RUN.md` | Detailed build instructions |
| `GETTING_STARTED.md` | Tutorial for beginners |

---

## Which Platform Should I Use?

```
┌─────────────────────────────────┐
│  Do you want to show GUI?       │
└────────────┬────────────────────┘
             │
    ┌────────▼─────────┐
    │   YES            │
    └────────┬─────────┘
             │
    ┌────────▼──────────────────────────┐
    │  Is this for embedded/production? │
    └────────┬──────────────────────────┘
             │
      ┌──────┴──────┐
      │             │
    YES            NO
      │             │
      ▼             ▼
   ┌─────┐      ┌──────┐
   │ DRM │      │ GTK4 │
   └─────┘      └──────┘
   Embedded     Desktop Dev
```

**Quick Answer:**
- **Raspberry Pi, kiosk, HMI?** → DRM
- **Desktop development?** → GTK4
- **Both?** → Build with `-Dplatforms=drm,gtk4`

---

## One-Line Builds

```bash
# Minimal DRM
meson setup build -Dplatforms=drm -Dexamples=true -Dlibportal=disabled && ninja -C build

# Minimal GTK4
meson setup build -Dplatforms=gtk4 -Dexamples=true && ninja -C build

# Both platforms
meson setup build -Dplatforms=drm,gtk4 -Dexamples=true && ninja -C build

# All three (DRM, GTK4, Headless)
meson setup build -Dplatforms=drm,gtk4,headless -Dexamples=true && ninja -C build
```

---

## Raspberry Pi Quick Start

```bash
# 1. Enable DRM/KMS
sudo nano /boot/firmware/config.txt
# Add: dtoverlay=vc4-kms-v3d
sudo reboot

# 2. Install dependencies
sudo apt-get update
sudo apt-get install -y \
    build-essential meson ninja-build \
    libwpewebkit-1.0-dev libwpe-1.0-dev \
    libglib2.0-dev libjson-glib-dev \
    libdrm-dev libgbm-dev libinput-dev libudev-dev libepoxy-dev

# 3. Build
cd cog
meson setup build -Dplatforms=drm -Dexamples=true -Dlibportal=disabled
ninja -C build

# 4. Setup permissions
sudo usermod -a -G video,input pi

# 5. Logout and login, then run
COG_MODULEDIR=./build/platform/drm ./build/cogbridge/examples/cogbridge-minimal
```

---

**For more details, see [DUAL_PLATFORM_GUIDE.md](DUAL_PLATFORM_GUIDE.md)**