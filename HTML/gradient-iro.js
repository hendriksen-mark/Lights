var _____WB$wombat$assign$function_____=function(name){return (self._wb_wombat && self._wb_wombat.local_init && self._wb_wombat.local_init(name))||self[name];};if(!self.__WB_pmw){self.__WB_pmw=function(obj){this.__WB_source=obj;return this;}}{
let window = _____WB$wombat$assign$function_____("window");
let self = _____WB$wombat$assign$function_____("self");
let document = _____WB$wombat$assign$function_____("document");
let location = _____WB$wombat$assign$function_____("location");
let top = _____WB$wombat$assign$function_____("top");
let parent = _____WB$wombat$assign$function_____("parent");
let frames = _____WB$wombat$assign$function_____("frames");
let opens = _____WB$wombat$assign$function_____("opens");
var slider;
var refreshInterval = 1500;
var multipleLights = false;
var lightscount = 3;
var pixelcount = 0;
let availablepixels = 0;
var InitReload = false;

let colors = ['rgb(81%, 100%, 100%)', 'rgb(100%, 100%, 100%)', 'rgb(100%, 81%, 81%)'];

var colorPicker = new iro.ColorPicker('#picker', {
  colors: colors,
  layout: [{
    component: iro.ui.Wheel,
    options: {}
  }, ]
});

colorPicker.on('input:end', function(color) {
  console.log(color.rgb)
  let points = []
  for (const point of colorPicker.colors) {
    points.push({"color": {"xy": rgb_to_cie(point.red, point.green, point.blue)}});
  }

  postData({'gradient': {'points': points}
  });
});

var ctPicker = new iro.ColorPicker('#ct', {
  layout: [{
    component: iro.ui.Slider,
    options: {
      sliderType: 'kelvin',
      minTemperature: 2300,
      maxTemperature: 8000
    }
  }]
});

ctPicker.on('input:end', function(color) {
  postData({
      ct: Math.floor( 1000000 / color.kelvin )
  });
});

function postData(t) {
    var e = new XMLHttpRequest;
    e.timeout = 2000;
    e.open("PUT", "/state", !0);
    e.setRequestHeader("Content-Type", "application/json");
    console.log(JSON.stringify(t)), e.send(JSON.stringify(t));
}
function postDataMultiple(t) {
    var e = {};
    for (light = 1; light <= lightscount; light += 1) e[light] = t;
    var o = new XMLHttpRequest;
    o.open("PUT", "/state", !0), o.setRequestHeader("Content-Type", "application/json"), console.log(JSON.stringify(e)), o.send(JSON.stringify(e))
}

function getPosition(t) {
    var e = 0,
        a = 0;
    if (t.offsetParent) {
        do {
            e += t.offsetLeft, a += t.offsetTop
        } while (t = t.offsetParent);
        return {
            x: e,
            y: a
        }
    }
}

function rgb_to_cie(t, e, a) {
    var n = .664511 * (t = t > .04045 ? Math.pow((t + .055) / 1.055, 2.4) : t / 12.92) + .154324 * (e = e > .04045 ? Math.pow((e + .055) / 1.055, 2.4) : e / 12.92) + .162028 * (a = a > .04045 ? Math.pow((a + .055) / 1.055, 2.4) : a / 12.92),
        o = .283881 * t + .668433 * e + .047685 * a,
        r = 88e-6 * t + .07231 * e + .986039 * a,
        i = (n / (n + o + r)).toFixed(4),
        s = (o / (n + o + r)).toFixed(4);
    return isNaN(i) && (i = 0), isNaN(s) && (s = 0), {"x": parseFloat(i), "y": parseFloat(s)}
}



function updateStatus() {
    $.ajax({
        type: 'GET',
        url: "/state",
        dataType: "json",
        data: [
            { name: "light", value: "1" },
        ],
        success: function (data) {
            if (!data) {
                return;
            }
            $("#pow").prop("checked", data.on);
            slider.noUiSlider.set(data.bri / 2.54);
            ctPicker.colors[0].kelvin = 1000000 / data.ct;
            if (InitReload)
                location.reload();
            console.log('update state done');
        },
        error: function () {
            console.log('error getting state');
        },
        timeout: 1000
    });
}

function updateConfig() {
    $.ajax({
        url: '/config',
        type: 'GET',
        tryCount: 0,
        retryLimit: 5,
        dataType: 'json',
        contentType: 'application/json',
        success: function (json) {
            pixelcount = json["pixelcount"];
            availablepixels = json["pixelcount"];
            $.each(json, function (key, val) {
                if ($("#" + key).is(':checkbox')) {
                    $("input[type=\"checkbox\"]#" + key).addClass('checked-' + val).prop("checked", !!val);
                } else {
                    $("#" + key).val(val);
                }
                $('select').formSelect();
            });

            var newDividedLight = lightscount;
            var newDividedLights = false;
            for (var n = 0; n < lightscount; n++) {
                var dividedLightElement;
                if (!json["dividedLight_" + n]) {
                    dividedLightElement = $('<div class="col s4 m3"><input type="number" id="dividedLight_' + n + '" class="dividedLight" data-skin="round" name="dividedLight_' + n + '" value="' + Math.floor(availablepixels / newDividedLight) + '"/></div>');
                    newDividedLights = true;
                }
                else {
                    newDividedLight -= 1;
                    dividedLightElement = $('<div class="col s4 m3"><input type="number" id="dividedLight_' + n + '" class="dividedLight" data-skin="round" name="dividedLight_' + n + '" value="' + json["dividedLight_" + n] + '"/></div>');
                }
                availablepixels -= parseInt(json["dividedLight_" + n]);
                $(".dividedLights").append(dividedLightElement);
            }

            if (newDividedLights)
                availablepixels = 'Please first adjust the pixels of the newly added lamps and confirm.'

            $(".availablepixels").html("<b>" + availablepixels + "<b>");

            $(".brand-logo").text(json.name);
            toggleSections($('input[type="checkbox"]'));
        },
        error: function () {
            return;
        },
        timeout: 5000,
    });
    console.log('update config done');
}

function toggleSections(ele) {
    ele.each(function (index) {
        var collapsable = $(this).parents('.row').next('.switchable').find('input[type="text"],input[type="number"]');
        if ($(this).is(":checked")) {
            collapsable.prop("disabled", !!0);
        } else {
            collapsable.prop('disabled', !!1);
        }
    });
}

$(function () {
    var oldValue;
    $('body').on('focus', '.dividedLight', function (cb) {
        oldValue = parseInt(cb.currentTarget.value);
    });

    $('body').on('change', '.dividedLight', function (cb) {
        var summe = parseInt(cb.currentTarget.value) - oldValue;
        if (summe > 0) {
            availablepixels = availablepixels - summe;
        } else {
            availablepixels = availablepixels + Math.abs(summe);
        }
        $(".availablepixels").html("<b>" + availablepixels.toString() + "<b>");
    });

    $('.tab').on('click', function (tab) {
        window.location.href = tab.currentTarget.title;
        $('html, body').scrollTop();
    });


    $('.sidenav').sidenav();

    $('.tabs').tabs();

    $('input[type="checkbox"]').change(function () {
        toggleSections($(this));
    });

    $("button[type=submit").click(function (e) {
        $("button[type=submit").addClass("disabled");
        e.preventDefault();
        var form = $(this).parents('form');
        $.ajax({
            type: "POST",
            url: $(this).parents('form').action,
            data: form.serialize(), // serializes the form's elements.
            success: function (data) {
                M.toast({ html: 'Succesful! Rebooting.', classes: 'teal lighten-2' });
                InitReload = true;
            },
            error: function (data) {
                M.toast({ html: 'Error occured while sending data. Try again.', classes: 'red lighten-2' });
                $("button[type=submit").removeClass("disabled");
            },
            timeout: 5000,
        });
    });

    $("#bri").hide().parent().append("<div class='section'><div id='briSlider'></div></div>");

    slider = $('#briSlider')[0];
    noUiSlider.create(slider, {
        start: 20,
        connect: [true, false],
        step: 1,
        behaviour: 'drag-tap',
        orientation: 'horizontal', // 'horizontal' or 'vertical'
        range: {
            'min': 0,
            'max': 100
        },
        format: wNumb({
            decimals: 0,
        })
    });

    slider.noUiSlider.on('start', function () {
        refreshIntervalId = clearInterval(refreshIntervalId);
    });

    slider.noUiSlider.on('end', function (values, handle) {
        if (refreshIntervalId) {
            clearInterval(refreshIntervalId);
        }
        refreshIntervalId = setInterval(updateStatus, refreshInterval);

        var value = values[handle];
        if (multipleLights) {
            postDataMultiple({
                bri: value * 2.54
            });
        } else {
            postData({
                bri: value * 2.54
            });
        }
    });

    slider.noUiSlider.on('set', function (values, handle) {

    });

    $("#pow").change(function () {
        if (multipleLights) {
            postDataMultiple({
                on: $(this).prop('checked') ? 1 : 0,
            });
        } else {
            postData({
                on: $(this).prop('checked') ? 1 : 0,
            });
        }
    });

    updateConfig();
    updateStatus();

    var refreshIntervalId = setInterval(updateStatus, refreshInterval);
});

}
