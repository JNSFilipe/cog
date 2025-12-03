/*
 * dashboard.c
 * Copyright (C) 2024 CogBridge Examples
 *
 * SPDX-License-Identifier: MIT
 *
 * Advanced example demonstrating real-world embedded system use cases:
 * - Simulated sensor data reading
 * - Hardware control commands
 * - Real-time dashboard updates
 * - JSON-based data interchange
 * - Multiple sensor types and hardware controls
 * - LED control and motor speed management
 * - System monitoring (CPU, memory)
 *
 * This example shows how CogBridge can be used to build rich interactive
 * dashboards for embedded systems with real-time data updates and hardware
 * control. Unlike the minimal example, this demonstrates:
 * - Complex state management
 * - Multiple bound functions
 * - Periodic sensor updates
 * - Rich HTML/CSS dashboard UI
 */

#include "../cogbridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <json-glib/json-glib.h>

/* Simulated hardware state */
typedef struct {
    double temperature;
    double humidity;
    double pressure;
    int cpu_usage;
    int memory_usage;
    gboolean led_state[4];
    gboolean motor_running;
    int motor_speed;
} HardwareState;

static HardwareState hw_state = {
    .temperature = 25.0,
    .humidity = 50.0,
    .pressure = 1013.25,
    .cpu_usage = 30,
    .memory_usage = 45,
    .led_state = {FALSE, FALSE, FALSE, FALSE},
    .motor_running = FALSE,
    .motor_speed = 0,
};

/* Dashboard HTML */
static const char *dashboard_html = 
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "    <meta charset='UTF-8'>"
    "    <title>Embedded System Dashboard</title>"
    "    <style>"
    "        * { margin: 0; padding: 0; box-sizing: border-box; }"
    "        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #1a1a2e; color: #eee; }"
    "        .dashboard { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; padding: 20px; }"
    "        .card { background: #16213e; border-radius: 12px; padding: 20px; box-shadow: 0 4px 6px rgba(0,0,0,0.3); }"
    "        .card h2 { color: #0f4c75; margin-bottom: 15px; font-size: 1.2em; border-bottom: 2px solid #0f4c75; padding-bottom: 10px; }"
    "        .sensor-value { font-size: 2.5em; font-weight: bold; color: #3282b8; margin: 10px 0; }"
    "        .sensor-label { color: #bbb; font-size: 0.9em; }"
    "        .progress-bar { background: #0a1929; height: 24px; border-radius: 12px; overflow: hidden; margin: 10px 0; }"
    "        .progress-fill { height: 100%; background: linear-gradient(90deg, #0f4c75, #3282b8); transition: width 0.3s; display: flex; align-items: center; justify-content: center; color: white; font-weight: bold; font-size: 0.85em; }"
    "        button { background: #0f4c75; color: white; border: none; padding: 12px 24px; border-radius: 6px; cursor: pointer; margin: 5px; font-size: 1em; transition: background 0.3s; }"
    "        button:hover { background: #3282b8; }"
    "        button:active { transform: scale(0.98); }"
    "        button.danger { background: #d32f2f; }"
    "        button.danger:hover { background: #f44336; }"
    "        button.success { background: #388e3c; }"
    "        button.success:hover { background: #4caf50; }"
    "        .led { display: inline-block; width: 30px; height: 30px; border-radius: 50%; margin: 5px; border: 2px solid #333; transition: all 0.3s; }"
    "        .led.off { background: #333; box-shadow: inset 0 2px 4px rgba(0,0,0,0.5); }"
    "        .led.on { box-shadow: 0 0 20px currentColor, inset 0 0 10px currentColor; }"
    "        .led.red { color: #f44336; }"
    "        .led.green { color: #4caf50; }"
    "        .led.blue { color: #2196f3; }"
    "        .led.yellow { color: #ffeb3b; }"
    "        .led.on.red { background: #f44336; }"
    "        .led.on.green { background: #4caf50; }"
    "        .led.on.blue { background: #2196f3; }"
    "        .led.on.yellow { background: #ffeb3b; }"
    "        .control-group { margin: 15px 0; }"
    "        .slider { width: 100%; height: 8px; border-radius: 4px; background: #0a1929; outline: none; }"
    "        .slider::-webkit-slider-thumb { width: 20px; height: 20px; border-radius: 50%; background: #3282b8; cursor: pointer; }"
    "        .log { background: #0a1929; padding: 10px; border-radius: 6px; max-height: 200px; overflow-y: auto; font-family: 'Courier New', monospace; font-size: 0.85em; }"
    "        .log-entry { padding: 5px; border-left: 3px solid #0f4c75; margin: 5px 0; }"
    "        .log-entry.info { border-color: #2196f3; color: #64b5f6; }"
    "        .log-entry.warning { border-color: #ff9800; color: #ffb74d; }"
    "        .log-entry.error { border-color: #f44336; color: #e57373; }"
    "        .header { background: #0f4c75; padding: 20px; text-align: center; box-shadow: 0 2px 4px rgba(0,0,0,0.3); }"
    "        .header h1 { color: white; font-size: 2em; }"
    "        .status-badge { display: inline-block; padding: 5px 15px; border-radius: 12px; font-size: 0.9em; margin-left: 10px; }"
    "        .status-badge.online { background: #4caf50; }"
    "        .status-badge.offline { background: #f44336; }"
    "    </style>"
    "</head>"
    "<body>"
    "    <div class='header'>"
    "        <h1>🔧 Embedded System Dashboard <span id='status' class='status-badge online'>ONLINE</span></h1>"
    "    </div>"
    "    <div class='dashboard'>"
    "        <!-- Sensors Card -->"
    "        <div class='card'>"
    "            <h2>📊 Sensor Readings</h2>"
    "            <div class='sensor-label'>Temperature</div>"
    "            <div class='sensor-value' id='temp'>--°C</div>"
    "            <div class='sensor-label'>Humidity</div>"
    "            <div class='sensor-value' id='humidity'>--%</div>"
    "            <div class='sensor-label'>Pressure</div>"
    "            <div class='sensor-value' id='pressure'>-- hPa</div>"
    "        </div>"
    "        "
    "        <!-- System Resources Card -->"
    "        <div class='card'>"
    "            <h2>💻 System Resources</h2>"
    "            <div class='sensor-label'>CPU Usage</div>"
    "            <div class='progress-bar'>"
    "                <div class='progress-fill' id='cpu-bar' style='width: 0%'>0%</div>"
    "            </div>"
    "            <div class='sensor-label'>Memory Usage</div>"
    "            <div class='progress-bar'>"
    "                <div class='progress-fill' id='mem-bar' style='width: 0%'>0%</div>"
    "            </div>"
    "            <button onclick='refreshSystemInfo()'>🔄 Refresh</button>"
    "        </div>"
    "        "
    "        <!-- LED Control Card -->"
    "        <div class='card'>"
    "            <h2>💡 LED Control</h2>"
    "            <div style='text-align: center; padding: 20px 0;'>"
    "                <div class='led red off' id='led0'></div>"
    "                <div class='led green off' id='led1'></div>"
    "                <div class='led blue off' id='led2'></div>"
    "                <div class='led yellow off' id='led3'></div>"
    "            </div>"
    "            <div style='text-align: center;'>"
    "                <button onclick='toggleLED(0)'>Toggle LED 1</button>"
    "                <button onclick='toggleLED(1)'>Toggle LED 2</button>"
    "                <button onclick='toggleLED(2)'>Toggle LED 3</button>"
    "                <button onclick='toggleLED(3)'>Toggle LED 4</button>"
    "            </div>"
    "            <div style='text-align: center; margin-top: 10px;'>"
    "                <button class='success' onclick='allLEDsOn()'>All ON</button>"
    "                <button class='danger' onclick='allLEDsOff()'>All OFF</button>"
    "            </div>"
    "        </div>"
    "        "
    "        <!-- Motor Control Card -->"
    "        <div class='card'>"
    "            <h2>⚙️ Motor Control</h2>"
    "            <div class='control-group'>"
    "                <div class='sensor-label'>Speed: <span id='motor-speed'>0</span>%</div>"
    "                <input type='range' min='0' max='100' value='0' class='slider' id='speed-slider' oninput='updateMotorSpeed(this.value)'>"
    "            </div>"
    "            <div style='text-align: center;'>"
    "                <button class='success' id='motor-btn' onclick='toggleMotor()'>▶️ Start Motor</button>"
    "                <button class='danger' onclick='emergencyStop()'>🛑 Emergency Stop</button>"
    "            </div>"
    "        </div>"
    "        "
    "        <!-- System Log Card -->"
    "        <div class='card' style='grid-column: 1 / -1;'>"
    "            <h2>📝 System Log</h2>"
    "            <div class='log' id='log'></div>"
    "        </div>"
    "    </div>"
    "    "
    "    <script>"
    "        let updateInterval = null;"
    "        "
    "        function log(message, level = 'info') {"
    "            const logDiv = document.getElementById('log');"
    "            const entry = document.createElement('div');"
    "            entry.className = 'log-entry ' + level;"
    "            const time = new Date().toLocaleTimeString();"
    "            entry.textContent = `[${time}] ${message}`;"
    "            logDiv.insertBefore(entry, logDiv.firstChild);"
    "            if (logDiv.children.length > 50) logDiv.removeChild(logDiv.lastChild);"
    "        }"
    "        "
    "        async function refreshSensors() {"
    "            try {"
    "                const data = await window.cogbridge.read_sensors();"
    "                document.getElementById('temp').textContent = data.temperature.toFixed(1) + '°C';"
    "                document.getElementById('humidity').textContent = data.humidity.toFixed(1) + '%';"
    "                document.getElementById('pressure').textContent = data.pressure.toFixed(2) + ' hPa';"
    "            } catch (e) {"
    "                log('Failed to read sensors: ' + e, 'error');"
    "            }"
    "        }"
    "        "
    "        async function refreshSystemInfo() {"
    "            try {"
    "                const data = await window.cogbridge.get_system_stats();"
    "                const cpuBar = document.getElementById('cpu-bar');"
    "                const memBar = document.getElementById('mem-bar');"
    "                cpuBar.style.width = data.cpu_usage + '%';"
    "                cpuBar.textContent = data.cpu_usage + '%';"
    "                memBar.style.width = data.memory_usage + '%';"
    "                memBar.textContent = data.memory_usage + '%';"
    "                log('System stats refreshed');"
    "            } catch (e) {"
    "                log('Failed to read system stats: ' + e, 'error');"
    "            }"
    "        }"
    "        "
    "        async function toggleLED(index) {"
    "            try {"
    "                const result = await window.cogbridge.toggle_led(index);"
    "                const led = document.getElementById('led' + index);"
    "                if (result.state) {"
    "                    led.classList.add('on');"
    "                    led.classList.remove('off');"
    "                } else {"
    "                    led.classList.add('off');"
    "                    led.classList.remove('on');"
    "                }"
    "                log(`LED ${index + 1} turned ${result.state ? 'ON' : 'OFF'}`);"
    "            } catch (e) {"
    "                log('Failed to toggle LED: ' + e, 'error');"
    "            }"
    "        }"
    "        "
    "        async function allLEDsOn() {"
    "            for (let i = 0; i < 4; i++) await toggleLED(i);"
    "        }"
    "        "
    "        async function allLEDsOff() {"
    "            for (let i = 0; i < 4; i++) {"
    "                const led = document.getElementById('led' + i);"
    "                if (led.classList.contains('on')) await toggleLED(i);"
    "            }"
    "        }"
    "        "
    "        async function toggleMotor() {"
    "            try {"
    "                const result = await window.cogbridge.toggle_motor();"
    "                const btn = document.getElementById('motor-btn');"
    "                if (result.running) {"
    "                    btn.textContent = '⏸️ Stop Motor';"
    "                    btn.className = 'button danger';"
    "                    log('Motor started at ' + result.speed + '%', 'info');"
    "                } else {"
    "                    btn.textContent = '▶️ Start Motor';"
    "                    btn.className = 'button success';"
    "                    log('Motor stopped', 'info');"
    "                }"
    "            } catch (e) {"
    "                log('Failed to toggle motor: ' + e, 'error');"
    "            }"
    "        }"
    "        "
    "        async function updateMotorSpeed(speed) {"
    "            document.getElementById('motor-speed').textContent = speed;"
    "            try {"
    "                await window.cogbridge.set_motor_speed(parseInt(speed));"
    "            } catch (e) {"
    "                log('Failed to set motor speed: ' + e, 'error');"
    "            }"
    "        }"
    "        "
    "        async function emergencyStop() {"
    "            try {"
    "                await window.cogbridge.emergency_stop();"
    "                document.getElementById('motor-btn').textContent = '▶️ Start Motor';"
    "                document.getElementById('motor-btn').className = 'button success';"
    "                document.getElementById('speed-slider').value = 0;"
    "                document.getElementById('motor-speed').textContent = '0';"
    "                log('EMERGENCY STOP ACTIVATED', 'warning');"
    "            } catch (e) {"
    "                log('Emergency stop failed: ' + e, 'error');"
    "            }"
    "        }"
    "        "
    "        window.cogbridge.on('sensor_update', function(data) {"
    "            document.getElementById('temp').textContent = data.temperature.toFixed(1) + '°C';"
    "            document.getElementById('humidity').textContent = data.humidity.toFixed(1) + '%';"
    "            document.getElementById('pressure').textContent = data.pressure.toFixed(2) + ' hPa';"
    "        });"
    "        "
    "        window.cogbridge.on('system_alert', function(data) {"
    "            log(data.message, data.level || 'warning');"
    "        });"
    "        "
    "        window.addEventListener('load', function() {"
    "            log('Dashboard initialized', 'info');"
    "            refreshSensors();"
    "            refreshSystemInfo();"
    "            updateInterval = setInterval(() => {"
    "                refreshSensors();"
    "                refreshSystemInfo();"
    "            }, 2000);"
    "        });"
    "    </script>"
    "</body>"
    "</html>";

/* Read sensors callback */
static char *
on_read_sensors(CogBridge *bridge, const char *function_name, const char *args_json, void *user_data)
{
    HardwareState *hw = (HardwareState *)user_data;
    
    // Simulate sensor variation
    hw->temperature += (g_random_double() - 0.5) * 0.5;
    hw->humidity += (g_random_double() - 0.5) * 2.0;
    hw->pressure += (g_random_double() - 0.5) * 0.1;
    
    // Keep values in reasonable ranges
    hw->temperature = CLAMP(hw->temperature, 20.0, 30.0);
    hw->humidity = CLAMP(hw->humidity, 40.0, 60.0);
    hw->pressure = CLAMP(hw->pressure, 1010.0, 1020.0);
    
    printf("[C] Reading sensors: T=%.1f°C H=%.1f%% P=%.2fhPa\n", 
           hw->temperature, hw->humidity, hw->pressure);
    
    return g_strdup_printf("{\"temperature\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f}",
                          hw->temperature, hw->humidity, hw->pressure);
}

/* Get system stats callback */
static char *
on_get_system_stats(CogBridge *bridge, const char *function_name, const char *args_json, void *user_data)
{
    HardwareState *hw = (HardwareState *)user_data;
    
    // Simulate CPU and memory usage variation
    hw->cpu_usage += (g_random_int_range(-10, 10));
    hw->memory_usage += (g_random_int_range(-5, 5));
    
    hw->cpu_usage = CLAMP(hw->cpu_usage, 20, 90);
    hw->memory_usage = CLAMP(hw->memory_usage, 30, 80);
    
    printf("[C] System stats: CPU=%d%% MEM=%d%%\n", hw->cpu_usage, hw->memory_usage);
    
    return g_strdup_printf("{\"cpu_usage\":%d,\"memory_usage\":%d}",
                          hw->cpu_usage, hw->memory_usage);
}

/* Toggle LED callback */
static char *
on_toggle_led(CogBridge *bridge, const char *function_name, const char *args_json, void *user_data)
{
    HardwareState *hw = (HardwareState *)user_data;
    JsonParser *parser = json_parser_new();
    GError *error = NULL;
    
    if (!json_parser_load_from_data(parser, args_json, -1, &error)) {
        g_object_unref(parser);
        return g_strdup("{\"error\":\"Invalid arguments\"}");
    }
    
    JsonNode *root = json_parser_get_root(parser);
    JsonArray *array = json_node_get_array(root);
    int index = (int)json_array_get_int_element(array, 0);
    g_object_unref(parser);
    
    if (index < 0 || index >= 4) {
        return g_strdup("{\"error\":\"Invalid LED index\"}");
    }
    
    hw->led_state[index] = !hw->led_state[index];
    printf("[C] LED %d toggled to %s\n", index, hw->led_state[index] ? "ON" : "OFF");
    
    return g_strdup_printf("{\"index\":%d,\"state\":%s}", 
                          index, hw->led_state[index] ? "true" : "false");
}

/* Toggle motor callback */
static char *
on_toggle_motor(CogBridge *bridge, const char *function_name, const char *args_json, void *user_data)
{
    HardwareState *hw = (HardwareState *)user_data;
    
    hw->motor_running = !hw->motor_running;
    
    printf("[C] Motor toggled to %s (speed: %d%%)\n", 
           hw->motor_running ? "RUNNING" : "STOPPED", hw->motor_speed);
    
    return g_strdup_printf("{\"running\":%s,\"speed\":%d}",
                          hw->motor_running ? "true" : "false", hw->motor_speed);
}

/* Set motor speed callback */
static char *
on_set_motor_speed(CogBridge *bridge, const char *function_name, const char *args_json, void *user_data)
{
    HardwareState *hw = (HardwareState *)user_data;
    JsonParser *parser = json_parser_new();
    GError *error = NULL;
    
    if (!json_parser_load_from_data(parser, args_json, -1, &error)) {
        g_object_unref(parser);
        return g_strdup("{\"error\":\"Invalid arguments\"}");
    }
    
    JsonNode *root = json_parser_get_root(parser);
    JsonArray *array = json_node_get_array(root);
    int speed = (int)json_array_get_int_element(array, 0);
    g_object_unref(parser);
    
    hw->motor_speed = CLAMP(speed, 0, 100);
    printf("[C] Motor speed set to %d%%\n", hw->motor_speed);
    
    return g_strdup_printf("{\"speed\":%d}", hw->motor_speed);
}

/* Emergency stop callback */
static char *
on_emergency_stop(CogBridge *bridge, const char *function_name, const char *args_json, void *user_data)
{
    HardwareState *hw = (HardwareState *)user_data;
    
    hw->motor_running = FALSE;
    hw->motor_speed = 0;
    
    printf("[C] EMERGENCY STOP ACTIVATED!\n");
    
    // Send alert to UI
    cogbridge_emit_event(bridge, "system_alert", 
                        "{\"message\":\"Emergency stop activated!\",\"level\":\"warning\"}");
    
    return g_strdup("{\"success\":true}");
}

/* Periodic sensor update */
static gboolean
on_sensor_timer(gpointer user_data)
{
    CogBridge *bridge = (CogBridge *)user_data;
    HardwareState *hw = &hw_state;
    
    // Simulate sensor readings
    hw->temperature += (g_random_double() - 0.5) * 0.3;
    hw->humidity += (g_random_double() - 0.5) * 1.5;
    hw->pressure += (g_random_double() - 0.5) * 0.05;
    
    hw->temperature = CLAMP(hw->temperature, 20.0, 30.0);
    hw->humidity = CLAMP(hw->humidity, 40.0, 60.0);
    hw->pressure = CLAMP(hw->pressure, 1010.0, 1020.0);
    
    char *event_data = g_strdup_printf(
        "{\"temperature\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f}",
        hw->temperature, hw->humidity, hw->pressure);
    
    cogbridge_emit_event(bridge, "sensor_update", event_data);
    g_free(event_data);
    
    return G_SOURCE_CONTINUE;
}

int
main(int argc, char *argv[])
{
    GError *error = NULL;
    
    printf("=== CogBridge Embedded Dashboard Example ===\n\n");
    
    // Initialize random number generator
    g_random_set_seed(time(NULL));
    
    // Initialize CogBridge with compile-time platform selection
    CogBridgeConfig config;
    cogbridge_get_default_config(&config);
    config.enable_console = true;
    config.enable_developer_extras = true;
    
    /* Platform is set at compile time via build system */
#ifdef COGBRIDGE_PLATFORM_DEFAULT
    config.platform = COGBRIDGE_PLATFORM_DEFAULT;
#endif
#ifdef COGBRIDGE_MODULE_DIR
    config.module_dir = COGBRIDGE_MODULE_DIR;
#endif
    
    if (!cogbridge_init(&config, &error)) {
        fprintf(stderr, "Failed to initialize CogBridge: %s\n", error->message);
        g_error_free(error);
        return 1;
    }
    
    // Create bridge instance
    CogBridge *bridge = cogbridge_new("embedded-dashboard");
    if (!bridge) {
        fprintf(stderr, "Failed to create CogBridge instance\n");
        cogbridge_cleanup();
        return 1;
    }
    
    // Bind hardware control functions
    printf("Binding hardware control functions...\n");
    cogbridge_bind_function(bridge, "read_sensors", on_read_sensors, &hw_state, NULL);
    cogbridge_bind_function(bridge, "get_system_stats", on_get_system_stats, &hw_state, NULL);
    cogbridge_bind_function(bridge, "toggle_led", on_toggle_led, &hw_state, NULL);
    cogbridge_bind_function(bridge, "toggle_motor", on_toggle_motor, &hw_state, NULL);
    cogbridge_bind_function(bridge, "set_motor_speed", on_set_motor_speed, &hw_state, NULL);
    cogbridge_bind_function(bridge, "emergency_stop", on_emergency_stop, &hw_state, NULL);
    
    // Load dashboard HTML
    printf("Loading dashboard...\n");
    cogbridge_load_html(bridge, dashboard_html, "file:///");
    
    // Wait for page to load
    printf("Waiting for dashboard to load...\n");
    if (!cogbridge_wait_ready(bridge, 10000)) {
        fprintf(stderr, "Timeout waiting for dashboard to load\n");
        cogbridge_free(bridge);
        cogbridge_cleanup();
        return 1;
    }
    
    printf("Dashboard loaded successfully!\n\n");
    
    // Set up periodic sensor updates
    printf("Starting sensor update timer...\n");
    g_timeout_add_seconds(3, on_sensor_timer, bridge);
    
    printf("\n=== Dashboard is running ===\n");
    printf("The dashboard simulates an embedded system with:\n");
    printf("  - Sensor readings (temperature, humidity, pressure)\n");
    printf("  - System resource monitoring (CPU, memory)\n");
    printf("  - LED controls (4 LEDs)\n");
    printf("  - Motor control with speed adjustment\n");
    printf("  - Real-time event updates\n\n");
    printf("Press Ctrl+C to exit.\n\n");
    
    // Run event loop
    cogbridge_run(bridge);
    
    printf("\n=== Shutting down ===\n");
    
    // Cleanup
    cogbridge_free(bridge);
    cogbridge_cleanup();
    
    printf("Done!\n");
    return 0;
}