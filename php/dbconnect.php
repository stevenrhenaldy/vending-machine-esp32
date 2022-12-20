<?php

$dbms = 'mysql';
$host = 'localhost';
$dbName = 'vending_machine';
$user = 'root';
$password = '';

$dsn = "$dbms:host=$host;dbname=$dbName";


try {
  $conn = new PDO($dsn, $user, $password);
  // set the PDO error mode to exception
  $conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
//   echo "Connected successfully";
} catch(PDOException $e) {
  echo "Connection failed: " . $e->getMessage();
}