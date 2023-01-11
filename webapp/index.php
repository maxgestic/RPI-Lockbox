<html>
<head>
    <title>Lock Box Control</title>
</head>
<body>
<h1>Lock Box Control</h1>
<h2>Register Fingerprint:</h2>
<form action="php/register.php" method="post">
    <label for="index">Index:</label><br>
    <input type="number" id="index" name="index"><br>
    <input type="submit" value="Register Fingerprint">
</form>
<h2>Delete Fingerprint:</h2>
<form action="php/delete.php" method="post">
    <label for="index">Index:</label><br>
    <input type="number" id="index" name="index"><br>
    <input type="submit" value="Delete Fingerprint">
</form>
<h2>Empty Fingerprint Store:</h2>
<form action="php/empty.php" method="post">
    <input type="submit" value="Empty Fingerstore">
</form>
</body>
</html>
