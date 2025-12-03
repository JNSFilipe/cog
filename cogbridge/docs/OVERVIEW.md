# CogBridge Framework Overview

## What is CogBridge?

CogBridge is a lightweight, headless WebView framework designed specifically for embedded systems. It bridges the gap between low-level C hardware control and modern web-based user interfaces, enabling developers to create rich, interactive dashboards and control panels using HTML/CSS/JavaScript while maintaining full control over hardware through C code.

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                   Embedded System Hardware                   │
│  (Sensors, GPIO, Motors, Displays, Network Interfaces, etc.) │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                   Your C Application                         │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  Hardware Control Layer                              │   │
│  │  - GPIO control                                      │   │
│  │  - Sensor reading (I2C, SPI, etc.)                  │   │
│  │  - Motor control                                     │   │
│  │  - Network communication                             │   │
│  └──────────────────────────────────────────────────────┘   │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                      CogBridge API                           │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  Function Binding (C → JavaScript)                   │   │
│  │  Script Execution (C → JavaScript)                   │   │
│  │  Event Emission (C → JavaScript)                     │   │
│  │  Callback Handling (JavaScript → C)                  │   │
│  └──────────────────────────────────────────────────────┘   │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                   WebKit WPE Engine                          │
│  (Headless browser engine optimized for embedded systems)    │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────────┐
│              HTML/CSS/JavaScript Application                 │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  User Interface                                      │   │
│  │  - Dashboards                                        │   │
│  │  - Control panels                                    │   │
│  │  - Data visualization                                │   │
│  │  - Real-time monitoring                              │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

## Key Components

### 1. CogBridge Core Library (`libcogbridge`)

The main library that provides:

- **Bridge Management**: Initialize and manage WebView instances
- **Function Binding**: Expose C functions to JavaScript
- **Script Execution**: Execute JavaScript from C code
- **Event System**: Publish-subscribe pattern for C→JS communication
- **Message Passing**: Efficient bidirectional communication via WebKit user messages

### 2. WebKit WPE Engine

- Mature, standards-compliant web rendering engine
- Optimized for embedded and low-resource environments
- Supports modern web technologies (HTML5, CSS3, ES6+)
- Hardware acceleration support
- Small memory footprint compared to Chromium

### 3. JavaScript API (`window.cogbridge`)

Automatically injected JavaScript object providing:

```javascript
// Call C functions
const result = await window.cogbridge.functionName(args...);

// Listen for C events
window.cogbridge.on('event_name', (data) => { ... });
```

## Communication Patterns

### Pattern 1: JavaScript Calls C Function

```
┌─────────────┐                                    ┌─────────────┐
│             │  window.cogbridge.read_sensor()    │             │
│  JavaScript │───────────────────────────────────▶│   CogBridge │
│             │                                    │    Core     │
│             │                                    │             │
└─────────────┘                                    └──────┬──────┘
                                                          │
                                                          │ Dispatch to
                                                          │ bound callback
                                                          ▼
                                                   ┌─────────────┐
      ┌────────────────────────────────────────────│ C Callback  │
      │                                            │  Function   │
      │                                            └─────────────┘
      │                                                   │
      │                                                   │ Read hardware
      │                                                   │ Return JSON
      │                                                   ▼
      │                                            ┌─────────────┐
      │                                            │  Hardware   │
      │                                            │    (I2C)    │
      │                                            └─────────────┘
      │
      │ Promise resolves with result
      ▼
┌─────────────┐
│  JavaScript │
│  receives   │
│    result   │
└─────────────┘
```

### Pattern 2: C Emits Event to JavaScript

```
┌─────────────┐                                    ┌─────────────┐
│  Hardware   │  Interrupt/Timer                   │   C Event   │
│   Trigger   │───────────────────────────────────▶│   Handler   │
└─────────────┘                                    └──────┬──────┘
                                                          │
                                                          │ Read sensor
                                                          │ Format JSON
                                                          ▼
                                                   ┌─────────────┐
                                                   │ cogbridge_  │
                                                   │ emit_event()│
                                                   └──────┬──────┘
                                                          │
                                                          │ Inject script
                                                          ▼
┌─────────────┐                                    ┌─────────────┐
│  JavaScript │◀───────────────────────────────────│  WebKit WPE │
│   Event     │  window.cogbridge._emit('event')   │    Engine   │
│  Listeners  │                                    └─────────────┘
└─────────────┘
```

### Pattern 3: C Executes JavaScript

```
┌─────────────┐                                    ┌─────────────┐
│     C       │  cogbridge_execute_script()        │  CogBridge  │
│  Application│───────────────────────────────────▶│    Core     │
└─────────────┘                                    └──────┬──────┘
                                                          │
                                                          │ webkit_web_view_
                                                          │ evaluate_javascript()
                                                          ▼
                                                   ┌─────────────┐
                                                   │  WebKit WPE │
                                                   │   Execute   │
                                                   │     JS      │
                                                   └──────┬──────┘
                                                          │
                                                          │ Result (optional)
                                                          ▼
┌─────────────┐                                    ┌─────────────┐
│     C       │◀───────────────────────────────────│  Callback   │
│  Callback   │  Result as JSON string             │  (async)    │
└─────────────┘                                    └─────────────┘
```

## Data Exchange Format

All data between C and JavaScript is exchanged as JSON:

### C → JavaScript

```c
// Return JSON from C callbacks
return g_strdup("{\"temperature\":25.5,\"unit\":\"celsius\"}");

// Emit events with JSON data
cogbridge_emit_event(bridge, "sensor_update", 
                    "{\"value\":42,\"status\":\"ok\"}");
```

### JavaScript → C

```javascript
// Arguments automatically serialized to JSON array
await window.cogbridge.set_motor_speed(75, "clockwise");
// C receives: ["75", "clockwise"] as JSON
```

## Memory Model

### C Side

- **Manual Memory Management**: Callbacks must return allocated strings
- **GLib Integration**: Use `g_strdup()`, `g_free()`, etc.
- **User Data**: Optional context pointers for callbacks
- **Destroy Notifiers**: Automatic cleanup of user data

```c
char *my_callback(CogBridge *bridge, const char *fn,
                  const char *args, void *user_data)
{
    // Allocate return value - caller will free
    return g_strdup("result");
}

// With user data
MyData *data = g_new0(MyData, 1);
cogbridge_bind_function(bridge, "func", callback, data, 
                       (GDestroyNotify)my_data_free);
```

### JavaScript Side

- **Automatic GC**: Standard JavaScript garbage collection
- **Promise-based**: All C function calls return Promises
- **Event-driven**: Event listeners managed by JavaScript

## Threading Model

CogBridge uses GLib's main loop (single-threaded event loop):

```
┌─────────────────────────────────────────────────────────┐
│                    Main Thread                           │
│  ┌───────────────────────────────────────────────────┐  │
│  │              GLib Main Loop                       │  │
│  │  ┌─────────────────────────────────────────────┐ │  │
│  │  │  - WebKit event processing                  │ │  │
│  │  │  - JavaScript execution                     │ │  │
│  │  │  - C callback invocation                    │ │  │
│  │  │  - Timer/idle handlers                      │ │  │
│  │  └─────────────────────────────────────────────┘ │  │
│  └───────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│                 Worker Threads (optional)                │
│  ┌───────────────────────────────────────────────────┐  │
│  │  - Hardware I/O                                   │  │
│  │  - Network operations                             │  │
│  │  - Heavy computation                              │  │
│  │                                                   │  │
│  │  Use g_idle_add() or g_timeout_add()            │  │
│  │  to communicate back to main thread              │  │
│  └───────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

### Thread Safety

- All CogBridge API calls must be made from the main thread
- Use `g_idle_add()` to marshal calls from worker threads

```c
// From worker thread
void *worker_thread(void *arg) {
    // Do work...
    g_idle_add((GSourceFunc)update_ui, result_data);
    return NULL;
}

// Will be called on main thread
gboolean update_ui(gpointer data) {
    cogbridge_execute_script(bridge, "updateDisplay()", NULL, NULL);
    return G_SOURCE_REMOVE;
}
```

## Performance Characteristics

### Memory Usage

| Component | Typical RAM Usage |
|-----------|------------------|
| WebKit WPE (base) | 40-80 MB |
| CogBridge library | ~500 KB |
| JavaScript heap | 5-20 MB (varies with content) |
| **Total minimum** | **~50 MB** |

### CPU Usage

- **Idle**: < 1% CPU (no animations)
- **Active UI**: 5-15% CPU (with animations/updates)
- **Heavy computation**: Depends on JavaScript workload

### Startup Time

- **Cold start**: 1-3 seconds (depends on hardware)
- **Page load**: 0.5-2 seconds (for typical dashboard)

### Optimization Tips

1. **Minimize JavaScript execution frequency**
2. **Use CSS animations over JavaScript**
3. **Batch DOM updates**
4. **Lazy load resources**
5. **Optimize images and assets**
6. **Use hardware acceleration when available**

## Deployment Scenarios

### Scenario 1: Industrial HMI

```
Hardware: ARM Cortex-A9, 512MB RAM, 800x480 display
Use Case: Touch-screen control panel for machinery
```

- Real-time sensor monitoring
- Manual control inputs
- Status indicators and alarms
- Data logging

### Scenario 2: IoT Gateway Dashboard

```
Hardware: ARM Cortex-A53, 1GB RAM, headless
Use Case: Web-based management interface
```

- Device status overview
- Configuration interface
- Network diagnostics
- Remote access via web browser

### Scenario 3: Digital Signage

```
Hardware: x86 Celeron, 2GB RAM, 1920x1080 display
Use Case: Interactive kiosk with hardware integration
```

- Dynamic content display
- Touch interaction
- Printer/scanner integration
- External sensor triggers

### Scenario 4: Test Equipment

```
Hardware: ARM Cortex-A72, 4GB RAM, headless
Use Case: Automated test system with web interface
```

- Instrument control
- Real-time measurements
- Data visualization
- Test result reporting

## Comparison with Alternatives

| Feature | CogBridge | Tauri | Electron | Qt WebEngine |
|---------|-----------|-------|----------|--------------|
| Target Platform | Embedded Linux | Desktop | Desktop/Server | Desktop/Embedded |
| Rendering Engine | WebKit | WebKit | Chromium | Chromium |
| Language | C | Rust | JavaScript | C++/QML |
| Memory Footprint | ~50 MB | ~80 MB | ~150 MB | ~100 MB |
| Headless Support | ✓ | ✗ | Limited | ✓ |
| Cross-platform | Linux | Win/Mac/Linux | Win/Mac/Linux | Win/Mac/Linux |
| Embedded Focus | ✓✓✓ | ✗ | ✗ | ✓ |
| Startup Time | Fast | Fast | Slow | Medium |
| Binary Size | Small | Medium | Large | Medium |

## Security Considerations

### Isolation

- JavaScript runs in WebKit sandbox
- C code has full system access
- No filesystem access from JavaScript by default

### Input Validation

- Always validate inputs from JavaScript
- Sanitize data before hardware operations
- Use JSON schema validation

```c
static char *set_gpio(CogBridge *bridge, const char *fn,
                      const char *args, void *user_data)
{
    // Parse and validate
    int pin = parse_pin(args);
    if (pin < 0 || pin > MAX_PIN) {
        return g_strdup("{\"error\":\"Invalid pin\"}");
    }
    
    // Safe to proceed
    gpio_set_value(pin, value);
    return g_strdup("{\"success\":true}");
}
```

### Best Practices

1. **Validate all inputs** from JavaScript
2. **Limit exposed functions** to only what's necessary
3. **Use HTTPS** for remote content
4. **Implement authentication** for sensitive operations
5. **Rate limit** high-frequency operations
6. **Log security events**

## Future Enhancements

### Planned Features

- [ ] TypeScript definitions for JavaScript API
- [ ] Rust bindings
- [ ] Python bindings
- [ ] WebSocket server for remote debugging
- [ ] Built-in HTTP server for static content
- [ ] Multi-view support (multiple web contexts)
- [ ] Custom URI schemes
- [ ] File upload/download support

### Community Contributions Welcome

- More language bindings
- Platform-specific optimizations
- Additional examples
- Documentation improvements
- Performance profiling tools

## Resources

### Documentation

- [README.md](README.md) - Full API reference
- [GETTING_STARTED.md](GETTING_STARTED.md) - Quick start guide
- [examples/](examples/) - Code examples

### Dependencies

- [WebKit WPE](https://wpewebkit.org/) - Web rendering engine
- [Cog](https://github.com/Igalia/cog) - Base WPE launcher
- [GLib](https://docs.gtk.org/glib/) - Core utilities
- [json-glib](https://gitlab.gnome.org/GNOME/json-glib/) - JSON parsing

### Related Projects

- [Tauri](https://tauri.app/) - Desktop application framework
- [Saucer](https://github.com/saucer/saucer) - C++ webview library
- [Webview](https://github.com/webview/webview) - Lightweight webview library

## Contributing

Contributions are welcome! Please:

1. Read the code and understand the architecture
2. Follow the existing code style
3. Add tests for new features
4. Update documentation
5. Submit pull requests

## License

CogBridge is licensed under the MIT License. See [LICENSE](LICENSE) for details.

---

**CogBridge** - Empowering embedded systems with modern web UIs. 🌉