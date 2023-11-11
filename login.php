<?php
include "koneksi/koneksi.php";
// Memeriksa koneksi
if (!$conn) {
    die("Connection failed: " . mysqli_connect_error());
}

// Memeriksa data login
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $username = $_POST["username"];
    $password = $_POST["password"];

    // Mengecek apakah username dan password valid
    $sql = "SELECT * FROM admin WHERE username='$username' AND password='$password'";
    $result = mysqli_query($conn, $sql);

    if (mysqli_num_rows($result) > 0) {
        // Login berhasil
        echo '<script>alert("Berhasil Login!");';
        header("Location: /login/dasboard/index.php");
        exit();
    } else {
        // Login gagal
        echo '<script>alert("Username atau password salah!");';
        echo 'window.location.href = "/login/index.html";</script>';
        return;
    }
}

mysqli_close($conn);
