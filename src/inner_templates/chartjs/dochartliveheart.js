var heartChart = null;

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

function process_arr_heart(samples) {
    const canvas = [document.getElementById('canvasheart'), document.getElementById('canvasheartFull')]
        .find(function(item) { return item && item.offsetParent !== null; }) || document.getElementById('canvasheart');
    const heart = [];
    let elapsed = 0;
    let maximum = 100;
    for (const sample of samples) {
        elapsed = sample.elapsed_s + sample.elapsed_m * 60 + sample.elapsed_h * 3600;
        heart.push({x: elapsed, y: sample.heart});
        maximum = Math.max(maximum, sample.heart + 5);
    }
    heartChart = new Chart(canvas.getContext('2d'), {
        type: 'line',
        data: {datasets: [{label: 'Heart rate (bpm)', data: heart, borderColor: 'rgb(255, 29, 0)', pointRadius: 0, borderWidth: 2}]},
        options: {
            responsive: true,
            maintainAspectRatio: false,
            animation: false,
            plugins: {title: {display: true, text: 'Heart Rate'}, legend: {display: false}},
            scales: {
                x: {type: 'linear', min: 0, max: Math.max(1, elapsed)},
                y: {min: 0, max: maximum}
            }
        }
    });
    if (typeof setNormalXScale === 'function') {
        setNormalXScale('heart', heartChart.options.scales.x.min, heartChart.options.scales.x.max);
    }
    refresh_heart();
}

function refresh_heart() {
    let el = new MainWSQueueElement({
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

$(window).on('load', load_heart_workout_data);
$(document).ready(function() { $('#loading').hide(); });
