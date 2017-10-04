var ipAddress = '149.56.101.13'//'192.168.1.9'
var port = '9998'

$(document).ready(function(){
  // Load google charts
  google.charts.load('current', {'packages':['corechart', 'gauge']});
  google.charts.setOnLoadCallback(drawLineChart);
  google.charts.setOnLoadCallback(socketConnection);
  google.charts.setOnLoadCallback(drawGauge);
  var d = new Date()
  var e = new Date()
  e.setHours(e.getHours() + 1)
  d.setMonth(d.getMonth() + 1)
  e.setMonth(e.getMonth() + 1)
  var datenow = d.getFullYear() + "-" + ('0' + d.getMonth()).slice(-2) + "-" + ('0' + d.getDate()).slice(-2)  + "T" +
                ('0' + d.getHours()).slice(-2) + ":" + ('0' + d.getMinutes()).slice(-2) + ":" + ('0' + e.getSeconds()).slice(-2)

  var datenow2 = e.getFullYear() + "-" + ('0' + e.getMonth()).slice(-2) + "-" + ('0' + d.getDate()).slice(-2)  + "T" +
                ('0' + e.getHours()).slice(-2) + ":" + ('0' + e.getMinutes()).slice(-2) + ":" + ('0' + e.getSeconds()).slice(-2)
  document.getElementById("start-date").value = datenow;
  document.getElementById("end-date").value = datenow2;
})

function socketConnection(){
  if(window.WebSocket){
    try{
      var ws = new WebSocket("ws://" + ipAddress + ":" + port + "/echo")
    } catch(e){
      console.log("Websocket connection failed: " + e)
    }

    ws.onopen = function(){
      console.log("Websocket open!")
    }

    ws.onmessage = function(e){
      try{
        var jdata = JSON.parse(e.data)
      }catch(e){
        console.log(e)
        return
      }
      var span = document.createElement('span')
      span.innerHTML = "Time: " + jdata["time"] + " - " +
                       "Temp: " + jdata["temp"] + "  " +
                       "Humidity: " + jdata["humidity"] + "  " +
                       "PPM: " + jdata["ppm"] + "  " +
                       "LPG: " + jdata["lpg"] + "  " +
                       "Concentration: " + jdata["conc"] + "  " +
                       "Smoke: " + jdata["smoke"] + "  " +
                       "O3: " + jdata["o3"] + "  " +
                       "<br>"
      drawGauge(jdata)
      var length = $('#log span').length
      if(length > 4){
        $("#log").children().first().remove()
      }
      document.getElementById('log').appendChild(span)
    }

    ws.onclose = function(){
      console.log("Websocket connection closed.")
    }
  } else {
    alert("Websocket is NOT supported by your browser.")
  }
}

function socketReconnect(){
  console.log("Reconnecting...")
  socketConnection()
}
function auto(){
    setInterval(date, 1000);
}
function date(){
  var start = document.getElementById("start-date").value
  var end = document.getElementById("end-date").value
  var dtype = document.getElementById("type").value
  $.get("functions.php", { startdate: start, enddate: end }, function(data){
    $('.database').html(data)
  });
  $.get("query.php", { startdate: start, enddate: end, type: dtype }, function(data){
    try{
      var json = JSON.parse(data)
      drawLineChart(json)
    }catch(error){
      console.log(error)
    }
  });
}

function initMap() {
  var wellington = {lat: 26.37, lng: -80.10};
  var map = new google.maps.Map(document.getElementById('map'), {
    zoom: 14,
    center: wellington,
    scrollwheel: false
  });
  var marker = new google.maps.Marker({
    position: wellington,
    map: map
  });
}

// Load google charts
google.charts.load('current', {'packages':['corechart', 'gauge', 'corechart']});
google.charts.setOnLoadCallback(drawLineChart);
google.charts.setOnLoadCallback(drawGauge);

//Line Chart
function drawLineChart(json) {
  if( json ){
    var data = new google.visualization.DataTable();
    data.addColumn('datetime', 'time');
    data.addColumn('number', json[0][1]);
    for(var i = 1; i<json.length; i++){
      var datetime = new Date(json[i][0])
      var temp = json[i][1]
      data.addRow([new Date(datetime.getTime()), json[i][1]])
    }
    var options = {
      title: json[0][1],
      curveType: 'function',
      legend: { position: 'bottom' },
    };

    var chart = new google.visualization.LineChart(document.getElementById('chart-line'));

    chart.draw(data, options);
  }
}

var gchart
//Gauge
function drawGauge(data) {
  if(data == undefined){
    gchart = new google.visualization.Gauge(document.getElementById('chart-gauge'));
    var data = google.visualization.arrayToDataTable([
      ['Label', 'Value'],
      ['Temp', 0],
      ['Humidity', 0],
      ['PPM', 0],
      ['LPG', 0],
      ['Gas Concentration', 0],
      ['Smoke', 0],
      ['O3', 0]
    ]);
  }else{
    var data = google.visualization.arrayToDataTable([
      ['Label', 'Value'],
      ['Temp', data["temp"]],
      ['Humidity', data["humidity"]],
      ['PPM', data["ppm"]],
      ['Gas LPG', data["lpg"]],
      ['Gas Conc', data["conc"]],
      ['Smoke', data["smoke"]],
      ['O3', data["o3"]]
    ]);
  }

  var options = {
    width: 800, height: 120,
    redFrom: 90, redTo: 100,
    yellowFrom:75, yellowTo: 90,
    minorTicks: 5,
    animation:{
      duration: 1000,
      easing: 'inAndOut'
    }
  };

  gchart.draw(data, options);
}
