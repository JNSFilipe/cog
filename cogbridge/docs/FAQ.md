# CogBridge - Frequently Asked Questions (FAQ)

## General Questions

### Q: Why don't I see a browser window when running the examples?

**A: This is the correct and intended behavior!** CogBridge is a **headless** framework designed for embedded systems without displays.

When you run:
```bash
./cogbridge/examples/run-examples.sh minimal
```

The WebKit engine runs **in the background**, executing your JavaScript code and communicating with your C code - but there's no visual window. This is perfect for:

- Embedded devices without screens (IoT sensors, industrial controllers)
- Server-side applications
- Automated testing
- Background web processing

### Q: But is the JavaScript actually running?

**A: Yes, absolutely!** You can verify this by checking the console output:

```
[C] add(42, 58) = 100
[C] greet('World')
[C] get_system_info()
```

These `[C]` messages prove that your C functions are being called from JavaScript! The JavaScript is executing, it's just not visible.

### Q: How can I verify it's working if I can't see anything?

**A: Several ways:**

1. **Check the logs** - Look for `[C]` messages showing C function calls
2. **Add printf statements** - Your C callbacks will print when called
3. **Use console.log** - JavaScript console messages appear in the terminal
4. **Check return values** - Your C functions return values to JavaScript

Example output from the minimal example:
```
** Message: CogBridge initialized successfully
** Message: CogBridge instance created: minimal-example
** Message: CogBridge: Bound function: add
** Message: CogBridge: Page loaded and ready
[C] add(42, 58) = 100          ← JavaScript called your C function!
[C] greet('World')              ← And again!
```

### Q: Can I see a GUI window for development/debugging?

**A: Yes, but it requires additional setup.**

The GUI mode needs:
1. A display server (X11 or Wayland)
2. Additional platform libraries (libportal-gtk4, xkbcommon-x11, etc.)
3. Rebuilding with GUI platform support

For embedded systems and production use, headless mode is recommended. For development, you can:
- Use the console logs to debug
- Add extensive printf/g_message statements
- Test your logic with the headless examples
- If you really need visual output, build with full GUI dependencies

### Q: What's the point of a web UI if I can't see it?

**A: Great question!** Here's why it's powerful:

1. **Separation of Concerns**: Your business logic (JavaScript) is separate from your hardware code (C)
2. **Web Technologies**: Use HTML/CSS/JavaScript for complex logic, algorithms, and data processing
3. **Easy Updates**: Update your application logic (HTML/JavaScript) without recompiling C code
4. **Familiar Tools**: Use standard web development tools and libraries
5. **Portable Logic**: The same JavaScript can run on different hardware

**Example Use Case:**
```
Embedded Temperature Controller
├── C Code (Hardware Layer)
│   ├── Read temperature sensor (I2C)
│   ├── Control heating element (GPIO)
│   └── Expose functions to JavaScript
│
└── JavaScript (Business Logic)
    ├── PID control algorithm
    ├── Temperature setpoint management
    ├── Alarm threshold logic
    └── Data logging to cloud
```

JavaScript handles the complex control logic, while C handles the hardware. No display needed!

---

## Technical Questions

### Q: What platform does CogBridge use by default?

**A:** The `headless` platform, which uses WebKit WPE without any display output.

### Q: Can I change the platform?

**A:** Yes! You can set the platform in your configuration:

```c
CogBridgeConfig config;
cogbridge_get_default_config(&config);
config.platform_name = "gtk4";  // For GUI window
// or
config.platform_name = "headless";  // For no display (default)

cogbridge_init(&config, NULL);
```

However, you must build with that platform enabled and have the required dependencies.

### Q: Why does it say "cannot find module 'headless'"?

**A:** You need to set the `COG_MODULEDIR` environment variable to tell CogBridge where to find platform modules:

```bash
export COG_MODULEDIR=./build/platform/headless
./build/cogbridge/examples/cogbridge-minimal
```

Or use the helper script which does this automatically:
```bash
./cogbridge/examples/run-examples.sh minimal
```

### Q: How much memory does it use?

**A:** In headless mode:
- Baseline: ~50 MB (WebKit engine)
- Your application: depends on HTML/JavaScript complexity
- Total typical: 50-100 MB

This is much lighter than Electron (~150+ MB) or running a full browser.

### Q: Can I use real web APIs?

**A:** Yes! You have access to:
- `fetch()` for HTTP requests
- `WebSocket` for real-time communication
- `localStorage` for data persistence
- DOM manipulation
- Modern JavaScript (ES6+)
- Many standard web APIs

The JavaScript runs in a real WebKit engine, just without visual output.

---

## Usage Questions

### Q: How do I call a C function from JavaScript?

**A:** 

1. **In C, bind your function:**
```c
static char *my_function(CogBridge *bridge, const char *fn,
                         const char *args_json, void *data) {
    printf("Called from JavaScript!\n");
    return g_strdup("\"result from C\"");
}

cogbridge_bind_function(bridge, "myFunction", my_function, NULL, NULL);
```

2. **In JavaScript, call it:**
```javascript
const result = await window.cogbridge.myFunction(arg1, arg2);
console.log('Result:', result);
```

### Q: How do I call JavaScript from C?

**A:**

```c
cogbridge_execute_script(bridge, 
    "document.title = 'New Title'; console.log('Hello from C');",
    callback,
    user_data);
```

### Q: How do I send data from C to JavaScript?

**A:** Use events:

```c
// In C - emit event
const char *data = "{\"temperature\":25.5,\"unit\":\"celsius\"}";
cogbridge_emit_event(bridge, "sensor_update", data);
```

```javascript
// In JavaScript - listen for event
window.cogbridge.on('sensor_update', function(data) {
    console.log('Temperature:', data.temperature);
    updateDisplay(data);
});
```

### Q: What format should I use for data exchange?

**A:** JSON is used for all data exchange:

- **JavaScript → C**: Arguments are JSON array `["arg1", 42, true]`
- **C → JavaScript**: Return JSON string `{"result": "value"}`
- **Events**: JSON data `{"key": "value"}`

Use `json-glib` in C to parse and generate JSON easily.

---

## Build Questions

### Q: What dependencies do I need?

**A:** Minimum requirements:

```bash
# Ubuntu/Debian
sudo apt-get install -y \
    build-essential meson ninja-build \
    libwpewebkit-1.0-dev libwpe-1.0-dev \
    libglib2.0-dev libjson-glib-dev
```

### Q: How do I build CogBridge?

**A:**

```bash
cd cog

meson setup build \
    -Dexamples=true \
    -Dlibportal=disabled \
    -Dprograms=false \
    -Dplatforms=headless

ninja -C build
```

### Q: Can I use CogBridge in my own project?

**A:** Yes! After building:

```c
#include <cogbridge/cogbridge.h>

int main(void) {
    cogbridge_init(NULL, NULL);
    CogBridge *bridge = cogbridge_new("my-app");
    
    // Your code here
    
    cogbridge_run(bridge);
    cogbridge_free(bridge);
    cogbridge_cleanup();
    return 0;
}
```

Compile with:
```bash
gcc -o my_app my_app.c $(pkg-config --cflags --libs cogbridge)
```

---

## Troubleshooting

### Q: The example runs but nothing happens

**A:** Check that:
1. You see "Page loaded and ready" in the logs
2. Your C callback functions are being called (add printf statements)
3. JavaScript console messages appear in terminal output
4. Check for error messages in the output

### Q: "Platform not usable" error

**A:** Make sure:
1. `COG_MODULEDIR` is set correctly
2. The platform module exists in that directory
3. You built with that platform enabled

```bash
# Check what you have
ls -l build/platform/

# Set the correct path
export COG_MODULEDIR=./build/platform/headless
```

### Q: Segmentation fault on startup

**A:** Common causes:
1. Platform module not found - set `COG_MODULEDIR`
2. Missing dependencies - check all are installed
3. Incompatible WebKit version - check version requirements

### Q: JavaScript functions not working

**A:** Verify:
1. Functions are bound BEFORE loading HTML
2. Function names match exactly
3. Check for JavaScript errors in console output
4. Ensure promises are being awaited

---

## Comparison Questions

### Q: How is this different from Electron?

**A:**

| Feature | CogBridge | Electron |
|---------|-----------|----------|
| Target | Embedded systems | Desktop apps |
| Memory | ~50 MB | ~150+ MB |
| Engine | WebKit | Chromium |
| Headless | Yes, by default | Difficult |
| Language | C | JavaScript/Node.js |
| Size | Small | Large |
| Use case | IoT, embedded | Desktop GUI |

### Q: How is this different from Tauri?

**A:**

| Feature | CogBridge | Tauri |
|---------|-----------|-------|
| Target | Embedded Linux | Desktop (multi-platform) |
| Language | C | Rust |
| Headless | Yes, by default | No |
| Display | Optional | Required |
| Focus | Hardware integration | Desktop applications |

### Q: How is this different from running a web server?

**A:**

With a web server, you need:
- Separate web server process
- HTTP communication overhead
- Browser on client side
- Network connectivity
- CORS, security tokens, etc.

With CogBridge:
- Single process
- Direct C ↔ JavaScript communication
- No browser needed
- No network required
- Simpler, faster, more secure

---

## Best Practices

### Q: Should I use headless or GUI mode?

**A:**

**Use Headless:**
- Production embedded systems
- IoT devices
- Server applications
- Automated processes
- Anywhere without a display

**Use GUI:**
- Development and debugging
- Desktop applications
- Kiosks with displays
- Digital signage
- When users need to see the UI

### Q: How should I structure my application?

**A:**

**Recommended structure:**
```
my_app/
├── src/
│   ├── main.c              # CogBridge setup, hardware interfacing
│   ├── hardware.c          # Hardware-specific code
│   └── callbacks.c         # C callbacks for JavaScript
├── web/
│   ├── index.html          # UI and logic
│   ├── app.js              # Business logic
│   └── style.css           # Styling
└── meson.build
```

**Layer separation:**
- C handles hardware, drivers, system calls
- JavaScript handles algorithms, business logic, data processing
- HTML provides structure (even if not displayed)

---

## Real-World Examples

### Q: What can I build with CogBridge?

**A:** Anything that needs C-level hardware access with complex logic:

1. **Smart Home Controller**
   - C reads sensors (temperature, motion, etc.)
   - JavaScript implements automation rules
   - No display needed

2. **Industrial PLC**
   - C controls relays, reads inputs
   - JavaScript handles ladder logic/control algorithms
   - Runs headlessly

3. **Data Logger**
   - C interfaces with measurement equipment
   - JavaScript processes and uploads data
   - Background operation

4. **Robot Controller**
   - C drives motors, reads sensors
   - JavaScript implements behavior algorithms
   - Headless embedded operation

5. **Test Equipment**
   - C controls instruments via GPIB/USB
   - JavaScript orchestrates test sequences
   - Can show GUI for operator or run headless

---

## Getting Help

### Q: Where can I find more documentation?

**A:**
- `cogbridge/README.md` - Full API documentation
- `cogbridge/GETTING_STARTED.md` - Tutorial
- `cogbridge/QUICK_REFERENCE.md` - API quick reference
- `cogbridge/OVERVIEW.md` - Architecture details
- `cogbridge/HEADLESS_VS_GUI.md` - Explains modes
- `cogbridge/examples/` - Working code examples

### Q: The examples are running correctly! Now what?

**A:** Congratulations! You've verified CogBridge is working. Next steps:

1. **Modify an example** - Change the HTML, add functions
2. **Create your own app** - Use examples as templates
3. **Add hardware integration** - Connect real sensors/devices
4. **Deploy to your target** - Cross-compile for embedded system

The framework is ready to use! 🚀

---

## Summary

**Key Points:**
- ✅ Headless = No window = **Correct behavior** for embedded systems
- ✅ JavaScript IS running even without visual output
- ✅ C ↔ JavaScript communication works perfectly headlessly
- ✅ This is designed for embedded systems, IoT, and servers
- ✅ GUI mode exists for development if you need it
- ✅ Verify it's working by checking console logs and `[C]` messages

**Quick Start:**
```bash
# Build
meson setup build -Dexamples=true -Dlibportal=disabled -Dprograms=false -Dplatforms=headless
ninja -C build

# Run (no window will appear - this is correct!)
./cogbridge/examples/run-examples.sh minimal

# Check output for [C] messages - if you see them, it's working!
```

Happy coding! 🎉