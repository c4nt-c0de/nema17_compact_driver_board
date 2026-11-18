#pragma once
#include <Arduino.h>

#if defined(ESP8266)
#include <ESP8266WebServer.h>
#elif defined(ESP32)
#include <WebServer.h>
#endif

#include <functional>

inline void register_web_routes(
    void* server_ptr,
    std::function<void()> toggle_func,
    std::function<float()> get_temperature,
    std::function<bool()> get_ir_triggered,
    std::function<int()> get_stall_guard_value,
    const volatile float* shaft_angle_ptr,   // range 0–360
    const volatile float* total_angle_ptr    // total angle
)
{
#if defined(ESP8266)
    ESP8266WebServer* server = static_cast<ESP8266WebServer*>(server_ptr);
#elif defined(ESP32)
    WebServer* server = static_cast<WebServer*>(server_ptr);
#endif

static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<body>
<h1>ESP Debug Page</h1>

<button onclick="fetch('/led1', {method:'POST'})">LED</button>

<h2>Status</h2>
<div>Internal Temp: <span id="tmc_temp">0.0</span> C</div>
<div>IR Sensor: <span id="ir_event">No</span></div>
<div>Total Angle: <span id="total_angle">0.0</span> deg</div>

<h2>ESP Internal Temperature</h2>
<canvas id="temp_plot" width="400" height="100" style="border:1px solid #000000;"></canvas>

<h2>StallGuard Value</h2>
<canvas id="sg_plot" width="400" height="100" style="border:1px solid #000000;"></canvas>

<h2>Shaft Angle</h2>
<canvas id="angle_canvas" width="200" height="200" style="border:1px solid #000000;"></canvas>

<script>
const MAX_POINTS = 100;
const update_interval = 200;

let temp_history = [];
let sg_history = [];

function update_status() {
    fetch('/status').then(r => r.json()).then(s => {
        document.getElementById('tmc_temp').innerText = s.tmc_temp.toFixed(1);
        document.getElementById('ir_event').innerText = s.ir ? "Yes" : "No";

        temp_history.push(s.tmc_temp);
        if (temp_history.length > MAX_POINTS) temp_history.shift();
        draw_temp_plot();

        sg_history.push(s.sg_value);
        if (sg_history.length > MAX_POINTS) sg_history.shift();
        draw_sg_plot();
    });
}

function draw_generic_plot({canvas_id, history, min_val, max_val, color, y_ticks = 5}) {
    const canvas = document.getElementById(canvas_id);
    const ctx = canvas.getContext('2d');
    const W = canvas.width;
    const H = canvas.height;

    ctx.clearRect(0,0,W,H);

    // draw y-axis ticks and labels
    ctx.strokeStyle = 'black';
    ctx.fillStyle = 'black';
    ctx.lineWidth = 1;
    ctx.font = '10px sans-serif';

    for (let i = 0; i <= y_ticks; i++) {
        const t = i / y_ticks;
        const y = H - t * H;
        const val = min_val + t * (max_val - min_val);

        // tick line
        ctx.beginPath();
        ctx.moveTo(0, y);
        ctx.lineTo(5, y);
        ctx.stroke();

        // value label
        ctx.fillText(val.toFixed(0), 8, y + 3);
    }

    if (history.length === 0) return;

    // plot the line
    ctx.beginPath();
    history.forEach((v,i)=>{
        const x = (i/(MAX_POINTS-1))*W;
        const y = H - ((v-min_val)/(max_val-min_val))*H;
        if(i===0) ctx.moveTo(x,y);
        else ctx.lineTo(x,y);
    });

    ctx.strokeStyle = color;
    ctx.lineWidth = 2;
    ctx.stroke();
}

function draw_temp_plot() {
    draw_generic_plot({
        canvas_id: "temp_plot",
        history: temp_history,
        min_val: 0,
        max_val: 80,
        color: "red"
    });
}

function draw_sg_plot() {
    draw_generic_plot({
        canvas_id: "sg_plot",
        history: sg_history,
        min_val: 0,
        max_val: 500,
        color: "blue"
    });
}

function draw_angle(angle){
    const canvas = document.getElementById('angle_canvas');
    const ctx = canvas.getContext('2d');
    const w = canvas.width, h = canvas.height;
    const cx = w/2, cy = h/2;
    const r = Math.min(cx, cy) - 10;

    ctx.clearRect(0, 0, w, h);

    // outer circle
    ctx.beginPath();
    ctx.arc(cx, cy, r, 0, 2*Math.PI);
    ctx.strokeStyle = 'black';
    ctx.lineWidth = 2;
    ctx.stroke();

    // pointer (counterclockwise)
    const rad = (-angle - 90) * Math.PI / 180;  // <-- flip the angle
    ctx.beginPath();
    ctx.moveTo(cx, cy);
    ctx.lineTo(cx + r * Math.cos(rad), cy + r * Math.sin(rad));
    ctx.strokeStyle = 'red';
    ctx.lineWidth = 3;
    ctx.stroke();
}

function update_angle(){
    fetch('/angle').then(r => r.json()).then(s => {
        draw_angle(s.angle);
    });
}

function update_total_angle() {
    fetch('/total_angle').then(r => r.json()).then(s => {
        document.getElementById('total_angle').innerText = s.total_angle.toFixed(1);
    });
}

// Update periodically, same interval as other updates
setInterval(update_total_angle, update_interval);
update_total_angle();

setInterval(update_status, update_interval);
setInterval(update_angle, update_interval);
update_status();
update_angle();
</script>

</body>
</html>
)rawliteral";

    server->on("/", HTTP_GET, [server]() {
        server->send_P(200, "text/html", INDEX_HTML);
    });

    server->on("/led1", HTTP_POST, [toggle_func, server]() {
        toggle_func();
        server->send(200);
    });

    server->on("/status", HTTP_GET,
        [server, get_temperature, get_ir_triggered, get_stall_guard_value]() {
            String json = "{";
            json += "\"tmc_temp\":" + String(get_temperature(),1) + ",";
            json += "\"ir\":" + String(get_ir_triggered() ? "true":"false") + ",";
            json += "\"sg_value\":" + String(get_stall_guard_value());
            json += "}";
            server->send(200,"application/json",json);
        }
    );

    server->on("/angle", HTTP_GET, [server, shaft_angle_ptr]() {
        server->send(
            200,
            "application/json",
            String("{\"angle\":") + String(*shaft_angle_ptr, 1) + "}"
        );
    });

    server->on("/total_angle", HTTP_GET, [server, total_angle_ptr]() {
        server->send(
            200,
            "application/json",
            String("{\"total_angle\":") + String(*total_angle_ptr, 1) + "}"
        );
    });
}
