window.chartColors = {
    red: 'rgb(255, 29, 0)',
    redt: 'rgb(255, 29, 0, 0.55)',
    orange: 'rgb(255, 159, 64)',
    oranget: 'rgb(255, 159, 64, 0.55)',
    darkorange: 'rgb(255, 140, 0)',
    darkoranget: 'rgb(255, 140, 0, 0.55)',
    orangered: 'rgb(255, 69, 0)',
    orangeredt: 'rgb(255, 69, 0, 0.55)',
    yellow: 'rgb(255, 205, 86)',
    yellowt: 'rgb(255, 205, 86, 0.55)',
    green: 'rgb(75, 192, 192)',
    greent: 'rgb(75, 192, 192, 0.55)',
    blue: 'rgb(54, 162, 235)',
    purple: 'rgb(153, 102, 255)',
    grey: 'rgb(201, 203, 207)',
    greyt: 'rgb(201, 203, 207, 0.55)',
    white: 'rgb(255, 255, 255)',
    whitet: 'rgb(255, 255, 255, 0.55)',
    limegreen: 'rgb(50, 205, 50)',
    limegreent: 'rgb(50, 205, 50, 0.55)',
    gold: 'rgb(255, 215, 0)',
    goldt: 'rgb(255, 215, 0, 0.55)',
    black: 'rgb(0, 0, 0)',
    blackt: 'rgb(0, 0, 0, 0.55)',
    lightsteelblue: 'rgb(176,192,222)',
    lightsteelbluet: 'rgb(176,192,222, 0.55)',
};

var maxHeartRate = 190;
var heartZones = [];
var miles = 1;
var heartChart = null;
var heartTargetAbove = 0;
var heartTargetBelow = 0;

function ensureHeartZones() {
    maxHeartRate = Number(maxHeartRate);
    if (!Number.isFinite(maxHeartRate) || maxHeartRate <= 0) {
        maxHeartRate = 190;
    }

    const defaultZoneRatios = [0.70, 0.80, 0.90, 1.00];
    for (let i = 0; i < defaultZoneRatios.length; i++) {
        heartZones[i] = Number(heartZones[i]);
        if (!Number.isFinite(heartZones[i]) || heartZones[i] <= 0) {
            heartZones[i] = Math.round(maxHeartRate * defaultZoneRatios[i]);
        }
    }
}

function positiveHeartTarget(value) {
    const target = Number(value);
    return Number.isFinite(target) && target > 0 ? target : 0;
}

function setHeartThresholdAnnotation(id, target, label) {
    if (!heartChart || !heartChart.options.plugins.annotation) {
        return;
    }

    const annotations = heartChart.options.plugins.annotation.annotations;
    if (target > 0) {
        annotations[id] = {
            type: 'line',
            scaleID: 'y',
            value: target,
            borderColor: window.chartColors.black,
            borderWidth: 2,
            borderDash: [6, 6],
            label: {
                enabled: true,
                content: label,
                position: 'end',
                backgroundColor: window.chartColors.blackt,
                color: window.chartColors.white,
                xPadding: 6,
                yPadding: 4,
                yAdjust: id === 'heartTargetAbove' ? -10 : 10,
            },
        };
    } else {
        delete annotations[id];
    }
}

function updateHeartThresholdAnnotations(arr) {
    heartTargetAbove = positiveHeartTarget(arr.target_heart_above);
    heartTargetBelow = positiveHeartTarget(arr.target_heart_below);

    setHeartThresholdAnnotation('heartTargetAbove', heartTargetAbove, 'HR >' + heartTargetAbove + ' bpm');
    setHeartThresholdAnnotation('heartTargetBelow', heartTargetBelow, 'HR <' + heartTargetBelow + ' bpm');

    const maxTarget = Math.max(heartTargetAbove, heartTargetBelow);
    if (maxTarget > heartChart.options.scales.y.max) {
        heartChart.options.scales.y.max = maxTarget + 5;
        heartChart.options.plugins.annotation.annotations.box5.yMax = heartChart.options.scales.y.max;
    }
    if (maxTarget > 0 && maxTarget < heartChart.options.scales.y.min) {
        heartChart.options.scales.y.min = Math.max(0, maxTarget - 5);
    }
}

function t(key, fallback) {
    return window.qzTranslate ? window.qzTranslate(key, fallback) : fallback;
}

function process_trainprogram_heart(arr) {
    let elapsed = 0;
    for (const segment of arr.list) {
        elapsed += segment.duration_s;
    }

    if(elapsed > 0) {
        if (typeof setNormalXScale === 'function') {
            setNormalXScale('heart', undefined, elapsed);
        }
        if (typeof isZoomedHeart === 'undefined' || !isZoomedHeart) {
            heartChart.options.scales.x.min = undefined;
            heartChart.options.scales.x.max = elapsed;
            heartChart.update();
        }
    }
}

function process_arr_heart(arr) {    
    // Try to get the active canvas - check all possible canvas IDs
    let ctx, div;
    if (document.getElementById('canvasheart') && document.getElementById('canvasheart').offsetParent !== null) {
        ctx = document.getElementById('canvasheart').getContext('2d');
        div = document.getElementById('divcanvasheart');
    } else if (document.getElementById('canvasheartFull') && document.getElementById('canvasheartFull').offsetParent !== null) {
        ctx = document.getElementById('canvasheartFull').getContext('2d');
        div = document.getElementById('divcanvasheartFull');
    } else {
        // Fallback to the first available canvas
        ctx = (document.getElementById('canvasheart') || document.getElementById('canvasheartFull')).getContext('2d');
        div = document.getElementById('divcanvasheart') || document.getElementById('divcanvasheartFull');
    }

    let heart = [];
    let maxEl = 0;
    for (const sample of arr) {
        const time = sample.elapsed_s + sample.elapsed_m * 60 + sample.elapsed_h * 3600;
        heart.push({x: time, y: sample.heart});
        maxEl = time;
    }

    const maxRecordedHeart = heart.reduce(function(maxValue, point) {
        return Math.max(maxValue, point.y);
    }, 0);
    const minRecordedHeart = heart.reduce(function(minValue, point) {
        return point.y > 0 ? Math.min(minValue, point.y) : minValue;
    }, Number.POSITIVE_INFINITY);
    ensureHeartZones();
    const heartTrainingFloor = Math.round(maxHeartRate * 0.5);
    const heartChartBottom = Math.max(
        0,
        Math.min(heartTrainingFloor, Number.isFinite(minRecordedHeart) ? minRecordedHeart - 5 : heartTrainingFloor)
    );
    const heartChartTop = Math.max(
        maxHeartRate,
        maxRecordedHeart > maxHeartRate ? maxRecordedHeart + 5 : maxHeartRate,
        heartChartBottom + 50
    );
    const heartZoneLabelPositions = [
        Math.round((heartChartBottom + heartZones[0]) / 2),
        Math.round((heartZones[0] + heartZones[1]) / 2),
        Math.round((heartZones[1] + heartZones[2]) / 2),
        Math.round((heartZones[2] + heartZones[3]) / 2),
        Math.round((heartZones[3] + heartChartTop) / 2)
    ];

    const backgroundFill = {
      id: 'custom_canvas_background_color',
      beforeDraw: (chart) => {
        const ctx = chart.canvas.getContext('2d');
        ctx.save();
        ctx.globalCompositeOperation = 'destination-over';
        ctx.fillStyle = 'white';
        ctx.fillRect(0, 0, chart.width, chart.height);
        ctx.restore();
      }
    };

    config = {
        type: 'line',
        plugins: [backgroundFill],
        data: {
            datasets: [{
                label: t('metric.heart', 'Heart'),
                backgroundColor: window.chartColors.red,
                borderColor: window.chartColors.red,
                //cubicInterpolationMode: 'monotone',
                data: heart,
                fill: false,
                pointRadius: 0,
                borderWidth: 2,
                segment: {
                   borderColor: ctx => ctx.p0.parsed.y < heartZones[0] && ctx.p1.parsed.y < heartZones[0] ? window.chartColors.lightsteelblue :
                                                                       ctx.p0.parsed.y < heartZones[1] && ctx.p1.parsed.y < heartZones[1] ? window.chartColors.green :
                                                                       ctx.p0.parsed.y < heartZones[2] && ctx.p1.parsed.y < heartZones[2] ? window.chartColors.yellow :
                                                                       ctx.p0.parsed.y < heartZones[3] && ctx.p1.parsed.y < heartZones[3] ? window.chartColors.orange :
                                                                       window.chartColors.red,
                }
            }]
        },
        options: {
            animation: {
              onComplete: function() {
                  // Live charts should not auto-save during workout
              }
            },
            responsive: true,
            maintainAspectRatio: false,
            grid: {
                zeroLineColor: 'rgba(0,255,0,1)'
            },
            plugins: {
                tooltip: {
                    enabled: false,
                },
                legend: {
                    display: false
                },
                annotation: {
                        annotations: {
                            box1: {
                            // Indicates the type of annotation
                            type: 'box',
                            adjustScaleRange: false,
                            yMin: heartChartBottom,
                            yMax: heartZones[0],
                            backgroundColor: window.chartColors.lightsteelbluet,
                            },
                            box2: {
                            // Indicates the type of annotation
                            type: 'box',
                            adjustScaleRange: false,
                            yMin: heartZones[0],
                            yMax: heartZones[1],
                            backgroundColor: window.chartColors.greent,
                            },
                            box3: {
                            // Indicates the type of annotation
                            type: 'box',
                            adjustScaleRange: false,
                            yMin: heartZones[1],
                            yMax: heartZones[2],
                            backgroundColor: window.chartColors.yellowt,
                            },
                            box4: {
                            // Indicates the type of annotation
                            type: 'box',
                            adjustScaleRange: false,
                            yMin: heartZones[2],
                            yMax: heartZones[3],
                            backgroundColor: window.chartColors.oranget,
                            },
                            box5: {
                            // Indicates the type of annotation
                            type: 'box',
                            adjustScaleRange: false,
                            yMin: heartZones[3],
                            yMax: heartChartTop,
                            backgroundColor: window.chartColors.redt,
                            },
                    }
                },
            },
            hover: {
                mode: 'nearest',
                intersect: true
            },
            scales: {
                x: {
                    type: 'linear',
                    display: true,
                    title: {
                        display: false,
                        text: 'Time'
                    },
                    ticks: {
                        // Include a dollar sign in the ticks
                        callback: function(value, index, values) {
                            return value !== 0 ? Math.floor(value / 3600).toString().padStart(2, "0") + ":" + Math.floor((value / 60) - (Math.floor(value / 3600) * 60)).toString().padStart(2, "0")  : "";
                        },
                        padding: -20,
                        //stepSize: 300,
                        align: "end",
                    },
                    //max: maxEl,
                },
                y: {
                    display: true,
                    min: heartChartBottom,
                    max: heartChartTop,
                    ticks: {
                        stepSize: 1,
                        autoSkip: false,
                        callback: value =>  heartZoneLabelPositions.includes(value) ?
                            value === heartZoneLabelPositions[0] ? 'heart z1' :
                            value === heartZoneLabelPositions[1] ? 'heart z2' :
                            value === heartZoneLabelPositions[2] ? 'heart z3' :
                            value === heartZoneLabelPositions[3] ? 'heart z4' :
                            value === heartZoneLabelPositions[4] ? 'heart z5' : undefined : undefined,
                        color: 'black',
                        padding: -70,
                        align: 'end',
                        z: 1,
                    }
                }
            }
        }
    };

    heartChart = new Chart(ctx, config);
    if (typeof setNormalXScale === 'function') {
        setNormalXScale('heart', heartChart.options.scales.x.min, heartChart.options.scales.x.max);
    }

    refresh_heart();
}

function refresh_heart() {
    el = new MainWSQueueElement({
        msg: null
    }, function(msg) {
        if (msg.msg === 'workout') {
            return msg.content;
        }
        return null;
    }, 2000, 1);
    el.enqueue().then(process_workout_heart).catch(function(err) {
        console.error('Error is ' + err);
        refresh_heart();
    });    
}

function process_workout_heart(arr) {    
    if (!heartChart) {
        setTimeout(refresh_heart, 1000);
        return;
    }
    let elapsed = arr.elapsed_s + (arr.elapsed_m * 60) + (arr.elapsed_h * 3600);
    heartChart.data.datasets[0].data.push({x: elapsed, y: arr.heart});
    updateHeartThresholdAnnotations(arr);
    if (typeof setNormalXScale === 'function' && typeof normalXScales !== 'undefined' &&
        normalXScales.heart.max !== undefined && elapsed > normalXScales.heart.max) {
        setNormalXScale('heart', normalXScales.heart.min, elapsed);
        if (typeof isZoomedHeart === 'undefined' || !isZoomedHeart) {
            heartChart.options.scales.x.max = elapsed;
        }
    }
    if(elapsed > heartChart.options.scales.x.max)
        heartChart.options.scales.x.max = elapsed;
    if(arr.heart > heartChart.options.scales.y.max) {
        heartChart.options.scales.y.max = arr.heart + 5;
        heartChart.options.plugins.annotation.annotations.box5.yMax = arr.heart + 5;
    }
    heartChart.update();
    refresh_heart();
}

function load_heart_training_program() {
    let el = new MainWSQueueElement({
        msg: 'gettrainingprogram'
    }, function(msg) {
        if (msg.msg === 'R_gettrainingprogram') {
            return msg.content;
        }
        return null;
    }, 15000, 3);
    el.enqueue().then(process_trainprogram_heart).catch(function(err) {
        console.error('Error is ' + err);
    });
}

function load_heart_workout_data() {
    let el = new MainWSQueueElement({
        msg: 'getsessionarray'
    }, function(msg) {
        if (msg.msg === 'R_getsessionarray') {
            return msg.content;
        }
        return null;
    }, 15000, 3);
    el.enqueue().then(function(arr) {
        process_arr_heart(arr);
        load_heart_training_program();
    }).catch(function(err) {
        console.error('Error is ' + err);
    });
}

function dochartheart_init() {
    keys_arr = ['miles_unit', 'age', 'heart_rate_zone1', 'heart_rate_zone2', 'heart_rate_zone3', 'heart_rate_zone4', 'heart_max_override_enable', 'heart_max_override_value']
    let el = new MainWSQueueElement({
            msg: 'getsettings',
            content: {
                keys: keys_arr
            }
        }, function(msg) {
            if (msg.msg === 'R_getsettings') {
                var heart_max_override_enable = false;
                var heart_max_override_value = 195;
                var heart_rate_zone1 = 0;
                var heart_rate_zone2 = 0;
                var heart_rate_zone3 = 0;
                var heart_rate_zone4 = 0;

                for (let key of keys_arr) {
                    if (msg.content[key] === undefined)
                        return null;
                    if (key === 'age') {
                        age = msg.content[key];
                        maxHeartRate = 220 - age;
                    } else if (key === 'heart_max_override_enable') {
                        heart_max_override_enable = (msg.content[key] === true || msg.content[key] === 'true');
                    } else if (key === 'heart_max_override_value') {
                        heart_max_override_value = msg.content[key];
                    } else if (key === 'heart_rate_zone1') {
                        heart_rate_zone1 = msg.content[key];
                        heartZones[0] = Math.round(maxHeartRate * (msg.content[key] / 100));
                    } else if (key === 'heart_rate_zone2') {
                        heart_rate_zone2 = msg.content[key];
                        heartZones[1] = Math.round(maxHeartRate * (msg.content[key] / 100));
                    } else if (key === 'heart_rate_zone3') {
                        heart_rate_zone3 = msg.content[key];
                        heartZones[2] = Math.round(maxHeartRate * (msg.content[key] / 100));
                    } else if (key === 'heart_rate_zone4') {
                        heart_rate_zone4 = msg.content[key];
                        heartZones[3] = Math.round(maxHeartRate * (msg.content[key] / 100));
                    } else if (key === 'miles_unit') {
                        if(msg.content[key] === true || msg.content[key] === 'true')
                            miles = 0.621371;
                    }
                }
                if(heart_max_override_enable) {
                    maxHeartRate = heart_max_override_value;
                    heartZones[0] = Math.round(maxHeartRate * (heart_rate_zone1 / 100));
                    heartZones[1] = Math.round(maxHeartRate * (heart_rate_zone2 / 100));
                    heartZones[2] = Math.round(maxHeartRate * (heart_rate_zone3 / 100));
                    heartZones[3] = Math.round(maxHeartRate * (heart_rate_zone4 / 100));
                }
                return msg.content;
            }
            return null;
        }, 5000, 3);
    el.enqueue().then(function() {
        ensureHeartZones();
        load_heart_workout_data();
    }).catch(function(err) {
            console.error('Error is ' + err);
            ensureHeartZones();
            load_heart_workout_data();
    })
}


$(window).on('load', function () {
    dochartheart_init(); return;


});

$(document).ready(function () {
    $('#loading').hide();
});
