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

function t(key, fallback) {
    return window.qzTranslate ? window.qzTranslate(key, fallback) : fallback;
}

function isTrueSetting(value) {
    return value === true || value === 'true' || value === 1 || value === '1';
}

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

function process_arr(arr) {
    let reqcadence = [];
    let heart = [];
    let cadence = [];
    let speed = [];
    let inclination = [];
    let resistance = [];
    let reqresistance = [];
    let pelotonresistance = [];
    let pelotonreqresistance = [];
    let maxEl = 0;
    let saveScreenshot = [];
    let workoutName = '';
    let workoutStartDate = '';
    let instructorName = '';
    let heart_avg = 0;
    let heart_max = 0;
    let deviceType = 0;
    let cadence_avg = 0;
    let peloton_resistance_avg = 0;
    let resistance_avg = 0;
    let calories = 0;
    let distance = 0;
    saveScreenshot[1] = false;
    saveScreenshot[2] = false;
    saveScreenshot[3] = false;
    saveScreenshot[4] = false;
    saveScreenshot[6] = false;

    for (let el of arr) {
        let reqcadenceel = {};
        let heartel = {};
        let cadenceel = {};
        let resistanceel = {};
        let reqresistanceel = {};
        let pelotonresistanceel = {};
        let pelotonreqresistanceel = {};
        let speedel = {};
        let inclinationel = {};
        let time = el.elapsed_s + el.elapsed_m * 60 + el.elapsed_h * 3600;
        workoutName = el.workoutName;
        workoutStartDate = el.workoutStartDate;
        instructorName = el.instructorName;
        heart_avg = el.heart_avg;
        heart_max = el.heart_max;
        deviceType = el.deviceType;
        peloton_resistance_avg = el.peloton_resistance_avg;
        resistance_avg = el.resistance_avg;
        cadence_avg = el.cadence_avg;
        distance = el.distance;
        calories = el.calories;
        maxEl = time;
        reqcadenceel.x = time;
        reqcadenceel.y = el.req_cadence;
        reqcadence.push(reqcadenceel);
        heartel.x = time;
        heartel.y = el.heart;
        heart.push(heartel);
        cadenceel.x = time;
        cadenceel.y = el.cadence;
        cadence.push(cadenceel);
        resistanceel.x = time;
        resistanceel.y = el.resistance;
        resistance.push(resistanceel);
        reqresistanceel.x = time;
        reqresistanceel.y = el.req_resistance;
        reqresistance.push(reqresistanceel);

        pelotonresistanceel.x = time;
        pelotonresistanceel.y = el.peloton_resistance;
        pelotonresistance.push(pelotonresistanceel);
        pelotonreqresistanceel.x = time;
        pelotonreqresistanceel.y = el.peloton_req_resistance;
        pelotonreqresistance.push(pelotonreqresistanceel);

        speedel.x = time;
        speedel.y = el.speed * miles; // Convert to user's preferred unit (km/h or mph)
        speed.push(speedel);
        inclinationel.x = time;
        inclinationel.y = el.inclination;
        inclination.push(inclinationel);
    }

    $('.workoutName').text(workoutName);
    $('.workoutStartDate').text(workoutStartDate);
    $('.instructorName').text((instructorName));
    if(instructorName.length === 0) {
        if(deviceType === 1)
            $('.workout_image').attr("src","run.png");
        else if(deviceType === 3) 
            $('.workout_image').attr("src","row.png");
        else if(deviceType === 4) 
            $('.workout_image').attr("src","elliptical.png");
        else
            $('.workout_image').attr("src","bike.png");
    }
    $('.workout_image').attr("crossOrigin","anonymous");
    $('.heart_avg').text('Heart Rate AVG: ' + Math.floor(heart_avg));
    $('.heart_max').text('Heart Rate MAX: ' + heart_max);

    $('.summary_calories').text(Math.floor(calories) + ' kcal');
    $('.summary_distance').text((distance * miles).toFixed(1) + (miles === 1 ? ' km' : ' mi'));
    $('.summary_cadence_avg').text(Math.floor(cadence_avg) + ' rpm');
    // Use resistance_avg for ellipticals (deviceType 4), peloton_resistance_avg for bikes/rowers
    $('.summary_resistance_avg').text(Math.floor(deviceType === 4 ? resistance_avg : peloton_resistance_avg) + ' lvl');    

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

    let config;
    let ctx;


    const minRecordedHeart = heart.reduce(function(minValue, point) {
        return point.y > 0 ? Math.min(minValue, point.y) : minValue;
    }, Number.POSITIVE_INFINITY);
    const heartTrainingFloor = Math.round(maxHeartRate * 0.5);
    const heartChartBottom = Math.max(
        0,
        Math.min(heartTrainingFloor, Number.isFinite(minRecordedHeart) ? minRecordedHeart - 5 : heartTrainingFloor)
    );
    const heartChartTop = Math.max(
        maxHeartRate,
        heart_max > maxHeartRate ? heart_max + 5 : maxHeartRate,
        heartChartBottom + 50
    );
    const heartZoneLabelPositions = [
        Math.round((heartChartBottom + heartZones[0]) / 2),
        Math.round((heartZones[0] + heartZones[1]) / 2),
        Math.round((heartZones[1] + heartZones[2]) / 2),
        Math.round((heartZones[2] + heartZones[3]) / 2),
        Math.round((heartZones[3] + heartChartTop) / 2),
    ];
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
                  if(saveScreenshot[1])
                      return;
                  saveScreenshot[1] = true;
                  let el = new MainWSQueueElement({
                      msg: 'savechart',
                      content: {
                          name: 'heart',
                          image: heartChart.toBase64Image()
                      }
                  }, function(msg) {
                      if (msg.msg === 'R_savechart') {
                          return msg.content;
                      }
                      return null;
                  }, 15000, 3);
                  el.enqueue().catch(function(err) {
                      console.error('Error is ' + err);
                  });
              }
            },
            responsive: true,
            aspectRatio: 1.5,
            grid: {
                zeroLineColor: 'rgba(0,255,0,1)'
            },
            plugins: {
                title:{
                    display:true,
                    text:'Heart Rate'
                },
                tooltips: {
                    mode: 'index',
                    intersect: false,
                },
                legend: {
                    display: false
                },
                annotation: {
                        annotations: {
                            box1: {
                            // Indicates the type of annotation
                            type: 'box',
                            xMin: 0,
                            xMax: maxEl,
                            yMin: heartChartBottom,
                            yMax: heartZones[0],
                            backgroundColor: window.chartColors.lightsteelbluet,
                            },
                            box2: {
                            // Indicates the type of annotation
                            type: 'box',
                            xMin: 0,
                            xMax: maxEl,
                            yMin: heartZones[0],
                            yMax: heartZones[1],
                            backgroundColor: window.chartColors.greent,
                            },
                            box3: {
                            // Indicates the type of annotation
                            type: 'box',
                            xMin: 0,
                            xMax: maxEl,
                            yMin: heartZones[1],
                            yMax: heartZones[2],
                            backgroundColor: window.chartColors.yellowt,
                            },
                            box4: {
                            // Indicates the type of annotation
                            type: 'box',
                            xMin: 0,
                            xMax: maxEl,
                            yMin: heartZones[2],
                            yMax: heartZones[3],
                            backgroundColor: window.chartColors.oranget,
                            },
                            box5: {
                            // Indicates the type of annotation
                            type: 'box',
                            xMin: 0,
                            xMax: maxEl,
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
                    max: maxEl,
                },
                y: {
                    display: true,
                    title: {
                        display: false,
                        text: 'Heart rate'
                    },
                    min: heartChartBottom,
                    max: heartChartTop,
                    ticks: {
                        stepSize: 1,
                        autoSkip: false,
                        callback: value => heartZoneLabelPositions.includes(value) ?
                            value === heartZoneLabelPositions[0] ? 'zone 1' :
                            value === heartZoneLabelPositions[1] ? 'zone 2' :
                            value === heartZoneLabelPositions[2] ? 'zone 3' :
                            value === heartZoneLabelPositions[3] ? 'zone 4' :
                            value === heartZoneLabelPositions[4] ? 'zone 5' : undefined : undefined,
                        color: 'black',
                        padding: -50,
                        align: 'end',
                        z: 1,
                    }
                }
            }
        }
    };

    ctx = document.getElementById('canvasHeart').getContext('2d');
    var heartChart = new Chart(ctx, config);

    config = {
        type: 'line',
        plugins: [backgroundFill],
        data: {
            datasets: [
                {
                    label: t('workoutEditor.resistance', 'Resistance'),
                    //cubicInterpolationMode: 'monotone',
                    data: resistance,
                    fill: false,
                    pointRadius: 0,
                    borderWidth: 2,
                    backgroundColor: window.chartColors.red,
                    borderColor: window.chartColors.red,
                },
                {
                    label: t('chart.targetResistanceShort', 'Target R.'),
                    //cubicInterpolationMode: 'monotone',
                    data: reqresistance,
                    fill: false,
                    pointRadius: 0,
                    borderWidth: 2,
                    backgroundColor: window.chartColors.black,
                    borderColor: window.chartColors.black,
                },
            ]
        },
        options: {
            animation: {
              onComplete: function() {
                  if(saveScreenshot[2])
                      return;
                  saveScreenshot[2] = true;
                  let el = new MainWSQueueElement({
                      msg: 'savechart',
                      content: {
                          name: 'resistance',
                          image: resistanceChart.toBase64Image()
                      }
                  }, function(msg) {
                      if (msg.msg === 'R_savechart') {
                          return msg.content;
                      }
                      return null;
                  }, 15000, 3);
                  el.enqueue().catch(function(err) {
                      console.error('Error is ' + err);
                  });
              }
            },
            responsive: true,
            aspectRatio: 1.5,
            grid: {
                zeroLineColor: 'rgba(0,255,0,1)'
            },
            plugins: {
                title:{
                    display:true,
                    text:'Resistance vs Target Resistance'
                },
                tooltips: {
                    mode: 'index',
                    intersect: false,
                },
                legend: {
                    display: false,
                    position: 'bottom',
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
                    max: maxEl,
                },
                y: {
                    display: true,
                    title: {
                        display: false,
                    },
                    min: 0,
                    ticks: {
                        stepSize: 10,
                        autoSkip: false,
                        color: 'black',
                        padding: -30,
                        align: 'end',
                        z: 1,
                        callback: function(value, index, values) {
                            return value !== 0 ? value  : "";
                        },
                    }
                }
            }
        }
    };

    ctx = document.getElementById('canvasResistance').getContext('2d');
    var resistanceChart = new Chart(ctx, config);

    config = {
        type: 'line',
        plugins: [backgroundFill],
        data: {
            datasets: [
                {
                    label: t('workoutEditor.resistance', 'Resistance'),
                    //cubicInterpolationMode: 'monotone',
                    data: pelotonresistance,
                    fill: false,
                    pointRadius: 0,
                    borderWidth: 2,
                    backgroundColor: window.chartColors.red,
                    borderColor: window.chartColors.red,
                },
                {
                    label: t('chart.targetResistanceShort', 'Target R.'),
                    //cubicInterpolationMode: 'monotone',
                    data: pelotonreqresistance,
                    fill: false,
                    pointRadius: 0,
                    borderWidth: 2,
                    backgroundColor: window.chartColors.black,
                    borderColor: window.chartColors.black,
                },
            ]
        },
        options: {
            animation: {
              onComplete: function() {
                  if(saveScreenshot[3])
                      return;
                  saveScreenshot[3] = true;
                  let el = new MainWSQueueElement({
                      msg: 'savechart',
                      content: {
                          name: 'pelotonresistance',
                          image: pelotonresistanceChart.toBase64Image()
                      }
                  }, function(msg) {
                      if (msg.msg === 'R_savechart') {
                          return msg.content;
                      }
                      return null;
                  }, 15000, 3);
                  el.enqueue().catch(function(err) {
                      console.error('Error is ' + err);
                  });
              }
            },
            responsive: true,
            aspectRatio: 1.5,
            grid: {
                zeroLineColor: 'rgba(0,255,0,1)'
            },
            plugins: {
                title:{
                    display:true,
                    text:'Peloton Resistance vs Target Peloton Resistance'
                },
                tooltips: {
                    mode: 'index',
                    intersect: false,
                },
                legend: {
                    display: false,
                    position: 'bottom',
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
                    max: maxEl,
                },
                y: {
                    display: true,
                    title: {
                        display: false,
                    },
                    min: 0,
                    ticks: {
                        stepSize: 10,
                        autoSkip: false,
                        color: 'black',
                        padding: -30,
                        align: 'end',
                        z: 1,
                        callback: function(value, index, values) {
                            return value !== 0 ? value  : "";
                        },
                    }
                }
            }
        }
    };

    ctx = document.getElementById('canvasPelotonResistance').getContext('2d');
    var pelotonresistanceChart = new Chart(ctx, config);

    config = {
        type: 'line',
        plugins: [backgroundFill],
        data: {
            datasets: [
                {
                    backgroundColor: window.chartColors.blue,
                    borderColor: window.chartColors.blue,
                    label: t('workoutEditor.cadence', 'Cadence'),
                    //cubicInterpolationMode: 'monotone',
                    data: cadence,
                    fill: false,
                    pointRadius: 0,
                    borderWidth: 2,
                },
                {
                    backgroundColor: window.chartColors.black,
                    borderColor: window.chartColors.black,
                    label: t('chart.targetCadenceShort', 'Target C.'),
                    //cubicInterpolationMode: 'monotone',
                    data: reqcadence,
                    fill: false,
                    pointRadius: 0,
                    borderWidth: 2,
                },
            ]
        },
        options: {
            animation: {
              onComplete: function() {
                  if(saveScreenshot[4])
                      return;
                  saveScreenshot[4] = true;
                  let el = new MainWSQueueElement({
                      msg: 'savechart',
                      content: {
                          name: 'cadence',
                          image: cadenceChart.toBase64Image()
                      }
                  }, function(msg) {
                      if (msg.msg === 'R_savechart') {
                          return msg.content;
                      }
                      return null;
                  }, 15000, 3);
                  el.enqueue().catch(function(err) {
                      console.error('Error is ' + err);
                  });
              }
            },
            responsive: true,
            aspectRatio: 1.5,
            grid: {
                zeroLineColor: 'rgba(0,255,0,1)'
            },
            plugins: {
                title:{
                    display:true,
                    text:'Cadence vs Target Cadence'
                },
                tooltips: {
                    mode: 'index',
                    intersect: false,
                },
                legend: {
                    display: false,
                    position: 'bottom',
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
                    max: maxEl,
                },
                y: {
                    display: true,
                    title: {
                        display: false,
                    },
                    min: 0,
                    ticks: {
                        stepSize: 10,
                        autoSkip: false,
                        color: 'black',
                        padding: -30,
                        align: 'end',
                        z: 1,
                        callback: function(value, index, values) {
                            return value !== 0 ? value  : "";
                        },
                    }
                }
            }
        }
    };

    ctx = document.getElementById('canvasCadence').getContext('2d');
    var cadenceChart = new Chart(ctx, config);



    config = {
        type: 'line',
        plugins: [backgroundFill],
        data: {
            datasets: [
                {
                    backgroundColor: window.chartColors.blue,
                    borderColor: window.chartColors.blue,
                    label: miles === 1 ? t('workoutEditor.speedKmh', 'Speed (km/h)') : t('workoutEditor.speedMph', 'Speed (mph)'),
                    //cubicInterpolationMode: 'monotone',
                    data: speed,
                    fill: false,
                    pointRadius: 0,
                    borderWidth: 2,
                },
                {
                    backgroundColor: window.chartColors.green,
                    borderColor: window.chartColors.green,
                    label: t('metric.inclinationTitle', 'Inclination'),
                    //cubicInterpolationMode: 'monotone',
                    data: inclination,
                    fill: false,
                    pointRadius: 0,
                    borderWidth: 2,
                },
            ]
        },
        options: {
            animation: {
              onComplete: function() {
                  if(saveScreenshot[6])
                      return;
                  saveScreenshot[6] = true;
                  let el = new MainWSQueueElement({
                      msg: 'savechart',
                      content: {
                          name: 'speedInclination',
                          image: speedInclinationChart.toBase64Image()
                      }
                  }, function(msg) {
                      if (msg.msg === 'R_savechart') {
                          return msg.content;
                      }
                      return null;
                  }, 15000, 3);
                  el.enqueue().catch(function(err) {
                      console.error('Error is ' + err);
                  });
              }
            },
            responsive: true,
            aspectRatio: 1.5,
            grid: {
                zeroLineColor: 'rgba(0,255,0,1)'
            },
            plugins: {
                title:{
                    display:true,
                    text:'Speed and Inclination'
                },
                tooltips: {
                    mode: 'index',
                    intersect: false,
                },
                legend: {
                    display: false,
                    position: 'bottom',
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
                    max: maxEl,
                },
                y: {
                    display: true,
                    title: {
                        display: false,
                    },
                    min: 0,
                    ticks: {
                        stepSize: 5,
                        autoSkip: false,
                        color: 'black',
                        padding: -30,
                        align: 'end',
                        z: 1,
                        callback: function(value, index, values) {
                            return value !== 0 ? value  : "";
                        },
                    }
                }
            }
        }
    };

    ctx = document.getElementById('canvasSpeedInclination').getContext('2d');
    var speedInclinationChart = new Chart(ctx, config);
}

function dochart_init() {
    keys_arr = ['miles_unit', 'age', 'heart_rate_zone1', 'heart_rate_zone2', 'heart_rate_zone3', 'heart_rate_zone4', 'heart_max_override_enable', 'heart_max_override_value']

    function load_workout_data() {
        let el = new MainWSQueueElement({
            msg: 'getsessionarray'
        }, function(msg) {
            if (msg.msg === 'R_getsessionarray') {
                return msg.content;
            }
            return null;
        }, 15000, 3);
        el.enqueue().then(process_arr).catch(function(err) {
            console.error('Error is ' + err);
        });
    }

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
                        heart_max_override_enable = isTrueSetting(msg.content[key]);
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
        load_workout_data();
    }).catch(function(err) {
        console.error('Error is ' + err);
        ensureHeartZones();
        load_workout_data();
    })

    let getPelotonImage = new MainWSQueueElement({
        msg: 'getpelotonimage',
    }, function(msg) {
        if (msg.msg === 'R_getpelotonimage' && msg.content.length > 0) {
            $('.workout_image').attr("src","data:image/png;base64," + msg.content);
        }
        return null;
    }, 15000, 1);
    getPelotonImage.enqueue().catch(function(err) {
        console.error('Error is ' + err);
    });

}


$(window).on('load', function () {
    dochart_init();
});

$(document).ready(function () {
    $('#loading').hide();
});
