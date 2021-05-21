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



function addValueToTelemetryChart(chartConfig, value, label) {
  chartConfig.data.datasets[0].data.push(value);
  chartConfig.data.labels.push(label);
  console.log(chartConfig);
  payload1TelemetryChart.update(chartConfig);
}

function sendCustomCommand() {
  let inputElem = document.getElementById('custom-command-input');
  sendCommand(inputElem.value);
}

function testAll() {
  parsePacketAndAddValues('2764,00:01:32,10,C,F,N,N,700.2,18.2,8.98,20:54:33,42.30402,34.30402,699.3,3,STARTUP,0,0,CXON');
}

testAll();