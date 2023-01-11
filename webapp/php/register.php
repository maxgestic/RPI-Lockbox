<?php
$myfile = fopen("/home/admin/RPI-Lockbox/server/index", "w") or die($php_errormsg);
$txt = $_POST["index"]."\n";
fwrite($myfile, $txt);
fclose($myfile);
$myfile = fopen("/home/admin/RPI-Lockbox/server/instruction", "w") or die($php_errormsg);
$txt = "reg\n";
fwrite($myfile, $txt);
fclose($myfile);
?>
<h3>Fingerprint Registration</h3>
<p>When the scanner is pulsing in blue place your finger to register.<br>Then remove the finger when the led turns off and place the finger back on to register when the led is pulsing blue again!</p>
<p>When the fingerprint scanner flashes blue and red twice the finger has been registered. Press the button below to get back to the menu!</p>
<div id="center_button">
    <button onclick="location.href='../'">Back to Home</button>
</div>
