var miles = 0.621371;

function process_arr(samples) {
    const heart = [];
    const speed = [];
    const inclination = [];
    let elapsed = 0;
    for (const sample of samples) {
        elapsed = sample.elapsed_s + sample.elapsed_m * 60 + sample.elapsed_h * 3600;
        heart.push({x: elapsed, y: sample.heart});
        speed.push({x: elapsed, y: sample.speed * miles});
        inclination.push({x: elapsed, y: sample.inclination});
    }
    const last = samples.length ? samples[samples.length - 1] : {};
    $('.workoutName').text(last.workoutName || 'X22i workout');
    $('.workoutStartDate').text(last.workoutStartDate || '');
    $('.summary_distance').text(((last.distance || 0) * miles).toFixed(1) + (miles === 1 ? ' km' : ' mi'));

    const backgroundFill = {
        id: 'custom_canvas_background_color',
        beforeDraw: function(chart) {
            const ctx = chart.canvas.getContext('2d');
            ctx.save();
            ctx.globalCompositeOperation = 'destination-over';
            ctx.fillStyle = 'white';
            ctx.fillRect(0, 0, chart.width, chart.height);
            ctx.restore();
        }
    };
    function renderChart(canvas, title, datasets, name) {
        let saved = false;
        new Chart(document.getElementById(canvas).getContext('2d'), {
            type: 'line',
            plugins: [backgroundFill],
            data: {datasets: datasets},
            options: {
                responsive: true,
                aspectRatio: 1.5,
                animation: {
                    onComplete: function(animation) {
                        if (saved) return;
                        saved = true;
                        const request = new MainWSQueueElement({
                            msg: 'savechart',
                            content: {name: name, image: animation.chart.toBase64Image()}
                        }, function(msg) {
                            return msg.msg === 'R_savechart' ? msg.content : null;
                        }, 15000, 3);
                        request.enqueue().catch(function(error) { console.error(error); });
                    }
                },
                plugins: {
                    title: {display: true, text: title},
                    legend: {display: datasets.length > 1}
                },
                elements: {point: {radius: 0}, line: {borderWidth: 2, fill: false}},
                scales: {
                    x: {
                        type: 'linear',
                        min: 0,
                        max: Math.max(1, elapsed),
                        ticks: {callback: function(value) {
                            return Math.floor(value / 60) + ':' + String(Math.floor(value % 60)).padStart(2, '0');
                        }}
                    },
                    y: {beginAtZero: true}
                }
            }
        });
    }
    renderChart('canvasHeart', 'Heart Rate', [
        {label: 'Heart rate (bpm)', data: heart, borderColor: 'rgb(255, 29, 0)'}
    ], 'heart');
    renderChart('canvasSpeedInclination', 'Speed and Incline', [
        {label: miles === 1 ? 'Speed (km/h)' : 'Speed (mph)', data: speed, borderColor: 'rgb(54, 162, 235)'},
        {label: 'Incline (%)', data: inclination, borderColor: 'rgb(75, 192, 192)'}
    ], 'speedInclination');
}

function dochart_init() {
    function loadWorkout() {
        const request = new MainWSQueueElement({msg: 'getsessionarray'}, function(msg) {
            return msg.msg === 'R_getsessionarray' ? msg.content : null;
        }, 15000, 3);
        request.enqueue().then(process_arr).catch(function(error) { console.error(error); });
    }
    const settings = new MainWSQueueElement({
        msg: 'getsettings', content: {keys: ['miles_unit']}
    }, function(msg) {
        if (msg.msg !== 'R_getsettings') return null;
        miles = msg.content.miles_unit === false || msg.content.miles_unit === 'false' ? 1 : 0.621371;
        return msg.content;
    }, 5000, 3);
    settings.enqueue().then(loadWorkout).catch(function(error) {
        console.error(error);
        loadWorkout();
    });
}

$(window).on('load', dochart_init);
$(document).ready(function() { $('#loading').hide(); });
