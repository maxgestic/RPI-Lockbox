<?php
$myfile = fopen("/home/admin/RPI-Lockbox/server/instruction", "w") or die($php_errormsg);
$txt = "empty\n";
fwrite($myfile, $txt);
fclose($myfile);
?>
<h3>Fingerprint Store Empty</h3>
<?php
echo "<p>The fingerprint store has been emptied.</p>"
?>
<div id="center_button">
    <button onclick="location.href='../'">Back to Home</button>
</div>
