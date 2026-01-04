// HTML content for I2C light setup page
#ifndef I2C_HTTP_CONTENT_H
#define I2C_HTTP_CONTENT_H

#pragma once

#include <pgmspace.h>

const char http_content_i2c[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>I2C Dimmable Light - DiyHue</title>
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
                <a href="#" class="brand-logo">DiyHue Dimmable I2C</a>
                <ul id="nav-mobile" class="right hide-on-med-and-down" style="position: relative;z-index: 10;">
                    <li><a target="_blank" href="https://github.com/diyhue"><i class="material-icons left">language</i>GitHub</a></li>
                    <li><a target="_blank" href="https://diyhue.readthedocs.io/en/latest/"><i class="material-icons left">description</i>Documentation</a></li>
                    <li><a target="_blank" href="https://diyhue.slack.com/"><i class="material-icons left">question_answer</i>Slack channel</a></li>
                </ul>
            </div>
            <div class="nav-content">
                <ul class="tabs tabs-transparent">
                    <li class="tab" title="#home"><a class="active" href="#home">Home</a></li>
                    <li class="tab" title="#preferences"><a href="#preferences">Preferences</a></li>
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
                                    <label>
                                        Off
                                        <input type="checkbox" name="pow" id="pow" value="1">
                                        <span class="lever"></span>
                                        On
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
                    </form>
                </div>

                <div id="preferences" class="col s12">
                    <form method="POST" action="/">
                        <input type="hidden" name="section" value="1">
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
                                    <option value="1">Bright</option>
                                    <option value="2">Nightly</option>
                                </select>
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
    <script>
        // Define wNumb globally for noUiSlider compatibility
        var wNumb = window.wNumb || function(options) {
            return {
                to: function(value) {
                    return options.decimals ? value.toFixed(options.decimals) : Math.round(value);
                },
                from: function(value) {
                    return parseFloat(value);
                }
            };
        };
    </script>
    <script src="https://raw.githubusercontent.com/diyhue/Lights/master/HTML/diyhue.js"></script>
</body>
</html>
)=====";

#endif // I2C_HTTP_CONTENT_H
