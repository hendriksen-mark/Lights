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
  </style>
</head>

<body>
  <h2>Light Setup</h2>
  <form method="post">
    <div>
      <label>Power:</label>
      <a class="btn" href="/?on=true">ON</a>
      <a class="btn off" href="/?on=false">OFF</a>
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
      <input name="bri" type="number" min="1" max="254">
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
