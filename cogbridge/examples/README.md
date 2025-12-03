# CogBridge Examples

Minimal and advanced examples demonstrating CogBridge functionality.

## Quick Start

```bash
# Build all platforms
./build_examples.sh

# Build specific platforms
./build_examples.sh drm headless

# Run the standalone executables
../build/cogbridge/examples/cogbridge-minimal-drm
../build/cogbridge/examples/cogbridge-minimal-headless
../build/cogbridge/examples/cogbridge-dashboard-gtk4
```

## Examples

- **minimal.c** - Truly minimal example demonstrating core CogBridge features (~180 lines)
- **dashboard.c** - Advanced embedded system simulation with rich UI (~540 lines)

See [docs/EXAMPLES.md](../docs/EXAMPLES.md) for detailed documentation.

## How Platform Selection Works

CogBridge examples use **compile-time platform selection**. Each example is compiled into separate executables for each platform:

### Build Output

```
build/cogbridge/examples/
├── cogbridge-minimal-drm          # DRM platform (framebuffer)
├── cogbridge-minimal-headless     # Headless platform (no display)
├── cogbridge-minimal-gtk4         # GTK4 platform (desktop window)
├── cogbridge-dashboard-drm
├── cogbridge-dashboard-headless
└── cogbridge-dashboard-gtk4
```

### How It Works

1. **Build System** - The meson build system creates platform-specific binaries:
   - Each binary gets compile-time defines:
     - `COGBRIDGE_PLATFORM_DEFAULT` - Platform enum value (0=DRM, 1=Headless, 2=GTK4)
     - `COGBRIDGE_MODULE_DIR` - Absolute path to platform module directory
   
2. **Source Code** - Examples use these defines to configure themselves:
   ```c
   CogBridgeConfig config;
   cogbridge_get_default_config(&config);
   
   #ifdef COGBRIDGE_PLATFORM_DEFAULT
   config.platform = COGBRIDGE_PLATFORM_DEFAULT;
   #endif
   #ifdef COGBRIDGE_MODULE_DIR
   config.module_dir = COGBRIDGE_MODULE_DIR;
   #endif
   
   cogbridge_init(&config, NULL);
   ```

3. **Runtime** - Each binary is **completely standalone**:
   - No `COG_MODULEDIR` environment variable needed
   - No wrapper scripts required
   - Platform module path is compiled in
   - Just run the executable!

## Platforms

### DRM (Direct Rendering Manager)
- **Purpose:** Direct framebuffer rendering for embedded systems
- **Display:** Renders directly to HDMI/display hardware (no X11/Wayland)
- **Requirements:** DRM-capable GPU, may need sudo or video group membership
- **Best for:** Embedded systems, kiosks, appliances, Raspberry Pi

**Run:**
```bash
sudo ../build/cogbridge/examples/cogbridge-minimal-drm
```

### Headless
- **Purpose:** No visual output, for testing and automation
- **Display:** None (offscreen rendering only)
- **Requirements:** None
- **Best for:** CI/CD, automated testing, server-side rendering

**Run:**
```bash
../build/cogbridge/examples/cogbridge-minimal-headless
```

### GTK4
- **Purpose:** Desktop development window
- **Display:** GTK4 window in your desktop environment
- **Requirements:** X11 or Wayland display server, GTK4 libraries
- **Best for:** Development, debugging, testing on desktop

**Run:**
```bash
../build/cogbridge/examples/cogbridge-minimal-gtk4
```

## Building

### Simple Build (recommended)

```bash
# Build all platforms
./build_examples.sh

# Build specific platforms
./build_examples.sh drm
./build_examples.sh headless
./build_examples.sh drm headless gtk4
```

### Manual Build

```bash
# Setup meson build
meson setup ../build -Dplatforms=drm,headless,gtk4 -Dexamples=true -Dprograms=false

# Compile
ninja -C ../build

# Run
sudo ../build/cogbridge/examples/cogbridge-minimal-drm
```

## Why This Design?

### Old Way (Environment Variables)
```bash
# Build
meson setup build -Dplatforms=drm
ninja -C build

# Run (environment variable required!)
COG_MODULEDIR=./build/platform/drm ./build/cogbridge/examples/cogbridge-minimal
```

**Problems:**
- ❌ Must set `COG_MODULEDIR` every time
- ❌ Easy to use wrong module path
- ❌ Requires wrapper scripts or documentation
- ❌ Not truly standalone

### New Way (Compile-Time Selection)
```bash
# Build
./build_examples.sh drm

# Run (no environment variables!)
./build/cogbridge/examples/cogbridge-minimal-drm
```

**Benefits:**
- ✅ No environment variables needed
- ✅ Module paths always correct (compiled in)
- ✅ Platform explicit in binary name
- ✅ True standalone executables
- ✅ Simpler to use and distribute

## Implementation Details

### Build System (meson.build)

```meson
platforms_config = {
    'drm': {
        'enum_value': '0',  # COGBRIDGE_PLATFORM_DRM
        'suffix': '-drm',
    },
    'headless': {
        'enum_value': '1',  # COGBRIDGE_PLATFORM_HEADLESS
        'suffix': '-headless',
    },
    'gtk4': {
        'enum_value': '2',  # COGBRIDGE_PLATFORM_GTK4
        'suffix': '-gtk4',
    },
}

foreach platform : build_platforms
    module_dir = build_root / 'platform' / platform
    
    platform_c_args = [
        '-DCOGBRIDGE_PLATFORM_DEFAULT=@0@'.format(enum_value),
        '-DCOGBRIDGE_MODULE_DIR="@0@"'.format(module_dir),
    ]
    
    executable('cogbridge-minimal' + suffix,
        'minimal.c',
        c_args: platform_c_args,
        ...
    )
endforeach
```

### API Changes

**New enum type:**
```c
typedef enum {
    COGBRIDGE_PLATFORM_DRM,      // 0
    COGBRIDGE_PLATFORM_HEADLESS, // 1
    COGBRIDGE_PLATFORM_GTK4,     // 2
    COGBRIDGE_PLATFORM_WAYLAND,  // 3
    COGBRIDGE_PLATFORM_X11,      // 4
    COGBRIDGE_PLATFORM_AUTO      // 5 (auto-detect)
} CogBridgePlatform;
```

**Updated config struct:**
```c
typedef struct {
    // ...existing fields...
    CogBridgePlatform  platform;      // New enum field
    const char        *platform_name; // Deprecated string field
    const char        *module_dir;    // Module directory path
} CogBridgeConfig;
```

## Troubleshooting

### Example doesn't run
1. Make sure you built the platform: `./build_examples.sh drm`
2. Check the binary exists: `ls -l ../build/cogbridge/examples/`
3. For DRM, may need sudo: `sudo ./cogbridge-minimal-drm`
4. For GTK4, ensure display server running: `echo $DISPLAY`

### Module not found error
This shouldn't happen with compile-time selection! If you see module errors:
1. Rebuild: `./build_examples.sh drm headless gtk4`
2. Check compile commands: `cat ../build/compile_commands.json | grep COGBRIDGE_MODULE_DIR`
3. Verify defines in binary: `strings ../build/cogbridge/examples/cogbridge-minimal-drm | grep platform`

### Wrong platform loads
Each binary should only load its designated platform. If wrong platform loads:
1. Check you're running the right binary (not `cogbridge-minimal-drm` when you want headless)
2. Rebuild to ensure defines are correct
3. Check source code isn't overriding `config.platform`

## Documentation

- **[EXAMPLES.md](../docs/EXAMPLES.md)** - Detailed example documentation and usage
- **[QUICK_REFERENCE.md](../docs/QUICK_REFERENCE.md)** - API reference
- **[DRM_FRAMEBUFFER_ISSUE.md](../docs/DRM_FRAMEBUFFER_ISSUE.md)** - DRM troubleshooting (Raspberry Pi 5)
- **[FAQ.md](../docs/FAQ.md)** - Frequently asked questions

## Contributing

When adding new examples:
1. Ensure they work with compile-time platform selection
2. Use the `#ifdef COGBRIDGE_PLATFORM_DEFAULT` pattern shown above
3. Test on all three platforms (drm, headless, gtk4)
4. Update this README and EXAMPLES.md

---

**Questions?** See [../docs/FAQ.md](../docs/FAQ.md) or [../docs/EXAMPLES.md](../docs/EXAMPLES.md)