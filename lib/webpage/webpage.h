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
    std::function<bool()> get_stall_guard_interrupt,
    std::function<uint8_t()> get_tmc_version,
    std::function<float()> get_angle,
    std::function<int()> get_stall_guard_value,
    std::function<void()> move_motor_func
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
<button onclick="fetch('/move_motor', {method:'POST'})">Spin Motor</button>

<h2>Status</h2>
<div>Internal Temp: <span id="tmc_temp">0.0</span> C</div>
<div>IR Sensor: <span id="ir_event">No</span></div>
<div>StallGuard: <span id="sg_event">No</span></div>
<div>TMC Version: <span id="tmc_version">0x00</span></div>

<h2>ESP Internal Temperature Plot</h2>
<canvas id="temp_plot" width="400" height="100" style="border:1px solid #000000;"></canvas>

<h2>StallGuard</h2>
<canvas id="sg_plot" width="400" height="100" style="border:1px solid #000000;"></canvas>

<h2>Shaft Angle</h2>
<canvas id="angle_canvas" width="200" height="200" style="border:1px solid #000000;"></canvas>

<script>
const MAX_POINTS = 100;
update_interval = 1000;
let temp_history = [];
let sg_history = [];

function update_status() {
    fetch('/status').then(r => r.json()).then(s => {
        let temp = s.tmc_temp;
        document.getElementById('tmc_temp').innerText = temp.toFixed(1);
        document.getElementById('ir_event').innerText = s.ir ? "Yes" : "No";
        document.getElementById('sg_event').innerText = s.stall_guard ? "Yes" : "No";
        document.getElementById('tmc_version').innerText = "0x" + s.tmc_version.toString(16).padStart(2,'0');

        temp_history.push(temp);
        if(temp_history.length > MAX_POINTS) temp_history.shift();
        draw_temp_plot();

        let sg_val = s.sg_value;  // <-- read the numeric value
        sg_history.push(sg_val);
        if(sg_history.length > MAX_POINTS) sg_history.shift();
        draw_sg_plot();
    });
}

function draw_generic_plot({
    canvas_id,
    history,
    min_val,
    max_val,
    units = "",
    color = "red",
    y_ticks = 5
}) {
    const canvas = document.getElementById(canvas_id);
    const ctx = canvas.getContext('2d');
    const W = canvas.width;
    const H = canvas.height;

    ctx.clearRect(0, 0, W, H);

    // axes / ticks
    ctx.strokeStyle = 'black';
    ctx.fillStyle = 'black';
    ctx.lineWidth = 1;
    ctx.font = '10px sans-serif';

    for (let i = 0; i <= y_ticks; i++) {
        const t = i / y_ticks;
        const y = H - t * H;
        const val = min_val + t * (max_val - min_val);

        ctx.beginPath();
        ctx.moveTo(0, y);
        ctx.lineTo(5, y);
        ctx.stroke();

        ctx.fillText(val.toFixed(0) + units, 8, y + 3);
    }

    if (history.length === 0) return;

    ctx.beginPath();
    history.forEach((v, i) => {
        const x = (i / MAX_POINTS) * W;
        const norm = (v - min_val) / (max_val - min_val);
        const y = H - norm * H;
        if (i === 0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
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
        units: " C",
        color: "red"
    });
}

function draw_sg_plot() {
    draw_generic_plot({
        canvas_id: "sg_plot",
        history: sg_history,
        min_val: -10,
        max_val: 400,
        units: "",
        color: "blue",
        y_ticks: 5
    });
}

function draw_angle(angle){
    const canvas = document.getElementById('angle_canvas');
    const ctx = canvas.getContext('2d');
    const w=canvas.width, h=canvas.height, cx=w/2, cy=h/2;
    const radius=Math.min(cx,cy)-10;

    ctx.clearRect(0,0,w,h);

    // circle
    ctx.beginPath();
    ctx.arc(cx,cy,radius,0,2*Math.PI);
    ctx.strokeStyle='black'; ctx.lineWidth=2;
    ctx.stroke();

    // pointer
    const rad=(angle-90)*Math.PI/180;
    const x=cx+radius*Math.cos(rad);
    const y=cy+radius*Math.sin(rad);
    ctx.beginPath();
    ctx.moveTo(cx,cy);
    ctx.lineTo(x,y);
    ctx.strokeStyle='red'; ctx.lineWidth=3;
    ctx.stroke();

    // labels every 90°
    ctx.fillStyle='black'; ctx.font='12px sans-serif';
    ['0','90','180','270'].forEach((label,i)=>{
        const a=i*90-90;
        const lx=cx+(radius+10)*Math.cos(a*Math.PI/180);
        const ly=cy+(radius+10)*Math.sin(a*Math.PI/180);
        ctx.fillText(label,lx-6,ly+4);
    });
}

// periodically update
setInterval(update_status,update_interval);
update_status();

function update_angle(){
    fetch('/angle').then(r=>r.json()).then(s=>{
        draw_angle(s.angle);
    });
}
setInterval(update_angle,update_interval);
update_angle();

</script>
</body>
</html>
)rawliteral";

    server->on("/", HTTP_GET,[server](){
        server->send_P(200,"text/html",INDEX_HTML);
    });

    server->on("/led1", HTTP_POST,[toggle_func,server](){
        toggle_func();
        server->send(200);
    });

    server->on("/status", HTTP_GET,[server,get_temperature, get_stall_guard_value, get_ir_triggered,get_stall_guard_interrupt,get_tmc_version](){
        String json="{";
        json+="\"tmc_temp\":"+String(get_temperature(),1)+",";
        json += "\"ir\":" + String(get_ir_triggered() ? "true" : "false") + ",";
        json += "\"stall_guard\":" + String(get_stall_guard_interrupt() ? "true" : "false") + ",";
        json += "\"sg_value\":" + String(get_stall_guard_value()) + ",";  // <--- add this
        json+="\"tmc_version\":"+String(get_tmc_version());
        json+="}";
        server->send(200,"application/json",json);
    });

    server->on("/angle", HTTP_GET,[server,get_angle](){
        String json="{";
        json+="\"angle\":"+String(get_angle());
        json+="}";
        server->send(200,"application/json",json);
    });

    server->on("/move_motor", HTTP_POST, [move_motor_func, server]() {
    move_motor_func();   // just call the preloaded function
    server->send(200);   // respond to browser
});
}
