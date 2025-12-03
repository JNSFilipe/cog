# CogBridge library build configuration

cogbridge_sources = [
    'cogbridge.c',
]

cogbridge_headers = [
    'cogbridge.h',
]

cogbridge_deps = [
    wpewebkit_dep,
    wpe_dep,
    gio_dep,
    dependency('json-glib-1.0'),
]

cogbridge_lib = library('cogbridge',
    cogbridge_sources,
    dependencies: cogbridge_deps + [cogcore_lib_dep],
    include_directories: [core_inc],
    version: cogcore_soversion,
    install: true,
)

cogbridge_dep = declare_dependency(
    link_with: cogbridge_lib,
    dependencies: cogbridge_deps,
    include_directories: include_directories('.'),
)

install_headers(cogbridge_headers, subdir: 'cogbridge')

# Generate pkg-config file
pkg = import('pkgconfig')
pkg.generate(
    cogbridge_lib,
    description: 'CogBridge - Headless embedded WebView framework for C',
    name: 'cogbridge',
    filebase: 'cogbridge',
    subdirs: 'cogbridge',
)

# Build examples if requested
if get_option('examples')
    subdir('examples')
endif
