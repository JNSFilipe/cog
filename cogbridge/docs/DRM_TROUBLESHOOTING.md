# DRM Platform Troubleshooting Guide

This guide helps you troubleshoot issues when running CogBridge with the DRM platform (direct framebuffer rendering).

## ✅ Raspberry Pi 5 Fix Applied

**Good news:** The framebuffer creation issue on Raspberry Pi 5 has been **RESOLVED** with shadow buffer implementation!

If you're on Raspberry Pi 5 and seeing garbled display or horizontal lines:
- The fix is already in the codebase (as of December 3, 2024)
- Shadow buffers automatically convert tiled GPU buffers to linear format
- No configuration needed - works automatically
- See **[DRM_FRAMEBUFFER_ISSUE.md](DRM_FRAMEBUFFER_ISSUE.md)** for technical details

**If you still have issues after rebuilding, continue with this troubleshooting guide.**

---

## Quick Checklist

Before diving into detailed troubleshooting:

- [ ] Built with `-Dplatforms=drm`
- [ ] Set `COG_MODULEDIR` to DRM platform directory
- [ ] Running on Linux with DRM/KMS support
- [ ] User has permissions for `/dev/dri/*` and `/dev/input/*`
- [ ] No other application is using the display (X11/Wayland stopped)
- [ ] Kernel has DRM drivers loaded

## Common Issues and Solutions

### 1. "Failed to open DRM device"

**Symptoms:**
```
** ERROR **: Failed to open DRM device /dev/dri/card0: Permission denied
```

**Cause:** User doesn't have permission to access DRM devices.

**Solutions:**

#### Option A: Run as root (Quick Test)
```bash
sudo ./build/cogbridge/examples/cogbridge-minimal
```

#### Option B: Add user to video group (Recommended)
```bash
# Add your user to the video group
sudo usermod -a -G video $USER

# Verify group membership
groups $USER

# Log out and log back in for changes to take effect
# Or use: su - $USER
```

#### Option C: Set device permissions (Temporary)
```bash
# Make DRM devices accessible (reset on reboot)
sudo chmod 666 /dev/dri/card*
sudo chmod 666 /dev/dri/renderD*

# Now run as regular user
./build/cogbridge/examples/cogbridge-minimal
```

#### Option D: Create udev rule (Permanent)
```bash
# Create udev rule
sudo tee /etc/udev/rules.d/99-drm.rules << EOF
KERNEL=="card[0-9]*", SUBSYSTEM=="drm", MODE="0666"
KERNEL=="renderD[0-9]*", SUBSYSTEM=="drm", MODE="0666"
EOF

# Reload udev rules
sudo udevadm control --reload-rules
sudo udevadm trigger

# Verify
ls -l /dev/dri/
```

---

### 2. "No suitable display found" or "No connector found"

**Symptoms:**
```
** ERROR **: No suitable display found
** ERROR **: No connector found
```

**Cause:** No active display connected, or DRM driver not loaded.

**Solutions:**

#### Check connected displays
```bash
# List DRM devices
ls -l /dev/dri/

# Show detailed DRM info (requires root)
sudo modetest -c

# Expected output shows connectors and modes
```

#### Verify DRM driver is loaded
```bash
# Check for DRM modules
lsmod | grep drm

# Expected output (example):
# drm_kms_helper
# drm
# i915 (Intel) or amdgpu (AMD) or nouveau (NVIDIA)

# If no output, load your GPU driver
sudo modprobe i915      # Intel
sudo modprobe amdgpu    # AMD
sudo modprobe nouveau   # NVIDIA (open source)
```

#### Check kernel support
```bash
# Verify DRM is compiled into kernel
grep -i drm /boot/config-$(uname -r)

# Should see:
# CONFIG_DRM=y (or =m)
# CONFIG_DRM_KMS_HELPER=y
```

---

### 3. Input devices not working (keyboard/mouse)

**Symptoms:**
- Display works but can't interact with buttons
- Keyboard input not registered

**Cause:** No permission to access `/dev/input/*` devices.

**Solutions:**

#### Add user to input group
```bash
# Add user to input group
sudo usermod -a -G input $USER

# Verify
groups $USER | grep input

# Log out and log back in
```

#### Set input permissions (Temporary)
```bash
sudo chmod 666 /dev/input/event*
```

#### Create udev rule (Permanent)
```bash
sudo tee /etc/udev/rules.d/99-input.rules << EOF
KERNEL=="event[0-9]*", SUBSYSTEM=="input", MODE="0660", GROUP="input"
KERNEL=="mice", SUBSYSTEM=="input", MODE="0660", GROUP="input"
KERNEL=="mouse[0-9]*", SUBSYSTEM=="input", MODE="0660", GROUP="input"
EOF

sudo udevadm control --reload-rules
sudo udevadm trigger
```

---

### 4. "COG_MODULEDIR not set" or "Failed to load platform module"

**Symptoms:**
```
** ERROR **: Failed to load platform module
Platform 'drm' not found
```

**Cause:** CogBridge can't find the DRM platform plugin.

**Solutions:**

#### Set COG_MODULEDIR
```bash
# When running from build directory
export COG_MODULEDIR=./build/platform/drm

# When installed system-wide
export COG_MODULEDIR=/usr/local/lib/cog/modules
# or
export COG_MODULEDIR=/usr/lib/cog/modules
```

#### Verify platform plugin exists
```bash
# Check if DRM plugin was built
ls -l ./build/platform/drm/

# Should see: cogplatform-drm.so

# If missing, rebuild with DRM platform
meson setup build -Dplatforms=drm --wipe
ninja -C build
```

---

### 5. Black screen or garbled display (Raspberry Pi 5)

**Symptoms:**
- Application starts without errors
- Screen shows horizontal lines or garbled colors
- Or screen is blank/black
- Console shows "Page loaded" but incorrect display

**Solution (Raspberry Pi 5 specific):**

This was a buffer format issue that is now **FIXED** with shadow buffer implementation.

#### If you're seeing this issue:
```bash
# Ensure you have the latest code with the fix
cd cog
git pull  # if using git

# Clean rebuild to ensure fix is included
rm -rf build
meson setup build -Dplatforms=drm -Dlibportal=disabled -Dexamples=true
ninja -C build

# Run with debug to verify shadow buffers are being used
sudo COG_MODULEDIR=./build/platform/drm G_MESSAGES_DEBUG=all \
  ./build/cogbridge/examples/cogbridge-minimal 2>&1 | grep shadow
```

#### Expected debug output (working):
```
Creating linear shadow buffer for tiled source (modifier 0x700000000000006)
Copied 1920x1080 pixels from tiled to linear buffer
```

If you see this output, the fix is working correctly!

**For other causes of black screen:**

#### HTML content not loaded
```bash
# Enable debug logging
export G_MESSAGES_DEBUG=all
./build/cogbridge/examples/cogbridge-minimal

# Check for WebKit errors in output
```

#### WebKit rendering issue
```c
// In your code, verify HTML is valid
const char *html = "<!DOCTYPE html><html>...";

// Ensure base_uri is set
cogbridge_load_html(bridge, html, "file:///");
```

#### Display resolution mismatch
```c
// Try setting explicit resolution
CogBridgeConfig config;
cogbridge_get_default_config(&config);
config.width = 1920;   // Match your display
config.height = 1080;
cogbridge_init(&config, NULL);
```

---

### 6. "Another program is using the display"

**Symptoms:**
```
** ERROR **: Device or resource busy
** ERROR **: Failed to set CRTC
```

**Cause:** X11, Wayland, or another application is using the display.

**Solutions:**

#### Stop display server
```bash
# Stop X11
sudo systemctl stop gdm         # GNOME
sudo systemctl stop lightdm     # LightDM
sudo systemctl stop sddm        # KDE

# Or stop from console (Ctrl+Alt+F1-F6)
sudo systemctl isolate multi-user.target

# Verify no X/Wayland running
ps aux | grep -E "Xorg|wayland"
```

#### Use different VT (Virtual Terminal)
```bash
# Switch to different VT
sudo chvt 2

# Run your application from SSH or serial console
ssh user@embedded-device
./your-cogbridge-app
```

#### Kill conflicting processes
```bash
# Find processes using DRM
sudo fuser /dev/dri/card0

# Kill them (be careful!)
sudo fuser -k /dev/dri/card0
```

---

### 7. Missing dependencies / Build errors

**Symptoms:**
```
ERROR: Dependency "libdrm" not found
ERROR: Dependency "libgbm" not found
```

**Solutions:**

#### Install all DRM dependencies
```bash
# Ubuntu/Debian
sudo apt-get install -y \
    libdrm-dev libgbm-dev libinput-dev \
    libudev-dev libepoxy-dev \
    libwpewebkit-1.0-dev libwpe-1.0-dev \
    libglib2.0-dev libjson-glib-dev

# Fedora/RHEL/CentOS
sudo dnf install -y \
    libdrm-devel mesa-libgbm-devel libinput-devel \
    systemd-devel libepoxy-devel \
    wpewebkit-devel wpe-devel \
    glib2-devel json-glib-devel

# Arch Linux
sudo pacman -S libdrm mesa libinput systemd-libs libepoxy \
    wpewebkit wpebackend-fdo glib2 json-glib
```

---

### 8. Application crashes or segfaults

**Symptoms:**
```
Segmentation fault (core dumped)
```

**Debug Steps:**

#### Run with GDB
```bash
# Build with debug symbols
meson configure build -Dbuildtype=debug
ninja -C build

# Run under debugger
gdb ./build/cogbridge/examples/cogbridge-minimal
(gdb) run
# ... crash occurs ...
(gdb) backtrace
```

#### Enable core dumps
```bash
ulimit -c unlimited
./build/cogbridge/examples/cogbridge-minimal

# If crash occurs, inspect core dump
gdb ./build/cogbridge/examples/cogbridge-minimal core
```

#### Check library versions
```bash
# Verify WPE WebKit version
pkg-config --modversion wpe-webkit-1.0

# Should be >= 2.28.0

# Check linked libraries
ldd ./build/cogbridge/examples/cogbridge-minimal
```

---

### 9. Performance issues / Slow rendering

**Symptoms:**
- Low frame rate
- Laggy UI
- High CPU usage

**Solutions:**

#### Enable hardware acceleration
```bash
# Ensure Mesa drivers are installed
sudo apt-get install mesa-utils

# Check OpenGL support
glxinfo | grep "OpenGL version"

# Verify EGL/GLES
eglinfo
```

#### Check for software rendering
```bash
# Run with debug
export LIBGL_DEBUG=verbose
./your-app

# If you see "llvmpipe" or "swrast", you're using software rendering
# Install proper GPU drivers
```

#### Optimize display mode
```bash
# Check current display mode
sudo modetest -c

# Force specific refresh rate in code
# (DRM platform supports mode selection)
```

---

## Raspberry Pi Specific Issues

### Raspberry Pi 5 - Shadow Buffer Fix

**Issue:** Tiled buffer format incompatibility (RESOLVED)

The Raspberry Pi 5's VideoCore VII GPU produces buffers with Broadcom UIF tiled format that the DRM driver doesn't accept for direct scanout. This is now automatically handled by shadow buffer conversion.

**Verification:**
```bash
# Run with debug logging
sudo COG_MODULEDIR=./build/platform/drm G_MESSAGES_DEBUG=all \
  ./build/cogbridge/examples/cogbridge-minimal 2>&1 | grep -E "shadow|modifier|Copied"

# You should see:
# Creating linear shadow buffer for tiled source (modifier 0x700000000000006)
# Copied 1920x1080 pixels from tiled to linear buffer
```

**Performance Impact:**
- ~2-5ms per frame for 1920×1080
- ~16-24MB additional RAM for shadow buffers
- Minimal - acceptable for embedded applications

**Technical Details:**
See [DRM_FRAMEBUFFER_ISSUE.md](DRM_FRAMEBUFFER_ISSUE.md) for complete technical explanation.

### Enable DRM/KMS on Raspberry Pi

**For Raspberry Pi 4 and later:**

```bash
# Edit config.txt
sudo nano /boot/firmware/config.txt

# Ensure these lines are present and NOT commented:
dtoverlay=vc4-kms-v3d
max_framebuffers=2

# Reboot
sudo reboot

# Verify DRM is active
ls -l /dev/dri/
# Should see card0, card1, renderD128
```

### Raspberry Pi permissions
```bash
# Add user to required groups
sudo usermod -a -G video,input,render pi

# Logout and login again
```

---

## Advanced Debugging

### Enable maximum logging
```bash
export G_MESSAGES_DEBUG=all
export LIBGL_DEBUG=verbose
export EGL_LOG_LEVEL=debug
export WAYLAND_DEBUG=1
export COG_PLATFORMS_DIR=./build/platform

./build/cogbridge/examples/cogbridge-minimal 2>&1 | tee debug.log
```

### Verify DRM capabilities
```bash
# Install drm_info tool
sudo apt-get install drm-info

# Get detailed DRM information
drm_info
```

### Check for kernel messages
```bash
# Watch kernel logs for DRM errors
sudo dmesg -w | grep -i drm

# Common issues:
# - "Failed to initialize" - driver problem
# - "No modes available" - no display connected
# - "flip_done timed out" - performance issue
```

---

## Testing DRM Without CogBridge

To verify your DRM setup works independently:

```bash
# Install kmscube (simple DRM test)
sudo apt-get install kmscube

# Run it
kmscube

# Should show a spinning cube on your display
# If this works, DRM is properly configured
```

---

## System Requirements Summary

### Minimum Requirements
- Linux kernel with DRM/KMS support (>= 4.4)
- GPU with DRM driver (Intel/AMD/NVIDIA/VC4/etc)
- Mesa >= 17.0 (for EGL/GBM)
- WPE WebKit >= 2.28
- libdrm >= 2.4.71

### Recommended Requirements
- Linux kernel >= 5.10
- Modern GPU with OpenGL ES 3.0+ support
- Mesa >= 21.0
- 2GB+ RAM
- Hardware video decode support

---

## Getting Help

If none of these solutions work:

1. **Gather system information:**
   ```bash
   # Create a system report
   {
     echo "=== System Info ==="
     uname -a
     echo "=== DRM Devices ==="
     ls -l /dev/dri/
     echo "=== Loaded Modules ==="
     lsmod | grep -E "drm|video"
     echo "=== Display Info ==="
     sudo modetest -c
     echo "=== Groups ==="
     groups
     echo "=== Package Versions ==="
     dpkg -l | grep -E "wpe|webkit|drm|gbm|mesa"
   } > system-report.txt
   ```

2. **Run with full debugging:**
   ```bash
   G_MESSAGES_DEBUG=all \
   COG_MODULEDIR=./build/platform/drm \
   ./build/cogbridge/examples/cogbridge-minimal 2>&1 | tee full-debug.log
   ```

3. **Report the issue with:**
   - Your `system-report.txt`
   - Your `full-debug.log`
   - Your hardware (CPU, GPU, display)
   - Your Linux distribution and version

---

## Success Indicators

You'll know DRM mode is working correctly when:

✅ Application starts without permission errors
✅ Display shows rendered HTML content (not garbled or black)
✅ Keyboard/mouse input works
✅ No "failed to open" errors in logs
✅ Console shows "Page loaded and ready"
✅ UI is responsive and smooth
✅ On Raspberry Pi 5: Shadow buffer debug messages appear (if G_MESSAGES_DEBUG=all)

Example successful output:
```
** Message: CogBridge initialized successfully
** Message: CogBridge instance created: minimal-example
** Message: Bound function: add
** Message: Bound function: greet
** Message: Page loaded and ready
** Message: DRM: Using device /dev/dri/card0
** Message: DRM: Display mode: 1920x1080@60Hz
[C] add(42, 58) = 100
```

### Raspberry Pi 5 Success Output:
```
(Cog-DRM-DEBUG): Creating linear shadow buffer for tiled source (modifier 0x700000000000006)
(Cog-DRM-DEBUG): Copied 1920x1080 pixels from tiled to linear buffer
** Message: CogBridge initialized successfully
** Message: Page loaded and ready
```

Good luck! 🚀