# CogBridge Examples build configuration

cogbridge_examples_deps = [
    cogbridge_dep,
    wpewebkit_dep,
    wpe_dep,
    gio_dep,
    dependency('json-glib-1.0'),
]

# Platform configuration map
# Note: enum values must match CogBridgePlatform in cogbridge.h
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

# Get the list of platforms to build
build_platforms = get_option('platforms')

# Build each example for each platform
foreach platform : build_platforms
    if platform in platforms_config
        platform_enum = platforms_config[platform]['enum_value']
        platform_suffix = platforms_config[platform]['suffix']

        # Calculate the absolute module directory for this platform at build time
        build_root = meson.project_build_root()
        module_dir = build_root / 'platform' / platform

        # Platform-specific compile arguments
        platform_c_args = [
            '-DCOGBRIDGE_PLATFORM_DEFAULT=@0@'.format(platform_enum),
            '-DCOGBRIDGE_MODULE_DIR="@0@"'.format(module_dir),
        ]

        # Minimal example - platform-specific binary
        executable('cogbridge-minimal' + platform_suffix,
            'minimal.c',
            dependencies: cogbridge_examples_deps,
            include_directories: [core_inc],
            c_args: platform_c_args,
            install: false,
        )

        # Dashboard example - platform-specific binary
        executable('cogbridge-dashboard' + platform_suffix,
            'dashboard.c',
            dependencies: cogbridge_examples_deps,
            include_directories: [core_inc],
            c_args: platform_c_args,
            install: false,
        )
    endif
endforeach
