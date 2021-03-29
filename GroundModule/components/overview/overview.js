const csv = require('csv-parser');
const fs = require('fs');
const chartjs = require('chart.js');

const CONTAINER_TELEMETRY_CHART_CONFIG = {
  type: 'line',
  data: {
    labels: ['00:01:02', '00:01:03', '00:01:04', '00:01:05' ,'00:01:06', '00:01:07', '00:01:08', '00:01:09', '00:01:10'],
    datasets: [{
      label: 'Container',
      backgroundColor: 'rgb(92,94,93)', //Gray
      borderColor: 'rgb(92,94,93)', //Gray
      data: [700,
        673,
        640,
        605,
        560,
        520,
        480,
        430,
        380],
      fill: false,
    },
    {
      label: 'Payload 1',
      backgroundColor: 'rgb(255, 99, 132)', //Red
      borderColor: 'rgb(255, 99, 132)', //Red
      data: [null,
        null,
        null,
        null,
        null,
        500,
        450,
        415,
        375],
      fill: false,
    },
    {
      label: 'Payload 2',
      backgroundColor: 'rgb(54, 162, 235)', //Blue
      borderColor: 'rgb(54, 162, 235)', //Blue
      data: [null,
        null,
        null,
        null,
        null,
        null,
        null,
        400,
        350],
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

// const CONTAINER_TELEMETRY_CHART_CONFIG = {
//   type: 'line',
//   data: {
//     labels: ['00:01:02', '00:01:03', '00:01:04', '00:01:05' ,'00:01:06', '00:01:07', '00:01:08', '00:01:09', '00:01:10'],
//     datasets: [{
//       label: 'Container',
//       backgroundColor: 'rgb(92,94,93)', //Gray
//       borderColor: 'rgb(92,94,93)', //Gray
//       data: [15.2,
//         16.2,
//         16.3,
//         16.8,
//         17.5,
//         17.8,
//         18.3,
//         18.4,
//         18.6],
//       fill: false,
//     }]
//   },
//   options: {
//     responsive: true,
//     mantainAspectRation: false,
//     title: {
//       display: true,
//       text: 'Container'
//     },
//     tooltips: {
//       mode: 'index',
//       intersect: false,
//     },
//     hover: {
//       mode: 'nearest',
//       intersect: true
//     },
//     scales: {
//       xAxes: [{
//         display: true,
//         scaleLabel: {
//           display: true,
//           labelString: 'Mission Time (hh/mm/ss)'
//         }
//       }],
//       yAxes: [{
//         display: true,
//         scaleLabel: {
//           display: true,
//           labelString: 'Temerature (°C)'
//         }
//       }]
//     }
//   }
// };

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
    // fs.createReadStream('data/simulation_data.csv')
    // .pipe(csv())
    // .on('data', (row) => {
    //   addValueToContainerTelemetryChart(Number(row.pressure_val));
    // })
    // .on('end', () => {
    //   console.log('CSV file successfully processed');
    // });
  // }

  function addValueToContainerTelemetryChart(value) {
    containerTelemetryChartConfig.data.datasets[0].data.push(value);
    if (!containerTelemetryChartConfig.data.labels.length) {
      containerTelemetryChartConfig.data.labels.push('0');
    } else {
      let lastNum = Number(lastElem(containerTelemetryChartConfig.data.labels));
      containerTelemetryChartConfig.data.labels.push(String(lastNum + 1));
    }
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

function lastElem(array) {
  return array[array.length - 1];
}