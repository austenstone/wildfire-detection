<?php
$startdate = $_GET['startdate'];
$enddate = $_GET['enddate'];
$type = $_GET['type'];

$servername = "149.56.101.13";
$username = "root";
$password = "Ams188623";
$dbname = "wildfire";
$conn = new mysqli($servername, $username, $password, $dbname);
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}
$result = $conn->query("SELECT *
                        FROM wildfire1
                        WHERE time BETWEEN '$startdate' AND '$enddate'
                        ORDER BY time DESC");
if ($result->num_rows > 0) {
    $temp[] = array("time", $type);
    while ($row = $result->fetch_assoc()) {
        $temp[] = array($row["time"], (int)$row[$type]);
    }
    echo json_encode($temp);
} else {
    echo "ZERO" . $type . " RESULTS";
    echo mysqli_connect_error();
}
$conn->close();
