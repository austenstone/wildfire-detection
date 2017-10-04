<!--
WILDFIRE SENSOR PROJECT
Author: Austen Stone.
Notes: Fire Icon=&#x1F525;
-->
<html>
<body>
  <head>
    <link rel="shortcut icon" href="http://austenstone.com/EGN/favicon.ico">
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js"></script>
    <script type="text/javascript" src="https://www.gstatic.com/charts/loader.js"></script>
    <script async defer src="https://maps.googleapis.com/maps/api/js?key=AIzaSyAcpxIEx2YrJA2tnVPq5B0VGGT4JZFADeY&callback=initMap" type="text/javascript"></script>
    <link href="https://fonts.googleapis.com/css?family=Lato|Roboto:400,700" rel="stylesheet">
    <script src="javascript.js"></script>
    <link href="style.css" rel="stylesheet">
    <title>Wildfire Sensor Test </title>
  </head>

  <!--Header Bar-->
  <header>
    <wrapper>
      <h1 class="title">Wildfire Detector</h1>
      <h2 class="sub-title">A Wildfire Sensor Project</h2>
    </wrapper>
  </header>

  <!--Content-->
  <content>
    <wrapper>
      <div class="selection-bar">
        <p style="margin-bottom: 0px;">Realtime Data:</p>
        <div id="log"></div>
        <button onclick="socketReconnect()">Reconnect</button>
        <p style="margin-bottom: 0px;">Database:</p>
        <input type="datetime-local" id="start-date" placeholder="2017-07-12T08:30" min="2017-07-12T08:30" id="startdate" name="start">
        <input type="datetime-local" id="end-date" min="2017-07-12T08:30" id="enddate" name="end">
        <select id="type">
          <option value="temp">Temperature</option>
          <option value="humidity">Humidity</option>
          <option value="ppm">PPM</option>
          <option value="lpg">LPG</option>
          <option value="concentration">Concentration</option>
          <option value="smoke">Smoke</option>
          <option value="othree">O3</option>
        </select>
        <button onclick="date()">View Data</button>
        <button onclick="auto()">Auto Mode</button>
        <div id="chart-line"></div>
        <div class="database-wrapper">
          <div class="database">
          </div>
        </div>
        <select name="node">
          <option value="1">Node 1</option>
          <option value="2">Node 2</option>
        </select>
      </div>
      <div class="gauge-box" id="chart-gauge"></div>
      <!--<div id="chart-scatter"></div>-->
      <!--<div class="selection-bar">
        <select name="node">
          <option value="1">Node 1</option>
          <option value="2">Node 2</option>
        </select>
      </div>-->
    </wrapper>
    <!--<div id="map"></div>-->
  </content>

  <!--Footer-->
  <footer>
    <?php include("footer.php") ?>
  </footer>
</body>

</html>
