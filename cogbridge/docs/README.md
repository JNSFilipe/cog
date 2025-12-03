# CogBridge Documentation

Welcome to the CogBridge documentation! This directory contains comprehensive guides, references, and troubleshooting information.

## 📚 Quick Navigation

### 🚀 Getting Started
- **[GETTING_STARTED.md](GETTING_STARTED.md)** - Step-by-step tutorial for your first CogBridge application
- **[BUILD_AND_RUN.md](BUILD_AND_RUN.md)** - Build instructions and running examples

### 🎯 Platform Selection
**New to CogBridge? Start here to understand platform choices:**

- **[PLATFORM_GUIDE.md](PLATFORM_GUIDE.md)** - Complete guide: DRM vs Headless vs GTK4 ⭐ **READ THIS FIRST**
- **[QUICK_PLATFORM_REFERENCE.md](QUICK_PLATFORM_REFERENCE.md)** - Quick reference card with all platform commands
- **[DUAL_PLATFORM_GUIDE.md](DUAL_PLATFORM_GUIDE.md)** - Build and test with multiple platforms
- **[HEADLESS_VS_GUI.md](HEADLESS_VS_GUI.md)** - Understanding "headless" terminology

### 📖 Reference Documentation
- **[OVERVIEW.md](OVERVIEW.md)** - Architecture, design patterns, and technical deep-dive
- **[QUICK_REFERENCE.md](QUICK_REFERENCE.md)** - API reference and code examples
- **[EXAMPLES.md](EXAMPLES.md)** - Guide to the streamlined examples (minimal & dashboard)
- **[FAQ.md](FAQ.md)** - Frequently asked questions and common issues

### 🔧 Troubleshooting & Solutions
- **[DRM_TROUBLESHOOTING.md](DRM_TROUBLESHOOTING.md)** - DRM platform troubleshooting guide
- **[DRM_FRAMEBUFFER_ISSUE.md](DRM_FRAMEBUFFER_ISSUE.md)** - Raspberry Pi 5 framebuffer fix (RESOLVED) ✅
- **[DRM_SCALE_FACTOR_ISSUE.md](DRM_SCALE_FACTOR_ISSUE.md)** - Device scale factor fix (RESOLVED) ✅
- **[DRM_SUCCESS.md](DRM_SUCCESS.md)** - DRM platform success story

### 📝 Additional Resources
- **[TESTING.md](TESTING.md)** - Testing guide and test strategy
- **[PROJECT_SUMMARY.md](PROJECT_SUMMARY.md)** - Project overview and achievements
- **[BUILD_SUCCESS.md](BUILD_SUCCESS.md)** - Build verification and success indicators

## 🎯 Common Use Cases

### "I want to show a GUI on my embedded system (Raspberry Pi, etc.)"
→ Use **DRM platform** - Read [PLATFORM_GUIDE.md](PLATFORM_GUIDE.md)

### "I want to test without a display (CI/CD, automation)"
→ Use **Headless platform** - Read [PLATFORM_GUIDE.md](PLATFORM_GUIDE.md)

### "I want to develop on my desktop/laptop"
→ Use **GTK4 platform** - Read [DUAL_PLATFORM_GUIDE.md](DUAL_PLATFORM_GUIDE.md)

### "I'm having issues on Raspberry Pi 5"
→ See [DRM_FRAMEBUFFER_ISSUE.md](DRM_FRAMEBUFFER_ISSUE.md) - Already fixed! ✅

## 📋 Documentation by Topic

### Installation & Setup
1. [GETTING_STARTED.md](GETTING_STARTED.md) - First-time setup
2. [BUILD_AND_RUN.md](BUILD_AND_RUN.md) - Build system details
3. [PLATFORM_GUIDE.md](PLATFORM_GUIDE.md) - Platform selection

### Development
1. [QUICK_REFERENCE.md](QUICK_REFERENCE.md) - API and code examples
2. [EXAMPLES.md](EXAMPLES.md) - Code examples (minimal & dashboard)
3. [OVERVIEW.md](OVERVIEW.md) - Architecture and design patterns
4. [TESTING.md](TESTING.md) - Testing your application

### Troubleshooting
1. [DRM_TROUBLESHOOTING.md](DRM_TROUBLESHOOTING.md) - Common DRM issues
2. [FAQ.md](FAQ.md) - Frequently asked questions
3. [DRM_FRAMEBUFFER_ISSUE.md](DRM_FRAMEBUFFER_ISSUE.md) - Raspberry Pi 5 specifics

## 🎉 Recent Updates

### ✅ December 2024 - Raspberry Pi 5 Full Support
- **DRM platform now works perfectly on Raspberry Pi 5**
- Shadow buffer implementation for tiled GPU format
- Automatic conversion with zero configuration
- See [DRM_FRAMEBUFFER_ISSUE.md](DRM_FRAMEBUFFER_ISSUE.md) for details

### ✅ Multi-Platform Build System
- Build DRM, Headless, and GTK4 platforms simultaneously
- Runtime platform selection
- See [DUAL_PLATFORM_GUIDE.md](DUAL_PLATFORM_GUIDE.md)

### ✅ Complete Documentation
- Platform selection guide
- Troubleshooting guides
- API reference
- Testing guide

## 🔍 Finding What You Need

### By Experience Level

**Beginner:**
1. Start with [GETTING_STARTED.md](GETTING_STARTED.md)
2. Read [PLATFORM_GUIDE.md](PLATFORM_GUIDE.md) to choose platform
3. Review [QUICK_REFERENCE.md](QUICK_REFERENCE.md) for API basics

**Intermediate:**
1. Read [OVERVIEW.md](OVERVIEW.md) for architecture
2. Check [DUAL_PLATFORM_GUIDE.md](DUAL_PLATFORM_GUIDE.md) for multi-platform builds
3. Review [FAQ.md](FAQ.md) for common patterns

**Advanced:**
1. Study [OVERVIEW.md](OVERVIEW.md) for deep technical details
2. Review [DRM_FRAMEBUFFER_ISSUE.md](DRM_FRAMEBUFFER_ISSUE.md) for implementation details
3. Check [TESTING.md](TESTING.md) for testing strategies

### By Problem

**"It won't build"**
→ [BUILD_AND_RUN.md](BUILD_AND_RUN.md)

**"It builds but won't run"**
→ [DRM_TROUBLESHOOTING.md](DRM_TROUBLESHOOTING.md)

**"Black screen or garbled display"**
→ [DRM_FRAMEBUFFER_ISSUE.md](DRM_FRAMEBUFFER_ISSUE.md) (Fixed!)

**"I don't understand platforms"**
→ [PLATFORM_GUIDE.md](PLATFORM_GUIDE.md)

**"Permission denied errors"**
→ [DRM_TROUBLESHOOTING.md](DRM_TROUBLESHOOTING.md) Section 1

**"My JavaScript/C communication isn't working"**
→ [QUICK_REFERENCE.md](QUICK_REFERENCE.md) + [FAQ.md](FAQ.md)

## 🌟 Key Concepts

### Platform Types
- **DRM**: Direct framebuffer rendering (embedded with display)
- **Headless**: No display output (testing, automation)
- **GTK4**: Desktop window (development)

### Architecture
- **CogBridge**: High-level C API for JavaScript ↔ C communication
- **Cog**: WPE WebKit launcher (embedded)
- **WPE WebKit**: WebKit engine for embedded systems
- **DRM/KMS**: Direct Rendering Manager for graphics

### Important Files
- `cogbridge.h` - Public API header
- `cog-platform-drm.c` - DRM platform implementation
- `cog-drm-modeset-renderer.c` - Framebuffer rendering (includes shadow buffer fix)

## 📞 Getting Help

1. **Check the FAQ**: [FAQ.md](FAQ.md)
2. **Search this docs folder**: Use grep or your editor
3. **Enable debug logging**: `G_MESSAGES_DEBUG=all`
4. **Report issues**: Include system info and logs

## 🤝 Contributing

Found a documentation issue or want to improve these docs?
- Documentation should be clear, accurate, and helpful
- Code examples should be tested and work
- Keep the structure organized and easy to navigate

## 📄 License

CogBridge and its documentation are released under the MIT License.

---

**Ready to start?** → [GETTING_STARTED.md](GETTING_STARTED.md)

**Need to choose a platform?** → [PLATFORM_GUIDE.md](PLATFORM_GUIDE.md)

**Already know what you're doing?** → [QUICK_REFERENCE.md](QUICK_REFERENCE.md)