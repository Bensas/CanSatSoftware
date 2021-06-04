const chartjs = require('chart.js');

//'rgb(92,94,93)', //Gray
//'rgb(255, 99, 132)', //Red
//'rgb(54, 162, 235)', //Blue

const containerTelemetryChartConfig = {
  type: 'line',
  data: {
    labels: ['00:00:00'],
    datasets: [{
      label: 'Altitude',
      backgroundColor: 'rgb(255, 99, 132)', //Red
      borderColor: 'rgb(255, 99, 132)', //Red
      data: [0],
      fill: false,
    }]
  },
  options: {
    responsive: true,
    mantainAspectRation: false,
    title: {
      display: true,
      text: 'Container'
    },
    tooltips: {
      mode: 'index',
      intersect: false,
    },
    hover: {
      mode: 'nearest',
      intersect: true
    },
    scales: {
      xAxes: [{
        display: true,
        scaleLabel: {
          display: true,
          labelString: 'Mission Time (hh/mm/ss)'
        }
      }],
      yAxes: [{
        display: true,
        scaleLabel: {
          display: true,
          labelString: 'Altitude (m)'
        }
      }]
    }
  }
};

const payload1TelemetryChartConfig = {
  type: 'line',
  data: {
    labels: ['00:00:00'],
    datasets: [{
      label: 'Altitude',
      backgroundColor: 'rgb(255, 99, 132)', //Red
      borderColor: 'rgb(255, 99, 132)', //Red
      data: [0],
      fill: false,
    }]
  },
  options: {
    responsive: true,
    mantainAspectRation: false,
    title: {
      display: true,
      text: 'Container'
    },
    tooltips: {
      mode: 'index',
      intersect: false,
    },
    hover: {
      mode: 'nearest',
      intersect: true
    },
    scales: {
      xAxes: [{
        display: true,
        scaleLabel: {
          display: true,
          labelString: 'Mission Time (hh/mm/ss)'
        }
      }],
      yAxes: [{
        display: true,
        scaleLabel: {
          display: true,
          labelString: 'Altitude (m)'
        }
      }]
    }
  }
};

const payload2TelemetryChartConfig = {
  type: 'line',
  data: {
    labels: ['00:00:00'],
    datasets: [{
      label: 'Altitude',
      backgroundColor: 'rgb(255, 99, 132)', //Red
      borderColor: 'rgb(255, 99, 132)', //Red
      data: [0],
      fill: false,
    }]
  },
  options: {
    responsive: true,
    mantainAspectRation: false,
    title: {
      display: true,
      text: 'Container'
    },
    tooltips: {
      mode: 'index',
      intersect: false,
    },
    hover: {
      mode: 'nearest',
      intersect: true
    },
    scales: {
      xAxes: [{
        display: true,
        scaleLabel: {
          display: true,
          labelString: 'Mission Time (hh/mm/ss)'
        }
      }],
      yAxes: [{
        display: true,
        scaleLabel: {
          display: true,
          labelString: 'Altitude (m)'
        }
      }]
    }
  }
};


containerTelemetryCanvasCtx = document.getElementById('container-telemetry-canvas').getContext('2d');
containerTelemetryChart = new chartjs.Chart(containerTelemetryCanvasCtx, containerTelemetryChartConfig);

payload1TelemetryCanvasCtx = document.getElementById('payload-1-telemetry-canvas').getContext('2d');
payload1TelemetryChart = new chartjs.Chart(payload1TelemetryCanvasCtx, payload1TelemetryChartConfig);

payload2TelemetryCanvasCtx = document.getElementById('payload-2-telemetry-canvas').getContext('2d');
payload2TelemetryChart = new chartjs.Chart(payload2TelemetryCanvasCtx, payload2TelemetryChartConfig);



function addValueToTelemetryChart(chart, value, label) {
  chart.data.datasets[0].data.push(value);
  chart.data.labels.push(label);
  if (chart.data.labels.length > 20) {
    chart.data.datasets[0].data.shift();
    chart.data.labels.shift();
  }
  console.log(chart);
  chart.update();
}

function sendCustomCommand() {
  let inputElem = document.getElementById('custom-command-input');
  sendCommand(inputElem.value);
}

function testAll() {
  parsePacketAndAddValues('2764,00:01:32,10,C,F,N,N,700.2,18.2,8.98,20:54:33,42.30402,34.30402,699.3,3,STARTUP,42,43,CXON');
  parsePacketAndAddValues('2764,00:00:00,0000,S1,   0.7,24.9,0000');
  parsePacketAndAddValues('2764,00:00:00,0588,S2,  12.7,61.8,0000');
}

//testAll();

function setCurrentTemperature(id, temp) {
  let elem = document.getElementById(id);
  elem.innerHTML = 'Current temperature: ' + temp;
}

function setCurrentBatteryVoltage(id, volts) {
  let elem = document.getElementById(id);
  elem.innerHTML = 'Current battery voltage: ' + volts;
}

function setCurrentGPSCoords(id, lat, lng) {
  let elem = document.getElementById(id);
  elem.innerHTML = 'Current GPS coordinates: ' + lat + ', ' + lng;
}

function setCurrentRotationRate(id, rate) {
  let elem = document.getElementById(id);
  elem.innerHTML = 'Current rotation rate: ' + rate;
}

