// HTML content for I2C light setup page
#pragma once
#include <pgmspace.h>

const char http_content_i2c[] PROGMEM = R"=====(
<!doctype html>
<html>

<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Light Setup</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      max-width: 600px;
      margin: 20px auto;
      padding: 20px
    }

    label {
      display: inline-block;
      width: 100px
    }

    input,
    select {
      margin: 5px 0;
      padding: 5px
    }

    .btn {
      background: #4CAF50;
      color: white;
      padding: 10px 20px;
      border: none;
      cursor: pointer;
      margin: 5px
    }

    .btn:hover {
      background: #45a049
    }

    .off {
      background: #f44336
    }

    .off:hover {
      background: #da190b
    }
    .switch {
      position: relative;
      display: inline-block;
      width: 50px;
      height: 24px;
      vertical-align: middle;
      margin-right: 8px;
    }
    .switch input { display: none; }
    .slider {
      position: absolute;
      cursor: pointer;
      top: 0; left: 0; right: 0; bottom: 0;
      background: #ccc;
      transition: .4s;
      border-radius: 24px;
    }
    .slider:before {
      position: absolute;
      content: "";
      height: 18px;
      width: 18px;
      left: 3px;
      bottom: 3px;
      background: white;
      transition: .4s;
      border-radius: 50%;
    }
    .switch input:checked + .slider { background: #4CAF50; }
    .switch input:checked + .slider:before { transform: translateX(26px); }
  </style>

  <script>
    document.addEventListener('DOMContentLoaded', function () {
      var s = document.getElementById('bri');
      var o = document.getElementById('briValue');
      if (s && o) o.textContent = s.value;
    var onSwitch = document.getElementById('onSwitch');
    var onLabel = document.getElementById('onLabel');
    var onValue = document.getElementById('onValue');
    if (onSwitch && onLabel && onValue) {
      onLabel.textContent = onSwitch.checked ? 'On' : 'Off';
      onValue.value = onSwitch.checked ? 'true' : 'false';
    }
    // populate controls from current device state (light 1) and config
    try {
      fetch('/state?light=1').then(function(resp){
        if (!resp.ok) return;
        return resp.json();
      }).then(function(data){
        if (!data) return;
        if (s && o && data.bri !== undefined) {
          s.value = data.bri;
          o.textContent = data.bri;
        }
        if (onSwitch && onLabel && onValue && data.on !== undefined) {
          onSwitch.checked = data.on ? true : false;
          onLabel.textContent = onSwitch.checked ? 'On' : 'Off';
          onValue.value = onSwitch.checked ? 'true' : 'false';
        }
      }).catch(function(){/* ignore */});

      fetch('/config').then(function(resp){
        if (!resp.ok) return;
        return resp.json();
      }).then(function(cfg){
        if (!cfg) return;
        var startup = document.querySelector('select[name="startup"]');
        var scene = document.querySelector('select[name="scene"]');
        if (startup && cfg.startup !== undefined) startup.value = cfg.startup;
        if (scene && cfg.scene !== undefined) scene.value = cfg.scene;
      }).catch(function(){/* ignore */});
    } catch(e) {}
    });
  </script>
</head>

<body>
  <h2>Light Setup</h2>
  <form method="post">
    <div>
      <label>Power:</label>
      <label class="switch">
        <input type="checkbox" id="onSwitch" checked onchange="document.getElementById('onValue').value=this.checked? 'true' : 'false'; document.getElementById('onLabel').textContent=this.checked? 'On' : 'Off'">
        <span class="slider"></span>
      </label>
      <span id="onLabel">On</span>
      <input type="hidden" name="on" id="onValue" value="true">
    </div>
    <div>
      <label>Startup:</label>
      <select name="startup" onchange="this.form.submit()">
        <option value="0">Last state</option>
        <option value="1">On</option>
        <option value="2">Off</option>
      </select>
    </div>
    <div>
      <label>Scene:</label>
      <select name="scene" onchange="this.form.submit()">
        <option value="0">Relax</option>
        <option value="1">Bright</option>
        <option value="2">Nightly</option>
      </select>
    </div>
    <div>
      <label>Brightness:</label>
      <input id="bri" name="bri" type="range" min="1" max="254" value="128"
        oninput="document.getElementById('briValue').textContent=this.value">
      <span id="briValue">128</span>
    </div>
    <div>
      <button type="submit" class="btn">Save</button>
    </div>
    <div>
      <a href="/?alert=1">Alert</a> | <a href="/?reset=1">Reset</a>
    </div>
  </form>
</body>

</html>
)=====";
