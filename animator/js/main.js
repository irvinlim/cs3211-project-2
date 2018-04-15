'use strict';

const gpu = new GPU();
let kernel;
let frame = 0;
let timeout;
let images = [];
let framesdir, regions, gridsize, timeslots, framerate;
let isLoading = false;

const $ = selector => document.querySelector(selector);

function initializeGpujs(regions, gridsize) {
    const length = Math.floor(Math.sqrt(regions)) * gridsize;

    kernel = gpu.createKernel(
        function(imageData) {
            var y = 3 * (this.constants.length - this.thread.y - 1) * this.constants.length;
            var x = 3 * this.thread.x;
            var r = imageData[x + y + 0] / 256;
            var g = imageData[x + y + 1] / 256;
            var b = imageData[x + y + 2] / 256;
            this.color(r, g, b, 1);
        },
        {
            graphical: true,
            constants: { length },
            output: [length, length]
        }
    );

    const canvas = kernel.getCanvas();
    canvas.width = gridsize * Math.sqrt(regions);
    canvas.height = gridsize * Math.sqrt(regions);
    $('.canvas-container').appendChild(canvas);
}

function start() {
    framesdir = $('#input-framesdir').value;
    regions = parseInt($('#input-regions').value);
    gridsize = parseInt($('#input-gridsize').value);
    timeslots = parseInt($('#input-timeslots').value);
    framerate = parseInt($('#input-framerate').value);

    console.log('Regions:', regions);
    console.log('GridSize:', gridsize);
    console.log('TimeSlots:', timeslots);

    // Stop kernel first if it is running.
    if (timeout) clearTimeout(timeout);
    frame = 0;

    // Set up kernel and canvas.
    initializeGpujs(regions, gridsize);

    // Preload all images via AJAX.
    console.log('Loading all images...');
    $('.loading-container').style.display = 'block';

    loadImages(function() {
        console.log('Loaded!');
        $('.loading-container').style.display = 'none';
        $('.canvas-container').style.display = 'block';

        // Start the animation.
        console.log(`Animating at ${framerate} fps.`);
        animate();

        function animate() {
            nextFrame(images[frame++]);
            frame %= timeslots;

            timeout = setTimeout(animate, 1000 / framerate);
        }
    });
}

function loadImages(callback) {
    images = [];
    let loaded = 0;

    // Reset progress bar.
    $('progress').innerHTML = '0%';
    document.querySelector('progress').value = 0;

    // Prevent cached XHRs by appending current timestamp to request.
    const date = new Date().getTime();

    for (let i = 0; i < timeslots; i++) {
        const xhr = new XMLHttpRequest();
        xhr.open('GET', `${framesdir}/${i}.ppm?r=${date}`, true);

        xhr.onload = function(e) {
            if (xhr.readyState !== xhr.DONE || xhr.status !== 200) return;

            const lines = xhr.responseText.split(/[\n ]/);
            const imagedata = lines.slice(4, lines.length - 1).map(x => parseInt(x));
            images[i] = imagedata;

            loaded++;

            // Update progress bar.
            const percentLoaded = Math.round(loaded / timeslots * 100);
            $('progress').innerHTML = `${percentLoaded}%`;
            $('progress').value = percentLoaded;

            // Call the callback once completed.
            if (loaded >= timeslots) {
                isLoading = false;
                callback();
            }
        };

        xhr.send();
    }
}

function nextFrame(imageData) {
    kernel(imageData);

    const canvas = kernel.getCanvas();
    canvas.width = gridsize * Math.sqrt(regions);
    canvas.height = gridsize * Math.sqrt(regions);
}

function stop() {
    if (timeout) clearTimeout(timeout);
}

function formHandler(e) {
    e.preventDefault();

    // Prevent double loading.
    if (isLoading) return;

    isLoading = true;
    start();
}

function updateFrameRate() {
    framerate = parseInt($('#input-framerate').value);
}

$('form').addEventListener('submit', formHandler);
$('#btn-stop').addEventListener('click', stop);
$('#input-framerate').addEventListener('change', updateFrameRate);
