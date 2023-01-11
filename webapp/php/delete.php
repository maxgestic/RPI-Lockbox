<?php
$myfile = fopen("/home/admin/RPI-Lockbox/server/index", "w") or die($php_errormsg);
$txt = $_POST["index"]."\n";
fwrite($myfile, $txt);
fclose($myfile);
$myfile = fopen("/home/admin/RPI-Lockbox/server/instruction", "w") or die($php_errormsg);
$txt = "delete\n";
fwrite($myfile, $txt);
fclose($myfile);
?>
<h3>Fingerprint Deletion</h3>
<?php
echo "<p>The fingerprint in the index ".$_POST["index"]." has been deleted.</p>"
?>
<div id="center_button">
    <button onclick="location.href='../'">Back to Home</button>
</div>
