const csv = require('csv-parser');
const fs = require('fs');
const chartjs = require('chart.js');

const CONTAINER_TELEMETRY_CHART_CONFIG = {
  type: 'line',
  data: {
    labels: [],
    datasets: [{
      label: 'Altitude',
      backgroundColor: 'rgb(255, 99, 132)', //Red
      borderColor: 'rgb(255, 99, 132)', //Red
      data: [],
      fill: false,
    }]
  },
  options: {
    responsive: true,
    mantainAspectRation: false,
    title: {
      display: true,
      text: 'Evolution'
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
          labelString: 'Mission Time (s)'
        }
      }],
      yAxes: [{
        display: true,
        scaleLabel: {
          display: true,
          labelString: 'Altitude (s)'
        }
      }]
    }
  }
};

// class OverviewTelemetryCharts {
  var containerTelemetryChart;
  var containerTelemetryChartConfig = CONTAINER_TELEMETRY_CHART_CONFIG;

  var payload1TelemetryChart;
  var payload1TelemetryChartConfig = CONTAINER_TELEMETRY_CHART_CONFIG;

  var payload2TelemetryChart;
  var payload2TelemetryChartConfig = CONTAINER_TELEMETRY_CHART_CONFIG;

  // constructor() {
    containerTelemetryCanvasCtx = document.getElementById('container-telemetry-canvas').getContext('2d');
    containerTelemetryChart = new chartjs.Chart(containerTelemetryCanvasCtx, containerTelemetryChartConfig);
  
    payload1TelemetryCanvasCtx = document.getElementById('payload-1-telemetry-canvas').getContext('2d');
    payload1TelemetryChart = new chartjs.Chart(payload1TelemetryCanvasCtx, payload1TelemetryChartConfig);
  
    payload2TelemetryCanvasCtx = document.getElementById('payload-2-telemetry-canvas').getContext('2d');
    payload2TelemetryChart = new chartjs.Chart(payload2TelemetryCanvasCtx, payload2TelemetryChartConfig);
    console.log(containerTelemetryChartConfig.data.datasets[0]);
    fs.createReadStream('data/simulation_data.csv')
    .pipe(csv())
    .on('data', (row) => {
      addValueToContainerTelemetryChart(Number(row.pressure_val));
    })
    .on('end', () => {
      console.log('CSV file successfully processed');
    });
  // }

  function addValueToContainerTelemetryChart(value) {
    console.log(containerTelemetryChartConfig.data.datasets[0]);
    containerTelemetryChartConfig.data.datasets[0].data.push(value);
    console.log(value);
    console.log(containerTelemetryChartConfig.data.datasets[0].data);

  }
// }


// window.onload = function(){
  // let telemetryCharts = new OverviewTelemetryCharts();
// };


// const createCsvWriter = require('csv-writer').createObjectCsvWriter;
// const csvWriter = createCsvWriter({
//   path: 'out.csv',
//   header: [
//     {id: 'name', title: 'Name'},
//     {id: 'surname', title: 'Surname'},
//     {id: 'age', title: 'Age'},
//     {id: 'gender', title: 'Gender'},
//   ]
// });

// const data = [
//   {
//     name: 'John',
//     surname: 'Snow',
//     age: 26,
//     gender: 'M'
//   }, {
//     name: 'Clair',
//     surname: 'White',
//     age: 33,
//     gender: 'F',
//   }, {
//     name: 'Fancy',
//     surname: 'Brown',
//     age: 78,
//     gender: 'F'
//   }
// ];

// csvWriter
//   .writeRecords(data)
//   .then(()=> console.log('The CSV file was written successfully'));