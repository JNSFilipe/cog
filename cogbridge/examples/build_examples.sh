#!/bin/bash
# build_examples.sh
# Build CogBridge examples for different platforms (DRM, Headless, GTK4)
#
# Usage:
#   ./build_examples.sh              # Build for all platforms
#   ./build_examples.sh drm          # Build only DRM platform
#   ./build_examples.sh headless     # Build only Headless platform
#   ./build_examples.sh gtk4         # Build only GTK4 platform
#   ./build_examples.sh drm headless # Build multiple platforms

set -e

# Determine script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)/cog"

# Color output helpers
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

info() {
    echo -e "${BLUE}[INFO]${NC} $*"
}

success() {
    echo -e "${GREEN}[SUCCESS]${NC} $*"
}

warn() {
    echo -e "${YELLOW}[WARN]${NC} $*"
}

error() {
    echo -e "${RED}[ERROR]${NC} $*"
}

# Main script
echo "========================================"
echo " CogBridge Examples Build Script"
echo "========================================"
echo ""
echo "Project root: $PROJECT_ROOT"
echo ""

# Parse command line arguments
PLATFORMS=()
if [ $# -eq 0 ]; then
    # No arguments - build all platforms
    PLATFORMS=("drm" "headless" "gtk4")
    info "No platform specified, building all: ${PLATFORMS[*]}"
else
    # Build specific platforms
    for arg in "$@"; do
        case "$arg" in
            drm|headless|gtk4)
                PLATFORMS+=("$arg")
                ;;
            *)
                error "Unknown platform: $arg"
                echo ""
                echo "Usage: $0 [drm|headless|gtk4]"
                echo ""
                echo "Platforms:"
                echo "  drm       - Direct Rendering Manager (framebuffer, no X/Wayland)"
                echo "  headless  - Headless mode (no visual output, for testing)"
                echo "  gtk4      - GTK4 desktop window (requires X/Wayland + GTK4)"
                echo ""
                echo "Examples:"
                echo "  $0                # Build all platforms"
                echo "  $0 drm            # Build only DRM"
                echo "  $0 drm headless   # Build DRM and Headless"
                exit 1
                ;;
        esac
    done
fi

# Convert array to comma-separated string for Meson
PLATFORMS_STR=$(IFS=,; echo "${PLATFORMS[*]}")

BUILD_DIR="$PROJECT_ROOT/build"

info "Building CogBridge examples for platforms: ${PLATFORMS[*]}"
info "Build directory: $BUILD_DIR"

# Setup meson build
if [ ! -d "$BUILD_DIR" ]; then
    info "Setting up meson build..."
    meson setup "$BUILD_DIR" "$PROJECT_ROOT" \
        -Dplatforms="$PLATFORMS_STR" \
        -Dexamples=true \
        -Dprograms=false \
        -Dlibportal=disabled \
        -Dlibmanette=disabled
else
    info "Build directory exists, reconfiguring..."
    meson configure "$BUILD_DIR" \
        -Dplatforms="$PLATFORMS_STR" \
        -Dexamples=true \
        -Dprograms=false \
        -Dlibportal=disabled \
        -Dlibmanette=disabled
fi

# Build
info "Compiling examples..."
ninja -C "$BUILD_DIR"

# Verify examples were built
info "Built examples:"
echo ""
for platform in "${PLATFORMS[@]}"; do
    echo "Platform: $platform"
    ls -lh "$BUILD_DIR/cogbridge/examples/cogbridge-"*"-$platform" 2>/dev/null || warn "  No examples found for $platform"
    echo ""
done

success "Build complete!"
echo ""
echo "========================================"
echo " Build Summary"
echo "========================================"
echo ""
echo "Build directory: $BUILD_DIR"
echo "Built platforms: ${PLATFORMS[*]}"
echo ""
echo "To run examples:"
for platform in "${PLATFORMS[@]}"; do
    echo "  $BUILD_DIR/cogbridge/examples/cogbridge-minimal-$platform"
    echo "  $BUILD_DIR/cogbridge/examples/cogbridge-dashboard-$platform"
done
echo ""
echo "Examples are standalone executables - no environment variables needed!"
echo ""
