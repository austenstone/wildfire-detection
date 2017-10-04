<?php
$startdate = $_GET['startdate'];
$enddate = $_GET['enddate'];

$servername = "73.139.183.89";
$username = "wildfir";
$password = "1102";
$dbname = "wildfire";
$conn = new mysqli($servername, $username, $password, $dbname);
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}
$result = $conn->query("SELECT *
  FROM demo
  WHERE time BETWEEN '$startdate' AND '$enddate'
  ORDER BY time DESC
  LIMIT 1000;");
if($result->num_rows > 0){
  echo "<table id='database'>";
  echo "<thead><th>Time</th><th>Temperature</th><th>Humidity</th><th>PPM</th><th>LPG</th><th>Concentration</th><th>Smoke</th><th>O3</th></thead>";
  while($row = $result->fetch_assoc()) {
    echo "<tr>";
    echo "<td>" . $row["time"] . "</td><td>" . $row["temp"] . "</td>
    <td>" . $row["humidity"] . "</td><td>" . $row["ppm"] . "</td><td>" . $row["lpg"] . "</td>
    <td>" . $row["concentration"] . "</td><td>" . $row["smoke"] . "</td><td>" . $row["othree"] . "</td>";
    echo "</tr>";
  }
  echo "</table>";
} else {
  echo "<p>ZERO RESULTS for " . $startdate . " - " . $enddate . "</p>";
}
$conn->close();
?>
