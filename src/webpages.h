// linha17: <button onclick="showUploadButtonFancy('/')">Upload File</button>
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
</head>
<body>
  <p>Main page</p>
  <p>Firmware: %FIRMWARE%</p>
  <p>Free Storage: <span id="freeSD">%FREESD%</span> | Used Storage: <span id="usedSD">%USEDSD%</span> | Total Storage: <span id="totalSD">%TOTALSD%</span></p>
  <p>
  <form id="save" enctype="multipart/form-data" method="post"><input type="hidden" id="actualFolder" name="actualFolder" value="/"></form>
  <button onclick="logoutButton()">Logout</button>
  <button onclick="rebootButton()">Reboot</button>
  <button onclick="WifiConfig()">Configure my WiFi</button>
  <button onclick="listFilesButton('/')">List Files</button>

  </p>
  <p id="detailsheader"></p>
  <p id="status"></p>
  <p id="details"></p>
  <p id="updetailsheader"></p>
  <p id="updetails"></p>
<script>

function WifiConfig() {
  let wifiSsid = prompt("Please enter the SSID of your network", "SSID");
  let wifiPwd = prompt("Please enter the Password of your network", "Password");
  if (wifiSsid == null || wifiSsid == "" || wifiPwd == null) {
    window.alert("Invalid SSID of Password");
  } else {
    xmlhttp=new XMLHttpRequest();
    xmlhttp.open("GET", "/wifi?ssid=" + wifiSsid + "&pwd=" + wifiPwd, false);
    xmlhttp.send();    
    document.getElementById("status").innerHTML = xmlhttp.responseText;
  }
}


function logoutButton() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/logout", true);
  xhr.send();
  setTimeout(function(){ window.open("/logged-out","_self"); }, 1000);
}

function rebootButton() {
  document.getElementById("statuSDetails").innerHTML = "Invoking Reboot ...";
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/reboot", true);
  xhr.send();
  window.open("/reboot","_self");
}
function listFilesButton(folders) {
  xmlhttp=new XMLHttpRequest();
  document.getElementById("actualFolder").value = "";
  document.getElementById("actualFolder").value = folders;
  xmlhttp.open("GET", "/listfiles?folder=" + folders, false);
  xmlhttp.send();
  document.getElementById("detailsheader").innerHTML = "<h3>Files<h3>";
  document.getElementById("details").innerHTML = xmlhttp.responseText;
  document.getElementById("updetailsheader").innerHTML = "<h3>Folder Actions<h3>"
  document.getElementById("updetails").innerHTML = "<button onclick=\"showUploadButtonFancy('" + folders + "')\">Upload File</button><button onclick=\"showCreateFolder('" + folders + "')\">Create Folder</button>";
}
function downloadDeleteButton(filename, action) {
  var urltocall = "/file?name=" + filename + "&action=" + action;
  var actualFolder = document.getElementById("actualFolder").value
  xmlhttp=new XMLHttpRequest();
  if (action == "delete" || action=="create") {
    xmlhttp.open("GET", urltocall, false);
    xmlhttp.send();
    document.getElementById("status").innerHTML = xmlhttp.responseText;
    listFilesButton(actualFolder);
  }
  if (action == "download") {
    document.getElementById("status").innerHTML = "";
    window.open(urltocall,"_blank");
  }
}
function showCreateFolder(folders) {
  document.getElementById("updetailsheader").innerHTML = "<h3>Create new Folder<h3>"
  document.getElementById("status").innerHTML = "";
  var uploadform =
  "<p>Creating folder at: <b>" + folders + "</b>"+
  "<form id=\"create_form\" enctype=\"multipart/form-data\" method=\"post\">" +
  "<input type=\"hidden\" id=\"folder\" name=\"folder\" value=\"" + folders + "\">" + 
  "<input type=\"text\" name=\"foldername\" id=\"foldername\">" +
  "<button onclick=\"CreateFolder()\">Create Folder</button>" +
  "</form></p>";
  document.getElementById("updetails").innerHTML = uploadform;
}

function CreateFolder() {
  var folderName = ""; 
  folderName = document.getElementById("folder").value + "/" + document.getElementById("foldername").value;
  downloadDeleteButton(folderName, 'create');
}

function showUploadButtonFancy(folders) {
  document.getElementById("updetailsheader").innerHTML = "<h3>Upload File<h3>"
  document.getElementById("status").innerHTML = "";
  var uploadform =
  "<p>Send file to " + folders + "</p>"+
  "<form id=\"upload_form\" enctype=\"multipart/form-data\" method=\"post\">" +
  "<input type=\"hidden\" id=\"folder\" name=\"folder\" value=\"" + folders + "\">" + 
  "<input type=\"file\" name=\"file1\" id=\"file1\" onchange=\"uploadFile('" + folders + "')\"><br>" +
  "<progress id=\"progressBar\" value=\"0\" max=\"100\" style=\"width:300px;\"></progress>" +
  "<h3 id=\"status\"></h3>" +
  "<p id=\"loaded_n_total\"></p>" +
  "</form>";
  document.getElementById("updetails").innerHTML = uploadform;
}
function _(el) {
  return document.getElementById(el);
}
function uploadFile(folder) {
  var file = _("file1").files[0];
  var folder = _("folder").value;
  // alert(file.name+" | "+file.size+" | "+file.type);
  var formdata = new FormData();
  formdata.append("file1", file);
  formdata.append("folder", folder);
  var ajax = new XMLHttpRequest();
  ajax.upload.addEventListener("progress", progressHandler, false);
  ajax.addEventListener("load", completeHandler, false); // doesnt appear to ever get called even upon success
  ajax.addEventListener("error", errorHandler, false);
  ajax.addEventListener("abort", abortHandler, false);
  ajax.open("POST", "/");
  ajax.send(formdata);
}
function progressHandler(event) {
  //_("loaded_n_total").innerHTML = "Uploaded " + event.loaded + " bytes of " + event.total; // event.total doesnt show accurate total file size
  _("loaded_n_total").innerHTML = "Uploaded " + event.loaded + " bytes";
  var percent = (event.loaded / event.total) * 100;
  _("progressBar").value = Math.round(percent);
  _("status").innerHTML = Math.round(percent) + "% uploaded... please wait";
  if (percent >= 100) {
    _("status").innerHTML = "Please wait, writing file to filesystem";
  }
}
function completeHandler(event) {
  _("status").innerHTML = "Upload Complete";
  _("progressBar").value = 0;
  var actualFolder = document.getElementById("actualFolder").value
  document.getElementById("status").innerHTML = "File Uploaded";
  listFilesButton(actualFolder);
}
function errorHandler(event) {
  _("status").innerHTML = "Upload Failed";
}
function abortHandler(event) {
  _("status").innerHTML = "inUpload Aborted";
}
</script>
</body>
</html>
)rawliteral";

const char logout_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
</head>
<body>
  <p><a href="/">Log Back In</a></p>
</body>
</html>
)rawliteral";

// reboot.html base upon https://gist.github.com/Joel-James/62d98e8cb3a1b6b05102
const char reboot_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
  <meta charset="UTF-8">
</head>
<body>
<h3>
  Rebooting, returning to main page in <span id="countdown">30</span> seconds
</h3>
<script type="text/javascript">
  var seconds = 20;
  function countdown() {
    seconds = seconds - 1;
    if (seconds < 0) {
      window.location = "/";
    } else {
      document.getElementById("countdown").innerHTML = seconds;
      window.setTimeout("countdown()", 1000);
    }
  }
  countdown();
</script>
</body>
</html>
)rawliteral";
