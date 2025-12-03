# CogBridge Testing Guide

## Testing Checklist

This document provides a comprehensive testing checklist for CogBridge to ensure all functionality works correctly on your target platform.

## Prerequisites

Before testing, ensure you have:
- [ ] Built CogBridge successfully
- [ ] All dependencies installed
- [ ] Examples compiled
- [ ] Test environment prepared (embedded system or development machine)

## Build Verification

### 1. Build System

```bash
# Clean build
rm -rf build
meson setup build -Dexamples=true
ninja -C build
```

Expected:
- [ ] No compilation errors
- [ ] No warnings in cogbridge.c
- [ ] libcogbridge.so created
- [ ] Examples built successfully

### 2. Installation (Optional)

```bash
sudo ninja -C build install
pkg-config --modversion cogbridge
pkg-config --cflags cogbridge
pkg-config --libs cogbridge
```

Expected:
- [ ] Installation succeeds
- [ ] pkg-config finds cogbridge
- [ ] Headers installed in correct location
- [ ] Library in system path

## Functional Testing

### Test 1: Minimal Example

```bash
./build/cogbridge/examples/cogbridge-minimal
```

**Expected Output:**
```
=== CogBridge Minimal Example ===

Binding C functions...
Loading HTML content...
Waiting for page to load...
Page loaded successfully!

[C] add(42, 58) = 100
[C] greet('World')
[C] get_system_info()
[C] trigger_event() - emitting custom_event

=== Running event loop ===
...
```

**Checklist:**
- [ ] Application starts without errors
- [ ] Console messages appear
- [ ] C functions are called from JavaScript
- [ ] Function results are correct (add returns 100)
- [ ] Events are emitted
- [ ] Application runs for ~10 seconds
- [ ] Application exits cleanly

### Test 2: Embedded Dashboard

```bash
./build/cogbridge/examples/cogbridge-dashboard
```

**Expected Output:**
```
=== CogBridge Embedded Dashboard Example ===

Binding hardware control functions...
Loading dashboard...
Waiting for dashboard to load...
Dashboard loaded successfully!

Starting sensor update timer...

=== Dashboard is running ===
...
```

**Checklist:**
- [ ] Application starts without errors
- [ ] All functions bound successfully
- [ ] Dashboard HTML loads
- [ ] Sensor readings simulated
- [ ] Periodic updates occur (every 3 seconds)
- [ ] Application continues running
- [ ] No memory leaks visible

## API Testing

### Test 3: Initialization

```c
// Test default config
CogBridgeConfig config;
cogbridge_get_default_config(&config);
assert(config.width == 1920);
assert(config.height == 1080);
assert(config.enable_console == true);

// Test initialization
GError *error = NULL;
bool result = cogbridge_init(&config, &error);
assert(result == true);
assert(error == NULL);

cogbridge_cleanup();
```

**Checklist:**
- [ ] Default config has expected values
- [ ] Initialization succeeds
- [ ] No errors reported
- [ ] Cleanup works without crash

### Test 4: Bridge Creation

```c
cogbridge_init(NULL, NULL);
CogBridge *bridge = cogbridge_new("test");
assert(bridge != NULL);
cogbridge_free(bridge);
cogbridge_cleanup();
```

**Checklist:**
- [ ] Bridge creates successfully
- [ ] Bridge frees without crash
- [ ] Multiple bridges can be created
- [ ] Bridge names are stored correctly

### Test 5: Function Binding

```c
static char *test_func(CogBridge *b, const char *n, 
                       const char *a, void *d) {
    return g_strdup("\"test\"");
}

cogbridge_init(NULL, NULL);
CogBridge *bridge = cogbridge_new("test");

bool result = cogbridge_bind_function(bridge, "test", test_func, NULL, NULL);
assert(result == true);

cogbridge_unbind_function(bridge, "test");

cogbridge_free(bridge);
cogbridge_cleanup();
```

**Checklist:**
- [ ] Binding succeeds
- [ ] Multiple functions can be bound
- [ ] Unbinding works
- [ ] Callback is called when function invoked from JS

### Test 6: Content Loading

```c
cogbridge_init(NULL, NULL);
CogBridge *bridge = cogbridge_new("test");

// Test HTML loading
cogbridge_load_html(bridge, "<html><body>Test</body></html>", NULL);
bool ready = cogbridge_wait_ready(bridge, 5000);
assert(ready == true);

// Test URI loading
cogbridge_load_uri(bridge, "data:text/html,<html><body>Test</body></html>");
ready = cogbridge_wait_ready(bridge, 5000);
assert(ready == true);

cogbridge_free(bridge);
cogbridge_cleanup();
```

**Checklist:**
- [ ] HTML string loads
- [ ] URI loads
- [ ] Ready state changes correctly
- [ ] Timeout works when page doesn't load

### Test 7: JavaScript Execution

```c
cogbridge_init(NULL, NULL);
CogBridge *bridge = cogbridge_new("test");
cogbridge_load_html(bridge, "<html></html>", NULL);
cogbridge_wait_ready(bridge, 5000);

// Async execution
int callback_called = 0;
void callback(CogBridge *b, const char *n, const char *r, void *d) {
    callback_called = 1;
}

cogbridge_execute_script(bridge, "2 + 2", callback, NULL);

// Wait for callback
// ...

assert(callback_called == 1);

cogbridge_free(bridge);
cogbridge_cleanup();
```

**Checklist:**
- [ ] Script executes
- [ ] Callback is invoked
- [ ] Result is correct
- [ ] Multiple scripts can execute

### Test 8: Event Emission

```c
cogbridge_init(NULL, NULL);
CogBridge *bridge = cogbridge_new("test");

const char *html = 
    "<html><body><script>"
    "let received = false;"
    "window.cogbridge.on('test_event', (data) => {"
    "  received = true;"
    "  console.log('Event received:', data);"
    "});"
    "</script></body></html>";

cogbridge_load_html(bridge, html, NULL);
cogbridge_wait_ready(bridge, 5000);

cogbridge_emit_event(bridge, "test_event", "{\"value\":42}");

// Verify event was received
// ...

cogbridge_free(bridge);
cogbridge_cleanup();
```

**Checklist:**
- [ ] Events are emitted
- [ ] JavaScript listeners are called
- [ ] Event data is passed correctly
- [ ] Multiple listeners work

## Memory Testing

### Test 9: Memory Leaks

Run with valgrind:

```bash
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         ./build/cogbridge/examples/cogbridge-minimal
```

**Checklist:**
- [ ] No definite leaks
- [ ] No invalid memory access
- [ ] All allocated memory freed

### Test 10: Long Running

Run dashboard for extended period:

```bash
timeout 3600 ./build/cogbridge/examples/cogbridge-dashboard
```

Monitor with:
```bash
top -p $(pidof cogbridge-dashboard)
```

**Checklist:**
- [ ] Memory usage stable
- [ ] CPU usage reasonable
- [ ] No crashes after 1 hour
- [ ] Responsive throughout

## Platform-Specific Testing

### ARM/Embedded Systems

**Checklist:**
- [ ] Cross-compilation works
- [ ] Runs on target hardware
- [ ] Performance acceptable
- [ ] Memory footprint acceptable
- [ ] Headless operation works
- [ ] No display server needed

### x86/x86_64

**Checklist:**
- [ ] Native compilation works
- [ ] All tests pass
- [ ] Examples run correctly

## Integration Testing

### Test 11: Hardware Integration (if available)

```c
// GPIO test
static char *gpio_test(CogBridge *b, const char *n,
                       const char *a, void *d) {
    // Actually toggle GPIO
    gpio_set_value(17, 1);
    usleep(100000);
    gpio_set_value(17, 0);
    return g_strdup("{\"success\":true}");
}
```

**Checklist:**
- [ ] GPIO control works
- [ ] Sensor reading works
- [ ] I2C/SPI communication works
- [ ] Hardware responds correctly

### Test 12: Network Integration

```c
cogbridge_load_uri(bridge, "http://localhost:8000/test.html");
```

**Checklist:**
- [ ] HTTP URLs load
- [ ] HTTPS URLs load (if supported)
- [ ] Network timeouts handled
- [ ] Offline handling works

## Error Handling

### Test 13: Invalid Input

```c
// NULL parameters
cogbridge_init(NULL, NULL); // Should use defaults
CogBridge *bridge = cogbridge_new(NULL); // Should use default name
cogbridge_bind_function(bridge, NULL, func, NULL, NULL); // Should fail gracefully

// Invalid JSON
char *result = callback(bridge, "func", "{invalid json}", NULL);
// Should handle gracefully
```

**Checklist:**
- [ ] NULL parameters handled
- [ ] Invalid JSON handled
- [ ] Error messages clear
- [ ] No crashes on bad input

### Test 14: Timeout Handling

```c
// Page that never loads
cogbridge_load_uri(bridge, "http://non-existent-domain-12345.com");
bool ready = cogbridge_wait_ready(bridge, 5000);
assert(ready == false); // Should timeout
```

**Checklist:**
- [ ] Timeouts work correctly
- [ ] No hang on invalid URLs
- [ ] Timeout returns false

## Performance Testing

### Test 15: Function Call Performance

```c
// Time 1000 function calls
clock_t start = clock();
for (int i = 0; i < 1000; i++) {
    // Call bound function from JS
}
clock_t end = clock();
double time = ((double)(end - start)) / CLOCKS_PER_SEC;
printf("1000 calls in %.3f seconds\n", time);
```

**Target:** < 1 second for 1000 calls

**Checklist:**
- [ ] Function calls are fast
- [ ] No noticeable lag
- [ ] Throughput acceptable

### Test 16: Event Emission Performance

```c
// Time 1000 event emissions
clock_t start = clock();
for (int i = 0; i < 1000; i++) {
    cogbridge_emit_event(bridge, "test", "{\"i\":0}");
}
clock_t end = clock();
```

**Target:** < 0.5 seconds for 1000 events

**Checklist:**
- [ ] Events are fast
- [ ] No backlog buildup
- [ ] Memory stable

## Documentation Verification

### Test 17: Documentation Examples

Try all examples from README.md:

**Checklist:**
- [ ] All code examples compile
- [ ] All code examples run
- [ ] Examples produce expected output
- [ ] No typos or errors in docs

### Test 18: API Coverage

Verify all API functions documented:

**Checklist:**
- [ ] cogbridge_init() documented
- [ ] cogbridge_new() documented
- [ ] cogbridge_bind_function() documented
- [ ] cogbridge_execute_script() documented
- [ ] cogbridge_emit_event() documented
- [ ] All functions have examples

## Regression Testing

After making changes:

**Checklist:**
- [ ] All previous tests still pass
- [ ] No new warnings
- [ ] Examples still work
- [ ] API unchanged (or properly versioned)
- [ ] Documentation updated

## Test Report Template

```
CogBridge Test Report
Date: ___________
Platform: ___________
Version: ___________

Build Tests:        [ ] PASS  [ ] FAIL
Functional Tests:   [ ] PASS  [ ] FAIL
API Tests:          [ ] PASS  [ ] FAIL
Memory Tests:       [ ] PASS  [ ] FAIL
Performance Tests:  [ ] PASS  [ ] FAIL

Issues Found:
1. ___________
2. ___________

Notes:
___________
```

## Automated Testing (Future)

TODO: Implement automated test suite

```bash
# Future automated tests
./run_tests.sh
```

Should include:
- Unit tests for each API function
- Integration tests
- Performance benchmarks
- Memory leak detection
- Continuous integration

## Debugging Failed Tests

### Enable Verbose Output

```c
config.enable_console = true;
config.enable_developer_extras = true;
```

### Check Logs

```bash
# Run with debug output
G_MESSAGES_DEBUG=all ./cogbridge-minimal
```

### Use GDB

```bash
gdb ./build/cogbridge/examples/cogbridge-minimal
(gdb) run
(gdb) backtrace
```

### Check Dependencies

```bash
ldd ./build/cogbridge/examples/cogbridge-minimal
pkg-config --list-all | grep -E '(wpe|webkit|json)'
```

## Platform-Specific Issues

### Raspberry Pi
- Ensure WPE backend installed
- May need more RAM (use 1GB+ model)
- Adjust timeouts for slower hardware

### BeagleBone
- Limited RAM - reduce config.width/height
- May need swap space
- Test with simpler HTML

### Industrial PC
- Should run without issues
- Full performance expected

## Reporting Issues

When reporting issues, include:
- Platform and architecture
- CogBridge version
- WebKit WPE version
- Error messages
- Test that failed
- Steps to reproduce

## Success Criteria

CogBridge is considered ready for production when:
- [ ] All functional tests pass
- [ ] No memory leaks detected
- [ ] Performance meets targets
- [ ] Examples run on target hardware
- [ ] Documentation complete and accurate
- [ ] At least one real-world application deployed

---

**Last Updated:** 2025-01-01
**Version:** 1.0.0