const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">

  <!-- CSS sample thanks to @im.nix (Discord) -->
  <style>
    .gg-folder {
      cursor: pointer;
      transform: scale(var(--ggs,1))
    }
    .gg-folder,
    .gg-folder::after {
        box-sizing: border-box;
        position: relative;
        display: inline-block;
        width: 22px;
        height: 16px;
        border: 2px solid;
        border-radius: 3px
    }
    .gg-folder::after {
        content: "";
        position: absolute;
        width: 10px;
        height: 4px;
        border-bottom: 0;
        border-top-left-radius: 2px;
        border-top-right-radius: 4px;
        border-bottom-left-radius: 0;
        border-bottom-right-radius: 0;
        top: -5px
    }
    .gg-trash {
      box-sizing: border-box;
      position: relative;
      display: inline-block;
      transform: scale(var(--ggs,1));
      width: 10px;
      height: 12px;
      border: 2px solid transparent;
      box-shadow:
          0 0 0 2px,
          inset -2px 0 0,
          inset 2px 0 0;
      border-bottom-left-radius: 1px;
      border-bottom-right-radius: 1px;
      margin-top: 4px;
      margin-bottom: 2px;
    cursor: pointer;
    }
    .gg-trash::after,
    .gg-trash::before {
        content: "";
        display: block;
        box-sizing: border-box;
        position: absolute
    }
    .gg-trash::after {
        background: currentColor;
        border-radius: 3px;
        width: 16px;
        height: 2px;
        top: -4px;
        left: -5px
    }
    .gg-trash::before {
        width: 10px;
        height: 4px;
        border: 2px solid;
        border-bottom: transparent;
        border-top-left-radius: 2px;
        border-top-right-radius: 2px;
        top: -7px;
        left: -2px
    }
    .gg-arrow-down-r {
        box-sizing: border-box;
        position: relative;
        display: inline-block;
        width: 22px;
        height: 22px;
        border: 2px solid;
        transform: scale(var(--ggs,1));
        cursor: pointer;
        border-radius: 4px
    }
    .gg-arrow-down-r::after,
    .gg-arrow-down-r::before {
        content: "";
        display: block;
        box-sizing: border-box;
        position: absolute;
        bottom: 4px
    }
    .gg-arrow-down-r::after {
        width: 6px;
        height: 6px;
        border-bottom: 2px solid;
        border-left: 2px solid;
        transform: rotate(-45deg);
        left: 6px
    }
    .gg-arrow-down-r::before {
        width: 2px;
        height: 10px;
        left: 8px;
        background: currentColor
    }
    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
      margin: 0;
      padding: 5px;
      color: #f0f0f0;
      background-color: #202124;
    }

    .container {
      max-width: 800px;
      margin: 5px auto;
      padding: 0 5px;
    }

    h3 {
      margin: 0;
      padding: 10px 0;
      border-bottom: 1px solid rgba(255, 255, 255, 0.1);
    }
 
    table {
      width: 100%%;
      border-collapse: collapse;
      border-bottom: 1px solid rgba(255, 255, 255, 0.1);
    }
 
    th, td {
      padding: 5px;
      border-bottom: 1px solid rgba(255, 255, 255, 0.1);
    }
 
    th {
      text-align: left;
    }
 
    a {
      color: #87CEEB;
      text-decoration: none;
    }
 
    a:hover {
      text-decoration: underline;
    }
 
    button {
      background-color: #303134;
      color: #fff;
      border: none;
      padding: 4px 8px;
      border-radius: 4px;
      cursor: pointer;
      margin: 5px;
    }
 
    button:hover {
      background-color: #292a2c;
    }
 
    #detailsheader, #updetailsheader {
      display: flex;
      justify-content: space-between;
    }
 
    @media (prefers-color-scheme: light) {
      body {
        color: #333;
        background-color: #f0f0f0;
      }
 
      h3 {
        border-bottom: 1px solid rgba(0, 0, 0, 0.1);
      }
 
      table, th, td {
        border-bottom: 1px solid rgba(0, 0, 0, 0.1);
      }
 
      a {
        color: #007bff;
      }
 
      button {
        background-color: #e9ecef;
        color: #333;
      }
 
      button:hover {
        background-color: #d3d3d3;
      }
    }
 
    @media (max-width: 768px) {
      body {
        font-size: 14px;
      }
 
      table {
        font-size: 12px;
      }
 
      th, td {
        padding: 5px;
      }
 
      button {
        font-size: 12px;
        padding: 6px 12px;
      }
    }
    th:first-child, td:first-child {
      width: 65%%;
    }
    th:last-child, td:last-child {
      width: 60px;
      text-align: center;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>File Management</h1>
    <p>Firmware version: %FIRMWARE%</p>
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
  </div>

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
  setTimeout(function(){ window.open("/logged-out","_self"); }, 500);
}

function rebootButton() {
  document.getElementById("status").innerHTML = "Invoking Reboot ...";
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
  var actualFolder = document.getElementById("actualFolder").value;
  var option;
  if (action == "delete") {
    option = confirm("Do you really want to DELETE the file: " + filename + " ?\n\nThis action can't be undone!");
  }

  xmlhttp=new XMLHttpRequest();
  if (option == true || action=="create") {
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
  "<progress id=\"progressBar\" value=\"0\" max=\"100\" style=\"width:100%;\"></progress>" +
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

window.addEventListener("load", function() {
  listFilesButton("/");
});

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
  <style>
    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
      margin: 0;
      padding: 20px;
      color: #f0f0f0;
      background-color: #202124;
    }
 
    h3 {
      margin: 0;
      padding: 10px 0;
      border-bottom: 1px solid rgba(255, 255, 255, 0.1);
    }
 
    #detailsheader, #updetailsheader {
      display: flex;
      justify-content: space-between;
    }
 
    @media (prefers-color-scheme: light) {
      body {
        color: #333;
        background-color: #f0f0f0;
      }
 
      h3 {
        border-bottom: 1px solid rgba(0, 0, 0, 0.1);
      }
    }
 
    @media (max-width: 768px) {
      body {
        font-size: 14px;
      }
    }
  </style>
</head>
<body>
  <h3><a href="/">Log Back In</a></h3>
</body>
</html>
)rawliteral";

// reboot.html base upon https://gist.github.com/Joel-James/62d98e8cb3a1b6b05102
const char reboot_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <style>
    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
      margin: 0;
      padding: 20px;
      color: #f0f0f0;
      background-color: #202124;
    }
 
    h3 {
      margin: 0;
      padding: 10px 0;
      border-bottom: 1px solid rgba(255, 255, 255, 0.1);
    }
 
    @media (prefers-color-scheme: light) {
      body {
        color: #333;
        background-color: #f0f0f0;
      }
 
      h3 {
        border-bottom: 1px solid rgba(0, 0, 0, 0.1);
      }
    }
 
    @media (max-width: 768px) {
      body {
        font-size: 14px;
      }
 
    }
  </style>
</head>
<body>
<h3>
  Rebooting... trying to return to main page in <span id="countdown">30</span> seconds
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
