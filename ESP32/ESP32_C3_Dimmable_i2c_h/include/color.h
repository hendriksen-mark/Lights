#ifndef COLOR_H
#define COLOR_H

#pragma once

#include <stdint.h>
#include <Arduino.h>
#include <cmath>
#include <NeoPixelBus.h>


extern RgbColor red;
extern RgbColor green;
extern RgbColor blue;
extern RgbColor yellow;
extern RgbColor cyan;
extern RgbColor magenta;
extern RgbColor orange;
extern RgbColor purple;
extern RgbColor white;
extern RgbColor black;

template <typename T>
void convertHue(T &light) // convert hue / sat values from HUE API to RGB
{
    double hh, p, q, t, ff, s, v;
    long i;

    s = light.sat / 255.0;
    v = light.bri / 255.0;

    if (s <= 0.0)
    { // < is bogus, just shuts up warnings
        light.colors[0] = (int)(v * 255.0);
        light.colors[1] = (int)(v * 255.0);
        light.colors[2] = (int)(v * 255.0);
        return;
    }
    hh = light.hue;
    if (hh >= 65535.0)
        hh = 0.0;
    hh /= 11850.0;
    i = (long)hh;
    ff = hh - i;
    p = v * (1.0 - s);
    q = v * (1.0 - (s * ff));
    t = v * (1.0 - (s * (1.0 - ff)));

    v = v * 255.0;
    t = t * 255.0;
    p = p * 255.0;
    q = q * 255.0;

    switch (i)
    {
    case 0:
        light.colors[0] = (int)(v);
        light.colors[1] = (int)(t);
        light.colors[2] = (int)(p);
        break;
    case 1:
        light.colors[0] = (int)(q);
        light.colors[1] = (int)(v);
        light.colors[2] = (int)(p);
        break;
    case 2:
        light.colors[0] = (int)(p);
        light.colors[1] = (int)(v);
        light.colors[2] = (int)(t);
        break;
    case 3:
        light.colors[0] = (int)(p);
        light.colors[1] = (int)(q);
        light.colors[2] = (int)(v);
        break;
    case 4:
        light.colors[0] = (int)(t);
        light.colors[1] = (int)(p);
        light.colors[2] = (int)(v);
        break;
    case 5:
    default:
        light.colors[0] = (int)(v);
        light.colors[1] = (int)(p);
        light.colors[2] = (int)(q);
        break;
    }
}

template <typename T>
void convertXy(T &light, uint8_t rgb_multiplier[3]) // convert CIE xy values from HUE API to RGB
{
    int optimal_bri = light.bri;
    if (optimal_bri < 5)
    {
        optimal_bri = 5;
    }
    float Y = light.y;
    float X = light.x;
    float Z = 1.0f - light.x - light.y;

    // sRGB D65 conversion
    float r = X * 3.2406f - Y * 1.5372f - Z * 0.4986f;
    float g = -X * 0.9689f + Y * 1.8758f + Z * 0.0415f;
    float b = X * 0.0557f - Y * 0.2040f + Z * 1.0570f;

    // Apply gamma correction
    r = r <= 0.0031308f ? 12.92f * r : (1.0f + 0.055f) * pow(r, (1.0f / 2.4f)) - 0.055f;
    g = g <= 0.0031308f ? 12.92f * g : (1.0f + 0.055f) * pow(g, (1.0f / 2.4f)) - 0.055f;
    b = b <= 0.0031308f ? 12.92f * b : (1.0f + 0.055f) * pow(b, (1.0f / 2.4f)) - 0.055f;

    // Apply multiplier for white correction
    r = r * rgb_multiplier[0] / 100;
    g = g * rgb_multiplier[1] / 100;
    b = b * rgb_multiplier[2] / 100;

    if (r > b && r > g)
    {
        // red is biggest
        if (r > 1.0f)
        {
            g = g / r;
            b = b / r;
            r = 1.0f;
        }
    }
    else if (g > b && g > r)
    {
        // green is biggest
        if (g > 1.0f)
        {
            r = r / g;
            b = b / g;
            g = 1.0f;
        }
    }
    else if (b > r && b > g)
    {
        // blue is biggest
        if (b > 1.0f)
        {
            r = r / b;
            g = g / b;
            b = 1.0f;
        }
    }

    r = r < 0 ? 0 : r;
    g = g < 0 ? 0 : g;
    b = b < 0 ? 0 : b;

    r = r * optimal_bri;
    g = g * optimal_bri;
    b = b * optimal_bri;

    light.colors[0] = (int)(r);
    light.colors[1] = (int)(g);
    light.colors[2] = (int)(b);
}

template <typename T>
void convertCt(T &light, uint8_t rgb_multiplier[3]) // convert ct (color temperature) value from HUE API to RGB
{
    int hectemp = 10000 / light.ct;
    int r, g, b;
    if (hectemp <= 66)
    {
        r = 255;
        g = 99.4708025861 * log(hectemp) - 161.1195681661;
        b = hectemp <= 19 ? 0 : (138.5177312231 * log(hectemp - 10) - 305.0447927307);
    }
    else
    {
        r = 329.698727446 * pow(hectemp - 60, -0.1332047592);
        g = 288.1221695283 * pow(hectemp - 60, -0.0755148492);
        b = 255;
    }

    r = r > 255 ? 255 : r;
    g = g > 255 ? 255 : g;
    b = b > 255 ? 255 : b;

    // Apply multiplier for white correction
    r = r * rgb_multiplier[0] / 100;
    g = g * rgb_multiplier[1] / 100;
    b = b * rgb_multiplier[2] / 100;

    r = r * (light.bri / 255.0f);
    g = g * (light.bri / 255.0f);
    b = b * (light.bri / 255.0f);

    light.colors[0] = (int)(r);
    light.colors[1] = (int)(g);
    light.colors[2] = (int)(b);
}
#endif
