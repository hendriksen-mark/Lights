// HTML content for WS2811 light setup page
#ifndef WS2811_HTTP_CONTENT_H
#define WS2811_HTTP_CONTENT_H

#pragma once

#include <pgmspace.h>

const char htmlContent_ws[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>DiyHue</title>
    <link rel="icon" type="image/png"
        href="https://diyhue.org/wp-content/uploads/2019/11/cropped-Zeichenfl%C3%A4che-4-1-32x32.png" sizes="32x32">
    <link href="https://fonts.googleapis.com/icon?family=Material+Icons" rel="stylesheet">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/materialize/1.0.0/css/materialize.min.css">
    <link rel="stylesheet" href="https://diyhue.org/cdn/nouislider.css" />
</head>

<body>
    <div class="wrapper">
        <nav class="nav-extended row" style="background-color: #26a69a !important;">
            <div class="nav-wrapper col s12">
                <a href="#" class="brand-logo">DiyHue</a>
                <ul id="nav-mobile" class="right hide-on-med-and-down" style="position: relative;z-index: 10;">
                    <li><a target="_blank" href="https://github.com/diyhue"><i
                                class="material-icons left">language</i>GitHub</a></li>
                    <li><a target="_blank" href="https://diyhue.readthedocs.io/en/latest/"><i
                                class="material-icons left">description</i>Documentation</a></li>
                    <li><a target="_blank" href="https://diyhue.slack.com/"><i
                                class="material-icons left">question_answer</i>Slack channel</a></li>
                </ul>
            </div>
            <div class="nav-content">
                <ul class="tabs tabs-transparent">
                    <li class="tab" title="#home"><a class="active" href="#home">Home</a></li>
                    <li class="tab" title="#preferences"><a href="#preferences">Preferences</a></li>
                    <li class="tab" title="/update"><a href="/update">Updater</a></li>
                    <li class="tab" title="/files"><a href="/files">File Manager</a></li>
                </ul>
            </div>
        </nav>
        <ul class="sidenav" id="mobile-demo">
            <li><a target="_blank" href="https://github.com/diyhue">GitHub</a></li>
            <li><a target="_blank" href="https://diyhue.readthedocs.io/en/latest/">Documentation</a></li>
            <li><a target="_blank" href="https://diyhue.slack.com/">Slack channel</a></li>
        </ul>
        <div class="container">
            <div class="section">
                <div id="home" class="col s12">
                    <form>
                        <input type="hidden" name="section" value="1">
                        <div class="row">
                            <div class="col s10">
                                <label for="power">Power</label>
                                <div id="power" class="switch section">
                                    <label> Off
                                        <input type="checkbox" name="pow" id="pow" value="1">
                                        <span class="lever"></span> On
                                    </label>
                                </div>
                            </div>
                        </div>
                        <div class="row">
                            <div class="col s12 m10">
                                <label for="bri">Brightness</label>
                                <input type="text" id="bri" class="js-range-slider" name="bri" value="" />
                            </div>
                        </div>
                        <div class="row">
                            <div class="col s12">
                                <label for="hue">Color</label>
                                <div>
                                    <canvas id="hue" width="320px" height="320px"
                                        style="border:1px solid #d3d3d3;"></canvas>
                                </div>
                            </div>
                        </div>
                        <div class="row">
                            <div class="col s12">
                                <label for="ct">Color Temp</label>
                                <div>
                                    <canvas id="ct" width="320px" height="50px"
                                        style="border:1px solid #d3d3d3;"></canvas>
                                </div>
                            </div>
                        </div>
                    </form>
                </div>
                <div id="preferences" class="col s12">
                    <form method="POST" action="/">
                        <input type="hidden" name="section" value="1">
                        <div class="row">
                            <div class="col s12">
                                <label for="name">Light Name</label>
                                <input type="text" id="name" name="name">
                            </div>
                        </div>
                        <div class="row">
                            <div class="col s12 m6">
                                <label for="startup">Default Power:</label>
                                <select name="startup" id="startup">
                                    <option value="0">Last State</option>
                                    <option value="1">On</option>
                                    <option value="2">Off</option>
                                </select>
                            </div>
                        </div>
                        <div class="row">
                            <div class="col s12 m6">
                                <label for="scene">Default Scene:</label>
                                <select name="scene" id="scene">
                                    <option value="0">Relax</option>
                                    <option value="1">Read</option>
                                    <option value="2">Concentrate</option>
                                    <option value="3">Energize</option>
                                    <option value="4">Bright</option>
                                    <option value="5">Dimmed</option>
                                    <option value="6">Nightlight</option>
                                    <option value="7">Savanna sunset</option>
                                    <option value="8">Tropical twilight</option>
                                    <option value="9">Arctic aurora</option>
                                    <option value="10">Spring blossom</option>
                                </select>
                            </div>
                        </div>
                        <div class="row">
                            <div class="col s4 m3">
                                <label for="pixelcount" class="col-form-label">Pixel count</label>
                                <input type="number" id="pixelcount" name="pixelcount">
                            </div>
                        </div>
                        <div class="row">
                            <div class="col s4 m3">
                                <label for="lightscount" class="col-form-label">Lights count</label>
                                <input type="number" id="lightscount" name="lightscount">
                            </div>
                        </div>
                        <label class="form-label">Light division</label>
                        </br>
                        <label>Available Pixels:</label>
                        <label class="availablepixels">
                            <b>null</b>
                        </label>
                        <div class="row dividedLights"> </div>
                        <div class="row">
                            <div class="col s4 m3">
                                <label for="transitionleds">Transition leds:</label>
                                <select name="transitionleds" id="transitionleds">
                                    <option value="0">0</option>
                                    <option value="2">2</option>
                                    <option value="4">4</option>
                                    <option value="6">6</option>
                                    <option value="8">8</option>
                                    <option value="10">10</option>
                                </select>
                            </div>
                        </div>
                        <div class="row">
                            <div class="col s4 m3">
                                <label for="transms">Transition frame (ms)</label>
                                <input type="number" id="transms" name="transms" min="1" max="1000">
                            </div>
                        </div>
                        <div class="row">
                            <div class="col s4 m3">
                                <label for="rpct" class="form-label">Red multiplier</label>
                                <input type="number" id="rpct" class="js-range-slider" data-skin="round" name="rpct"
                                    value="" />
                            </div>
                            <div class="col s4 m3">
                                <label for="gpct" class="form-label">Green multiplier</label>
                                <input type="number" id="gpct" class="js-range-slider" data-skin="round" name="gpct"
                                    value="" />
                            </div>
                            <div class="col s4 m3">
                                <label for="bpct" class="form-label">Blue multiplier</label>
                                <input type="number" id="bpct" class="js-range-slider" data-skin="round" name="bpct"
                                    value="" />
                            </div>
                        </div>
                        <div class="row">
                            <div class="col s10">
                                <button type="submit" class="waves-effect waves-light btn teal">Save</button>
                            </div>
                        </div>
                    </form>
                </div>
            </div>
        </div>
    </div>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/jquery/3.5.1/jquery.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/materialize/1.0.0/js/materialize.min.js"></script>
    <script src="https://diyhue.org/cdn/nouislider.js"></script>
    <script src="https://diyhue.org/cdn/diyhue.js"></script>
</body>

</html>
)=====";

#endif // WS2811_HTTP_CONTENT_H
