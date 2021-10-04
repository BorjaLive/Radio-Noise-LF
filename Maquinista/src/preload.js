var Chart = require('chart.js');
const SerialPort = require('serialport');

const N_SENSORS = 8;
const ESTADOS = { DETENIDO: 1, SIGUIENDO: 2, PERDIDO: 3 };
const MAX_LOST_LINE_TIME = 30;
const MAX_TURN_FORCE = 2;
const CONFIG_PRESETS = [{
        name: "Lento",
        s: 100,
        p: 1,
        i: 0,
        d: 0
    },
    {
        name: "Rapido",
        s: 255,
        p: 1,
        i: .75,
        d: .5
    }
];

var port = null,
    sercom = null;
SerialPort.list().then((ports) => {
    ports.forEach((p) => {
        pm = p["manufacturer"];
        if (typeof pm !== "undefined" && pm.includes("arduino")) {
            port = new SerialPort(p.path, { baudRate: 9600 });
            port.on("open", () => {
                console.log('Serial port open');
            });
            sercom = new RNLFSerCom(port);
        }
    });
});
var estado = ESTADOS.DETENIDO;

window.addEventListener("DOMContentLoaded", () => {
    var sensorsDiv = document.getElementById("sensorsDiv");

    var sensorBalls = [];
    for (let i = 0; i < N_SENSORS; i++) {
        let ball = document.createElement("div");
        ball.classList.add("sensor-ball");
        sensorsDiv.appendChild(ball);
        sensorBalls.push(ball);
    }

    var velocidadSlider = document.getElementById("velocidadSlider");
    var velocidadInput = document.getElementById("velocidadInput");
    var pidPSlider = document.getElementById("pidPSlider");
    var pidPInput = document.getElementById("pidPInput");
    var pidISlider = document.getElementById("pidISlider");
    var pidIInput = document.getElementById("pidIInput");
    var pidDSlider = document.getElementById("pidDSlider");
    var pidDInput = document.getElementById("pidDInput");

    var pid = new PIDcontroller(0, 0)
    let updatePIDsettings = () => {
        //TODO: Agregar la deadband ya si eso
        pid.tune(pidPInput.value, pidIInput.value, pidDInput.value, 0);
    };
    velocidadSlider.addEventListener("change", () => {
        velocidadInput.value = Math.floor(velocidadSlider.value * 1000) / 1000;
        updatePIDsettings();
    });
    pidPSlider.addEventListener("change", () => {
        pidPInput.value = Math.floor(pidPSlider.value * 1000) / 1000;
        updatePIDsettings();
    });
    pidISlider.addEventListener("change", () => {
        pidIInput.value = Math.floor(pidISlider.value * 1000) / 1000;
        updatePIDsettings();
    });
    pidDSlider.addEventListener("change", () => {
        pidDInput.value = Math.floor(pidDSlider.value * 1000) / 1000;
        updatePIDsettings();
    });
    velocidadInput.addEventListener("keypress", (event) => {
        if (event.code == "Enter") velocidadSlider.value = velocidadInput.value;
        updatePIDsettings();
    });
    pidPInput.addEventListener("keypress", (event) => {
        if (event.code == "Enter") pidPSlider.value = pidPInput.value;
        updatePIDsettings();
    });
    pidIInput.addEventListener("keypress", (event) => {
        if (event.code == "Enter") pidISlider.value = pidIInput.value;
        updatePIDsettings();
    });
    pidDInput.addEventListener("keypress", (event) => {
        if (event.code == "Enter") pidDSlider.value = pidDInput.value;
        updatePIDsettings();
    });

    var presetsDiv = document.getElementById("presetsDiv");
    CONFIG_PRESETS.forEach(conf => {
        let btn = document.createElement("button");
        btn.type = "button";
        btn.classList.add("presetButton");
        btn.innerText = conf.name;
        btn.addEventListener("click", () => {
            velocidadSlider.value = conf.v;
            pidPSlider.value = conf.p;
            pidISlider.value = conf.i;
            pidDSlider.value = conf.d;
            velocidadSlider.dispatchEvent(new Event("change"));
            pidPSlider.dispatchEvent(new Event("change"));
            pidISlider.dispatchEvent(new Event("change"));
            pidDSlider.dispatchEvent(new Event("change"));
        });
        presetsDiv.appendChild(btn);
    });

    var errorPlot = new Chart(document.getElementById("errorPlot"), {
        type: 'line',
        data: {
            datasets: [{
                label: "Error",
                // backgroundColor: 'rgb(255, 99, 132)',  // fill color
                borderColor: 'rgb(255, 99, 132)',
                borderWidth: 2,
                pointRadius: 0
            }]
        },
        options: {
            scales: {
                yAxes: [{
                    ticks: {
                        min: 0,
                        max: 4095,
                        stepSize: 512
                    }
                }],
                xAxes: [{
                    type: 'time',
                    time: {
                        unit: 'second'
                    },
                    ticks: {
                        maxRotation: 0
                    },
                }]
            },
            animation: {
                duration: 0
            }
        }
    });

    function errorAddDot(label, data) {
        errorPlot.data.labels.push(label);
        errorPlot.data.datasets.forEach((dataset) => {
            dataset.data.push(data);
        });
    }

    function errorRemoveDot() {
        errorPlot.data.labels.shift();
        errorPlot.data.datasets.forEach((dataset) => {
            dataset.data.shift();
        });
    }

    const numPoints = 150;
    var pointsCnt = 0;

    function errorPlotUpdate(val) {
        pointsCnt++;
        if (pointsCnt > numPoints) {
            pointsCnt = numPoints + 1;
            errorRemoveDot();
        }
        errorAddDot("", val);
        errorPlot.update();
    };

    var centerMarker = document.getElementById("centerMarker");
    var steerMarker = document.getElementById("steerMarker");

    function setCenterMarkerPos(pos) {
        centerMarker.style.marginLeft = (50 * (pos + 1)) + "%"
    }

    function setSteerMarkerPos(pos) {
        steerMarker.style.marginLeft = (25 * (pos + 2)) + "%"
    }

    setCenterMarkerPos(0);
    setSteerMarkerPos(0);

    var statusDiv = document.getElementById("statusDiv");

    function updateEstado(nuevo) {
        estado = nuevo;
        switch (estado) {
            case ESTADOS.DETENIDO:
                statusDiv.children.item(0).style.color = "red";
                statusDiv.children.item(1).style.color = "gray";
                statusDiv.children.item(2).style.color = "gray";
                break;
            case ESTADOS.SIGUIENDO:
                statusDiv.children.item(0).style.color = "gray";
                statusDiv.children.item(1).style.color = "lime";
                statusDiv.children.item(2).style.color = "gray";
                break;
            case ESTADOS.PERDIDO:
                statusDiv.children.item(0).style.color = "gray";
                statusDiv.children.item(1).style.color = "gray";
                statusDiv.children.item(2).style.color = "yellow";
                break;
        }
    }
    updateEstado(ESTADOS.DETENIDO);

    document.getElementById("estopButton").addEventListener("click", () => updateEstado(ESTADOS.DETENIDO));
    document.getElementById("startButton").addEventListener("click", () => updateEstado(ESTADOS.SIGUIENDO));

    var lostLineCounter = 0;
    pid.setinteg(0);
    pid.bumpless();
    var sensorFilter = new AvgFilter(10, 0);
    //Funcion de actualizacion (30fps)
    setInterval(() => {
        if (sercom === null) return;
        if (sercom.isNewData()) {
            //console.log("new Data", sercom.getData());
            let sensors = sercom.getData();
            let nStart = -1,
                nEnd = -1;
            sensors.forEach((val, i) => {
                sensorBalls[i].style.backgroundColor = val ? "gray" : "white";
                if (val) {
                    if (nStart == -1) nStart = i;
                    nEnd = i;
                }
            });
            let center = 0;
            if (nStart == -1) {
                //TODO: Se ha perdido la linea
                lostLineCounter++;
            } else if (nEnd - nStart > 5) {
                //TODO: Definir la constante de maxima cobertra
                //TODO: La linea no esta lo suficientemente perpendicular
                lostLineCounter++;
            } else {
                center = ((((nStart + nEnd) / 2) / (N_SENSORS - 1)) * 2) - 1;
                lostLineCounter = 0;
            }
            sensorFilter.add(center);
            center = sensorFilter.get();
            setCenterMarkerPos(center);
            errorPlotUpdate(center);

            if (estado == ESTADOS.DETENIDO) {
                sercom.sendData(0, true, 0, true);
                return;
            }

            if (lostLineCounter > MAX_LOST_LINE_TIME) {
                updateEstado(ESTADOS.PERDIDO);
            } else if (estado == ESTADOS.PERDIDO) {
                updateEstado(ESTADOS.SIGUIENDO);
            }

            let baseSpeed, steer;
            switch (estado) {
                case ESTADOS.SIGUIENDO:
                    baseSpeed = velocidadInput.value;
                    pid.update(center);
                    steer = pid.calc();
                    break;
                case ESTADOS.DETENIDO:
                case ESTADOS.PERDIDO:
                    baseSpeed = steer = 0;
                    break;
            }
            setSteerMarkerPos(steer);

            //Conducir
            let pwmI = baseSpeed * (steer + 1),
                pwmD = baseSpeed * (1 - steer),
                dirI = true,
                dirD = true;

            if (pwmI < 0) {
                pwmI = -pwmI;
                dirI = false;
            }

            if (pwmD < 0) {
                pwmD = -pwmD;
                dirD = false;
            }
            sercom.sendData(pwmI, dirI, pwmD, dirD);
            //console.log(steer, pwmI, dirI, pwmD, dirD);
        }
    }, 1000 / 30);
});


function AvgFilter(n, def = 0) {
    this.filter = Array(n);
    for (let i = 0; i < n; i++) this.filter[i] = def;

    this.add = (val) => {
        this.filter.shift();
        this.filter.push(val);
    }

    this.get = () => {
        let avg = 0;
        this.filter.forEach(val => avg += val);
        return avg / this.filter.length;
    }
}


function RNLFSerCom(serial) {
    this.serial = serial;
    this.dataIn = new Uint8Array(4);
    this.dataOut = new Uint8Array(6);
    this.inPos = 0;
    this.newData = false;

    this.dataOut[0] = 0xF0;
    this.dataOut[5] = 0x0F;

    this.serial.on("data", data => {
        data.forEach(b => {
            switch (this.inPos) {
                case 0:
                    if (b != 0xF0) this.inPos = 0;
                    else this.inPos++;
                    break;
                case 5:
                    let crc = Math.floor((this.dataIn[0] + this.dataIn[1] + this.dataIn[2] + this.dataIn[3]) / 4);
                    if (b != crc) this.inPos = 0;
                    else this.inPos++;
                    break;
                case 6:
                    if (b == 0x0F) this.newData = true;
                    this.inPos = 0;
                    break;
                default:
                    this.dataIn[this.inPos - 1] = b;
                    this.inPos++;
            }
        });
    });

    this.isNewData = () => { return this.newData; }

    this.getData = () => {
        let values = Array(N_SENSORS);
        for (let i = 0; i < Math.floor(N_SENSORS / 8); i++) {
            for (let j = 0; j < 8; j++) {
                values[8 * i + j] = bitRead(this.dataIn[3 - i], j);
            }
        }
        return values;
    }

    this.sendData = (pwmI, dirI, pwmD, dirD) => {
        this.dataOut[1] = Math.floor(normaliceValue(pwmI, 0, 248));
        this.dataOut[2] = Math.floor(normaliceValue(pwmD, 0, 248));
        this.dataOut[3] = 0;
        this.dataOut[3] = bitWrite(this.dataOut[3], 0, dirI);
        this.dataOut[3] = bitWrite(this.dataOut[3], 1, dirD);
        this.dataOut[4] = (this.dataOut[1] + this.dataOut[2] + this.dataOut[3]) / 3;
        this.serial.write(this.dataOut);
    }
}

function bitWrite(x, n, value) {
    if (value)
        x |= (1 << n);
    else
        x &= ~(1 << n);
    return x;
}

function bitRead(x, n) {
    return (x & (1 << n)) ? 1 : 0;
}

function PIDcontroller(pv, sp) {
    this.pv = pv; //Steer (-2 a 2)
    this.sp = sp; //Desired center location (-1 a 1)
    this.integral;
    this.pgain;
    this.igain;
    this.dgain;
    this.deadband;
    this.last_error;

    this.tune = (p_gain, i_gain, d_gain, dead_band) => {
        this.pgain = p_gain;
        this.igain = i_gain;
        this.dgain = d_gain;
        this.deadband = dead_band;
        this.integral = 0.0;
        this.last_error = 0;
    }

    this.setinteg = (new_integ) => {
        this.integral = new_integ;
        this.last_error = 0;
    }

    this.update = (sp) => {
        this.sp = sp;
    }

    this.bumpless = () => {
        this.last_error = this.sp - this.pv;
    }

    this.calc = () => {
        let err, pterm, dterm, result;
        err = this.sp - this.pv;
        if (Math.abs(err) > this.deadband) {
            pterm = this.pgain * err;
            if (pterm > 100 || pterm < -100)
                this.integral = 0.0;
            else {
                this.integral += this.igain * err;
                this.integral = normaliceValue(this.integral, -MAX_TURN_FORCE, MAX_TURN_FORCE);
            }
            dterm = (err - this.last_error) * this.dgain;
            result = pterm + this.integral + dterm;
        } else
            result = this.integral;
        this.last_error = err;
        return normaliceValue(result, -MAX_TURN_FORCE, MAX_TURN_FORCE);
    }
}

function normaliceValue(val, min, max) {
    if (val < min)
        return min;
    else if (val > max)
        return max;
    else
        return val;
}