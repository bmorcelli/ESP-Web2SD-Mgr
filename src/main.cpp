#define FIRMWARE_VERSION "v0.0.1"

#include <WiFi.h>
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include <SD.h>
#include <SPI.h>
#include <ESPmDNS.h>
#include <TFT_eSPI.h>


// configuration structure
struct Config {
  String ssid;               // wifi ssid
  String wifipassword;       // wifi password
  String httpuser;           // username to access web admin
  String httppassword;       // password to access web admin
  int webserverporthttp;     // http port number for web admin
};

// variables
  // WiFi as a Client
  String default_ssid = "SSID_of_my_network";
  String default_wifipassword = "Password_of_my_network";
  String default_httpuser = "admin";
  String default_httppassword = "admin";
  const int default_webserverporthttp = 80;

  //WiFi as an Access Point
  const char* apSsidName = "WiFi2Sd_Mgr";
  IPAddress AP_GATEWAY(172, 0, 0, 1);  // Gateway

Config config;                        // configuration
SPIClass sdcardSPI;
AsyncWebServer *server;               // initialise webserver
const char* host = "esp";
const String fileconf = "/ESPWeb2SD.txt";
bool shouldReboot = false;            // schedule a reboot


// function defaults
void rebootESP(String message);
String humanReadableSize(const size_t bytes);
String listFiles(bool ishtml, String folder);


// MACROS
TFT_eSPI tft = TFT_eSPI();         // Invoke custom library

#include "webpages.h"

void rebootESP(String message) {
  Serial.print("Rebooting ESP32: "); Serial.println(message);
  ESP.restart();
}

// Make size of files human readable
// source: https://github.com/CelliesProjects/minimalUploadAuthESP32

String humanReadableSize(const size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}

// list all of the files, if ishtml=true, return html rather than simple text
String listFiles(bool ishtml, String folder) {
  String returnText = "";
  Serial.println("Listing files stored on SD");

  if (ishtml) {
    returnText += "<table><tr><th align='left'>Name</th><th align='left'>Size</th><th></th><th></th></tr>";
  }
  File root = SD.open(folder);
  File foundfile = root.openNextFile();
  String PreFolder = folder;
  PreFolder = PreFolder.substring(0, PreFolder.lastIndexOf("/"));
  if(PreFolder != ""){ returnText += "<tr><th align='left'><a onclick=\"listFilesButton('"+ PreFolder + "')\" href='javascript:void(0);'>... </a></th><th align='left'></th><th></th><th></th></tr>"; }

  if (folder=="/") folder = "";
  while (foundfile) {
    if(foundfile.isDirectory()) {
      if (ishtml) {
        returnText += "<tr align='left'><td><a onclick=\"listFilesButton('"+ folder +"/" + String(foundfile.name()) + "')\" href='javascript:void(0);'>" + String(foundfile.name()) + "</a></td><td>" + humanReadableSize(foundfile.size()) + "</td>";
        returnText += "<td><button onclick=\"listFilesButton('" + folder + "/" + String(foundfile.name()) + "')\">Open Folder</button>";
        returnText += "<td><button onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'delete\')\">Delete</button></tr>";
      } else {
        returnText += "Folder: " + String(foundfile.name()) + " Size: " + humanReadableSize(foundfile.size()) + "\n";
      }
    }
    foundfile = root.openNextFile();
  }
  root.close();
  foundfile.close();

  root = SD.open(folder);
  foundfile = root.openNextFile();
  while (foundfile) {
    if(!(foundfile.isDirectory())) {
      if (ishtml) {
        returnText += "<tr align='left'><td>" + String(foundfile.name()) + "</td><td>" + humanReadableSize(foundfile.size()) + "</td>";
        returnText += "<td><button onclick=\"downloadDeleteButton(\'"+ folder + "/" + String(foundfile.name()) + "\', \'download\')\">Download</button>";
        returnText += "<td><button onclick=\"downloadDeleteButton(\'"+ folder + "/" + String(foundfile.name()) + "\', \'delete\')\">Delete</button></tr>";
      } else {
        returnText += "File: " + String(foundfile.name()) + " Size: " + humanReadableSize(foundfile.size()) + "\n";
      }
    }
    foundfile = root.openNextFile();
  }
  root.close();
  foundfile.close();

  if (ishtml) {
    returnText += "</table>";
  }

  return returnText;
}

// parses and processes webpages
// if the webpage has %SOMETHING% or %SOMETHINGELSE% it will replace those strings with the ones defined
String processor(const String& var) {
  if (var == "FIRMWARE") return FIRMWARE_VERSION;
  else if (var == "FREESD") return humanReadableSize(SD.usedBytes());
  else if (var == "USEDSD") return humanReadableSize((SD.cardSize() - SD.usedBytes())*2);
  else if (var == "TOTALSD") return humanReadableSize((SD.cardSize()));
  else return "Attribute not configured"; 
}

// used by server.on functions to discern whether a user has the correct httpapitoken OR is authenticated by username and password
bool checkUserWebAuth(AsyncWebServerRequest * request) {
  bool isAuthenticated = false;

  if (request->authenticate(config.httpuser.c_str(), config.httppassword.c_str())) {
    Serial.println("is authenticated via username and password");
    isAuthenticated = true;
  }
  return isAuthenticated;
}

// handles uploads to the filserver
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  // make sure authenticated before allowing upload
  if (checkUserWebAuth(request)) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    Serial.println(logmessage);

    if (!index) {
      logmessage = "Upload Start: " + String(filename);
      // open the file on first call and store the file handle in the request object
      request->_tempFile = SD.open("/" + filename, "w");
      Serial.println(logmessage);
    }

    if (len) {
      // stream the incoming chunk to the opened file
      request->_tempFile.write(data, len);
      logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
      Serial.println(logmessage);
    }

    if (final) {
      logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
      // close the file handle as the upload is now done
      request->_tempFile.close();
      Serial.println(logmessage);
      request->redirect("/");
    }
  } else {
    Serial.println("Auth: Failed");
    return request->requestAuthentication();
  }
}

void notFound(AsyncWebServerRequest *request) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);
  request->send(404, "text/plain", "Not found");
}


void configureWebServer() {
  // configure web server
  
  MDNS.begin(host);
  
  // if url isn't found
  server->onNotFound([](AsyncWebServerRequest * request) {
    request->redirect("/");
  });

  // run handleUpload function when any file is uploaded
  server->onFileUpload(handleUpload);

  // visiting this page will cause you to be logged out
  server->on("/logout", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->requestAuthentication();
    request->send(401);
  });

  // presents a "you are now logged out webpage
  server->on("/logged-out", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    Serial.println(logmessage);
    request->send_P(401, "text/html", logout_html, processor);

  });

  server->on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + + " " + request->url();

    if (checkUserWebAuth(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      request->send_P(200, "text/html", index_html, processor);
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }

  });

  server->on("/reboot", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();

    if (checkUserWebAuth(request)) {
      request->send(200, "text/html", reboot_html);
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      shouldReboot = true;
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }
  });

  server->on("/listfiles", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    if (checkUserWebAuth(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      if (request->hasParam("folder")) {
        String folder = request->getParam("folder")->value().c_str();
        request->send(200, "text/plain", listFiles(true, folder));
      } else {
        request->send(400, "text/plain", "Folder not specified");
      }
      //request->send(200, "text/plain", listFiles(true));
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }
  });

  server->on("/file", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    if (checkUserWebAuth(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);

      if (request->hasParam("name") && request->hasParam("action")) {
        const char *fileName = request->getParam("name")->value().c_str();
        const char *fileAction = request->getParam("action")->value().c_str();

        logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url() + "?name=" + String(fileName) + "&action=" + String(fileAction);

        if (!SD.exists(fileName)) {
          Serial.println(logmessage + " ERROR: file does not exist");
          request->send(400, "text/plain", "ERROR: file does not exist");
        } else {
          Serial.println(logmessage + " file exists");
          if (strcmp(fileAction, "download") == 0) {
            logmessage += " downloaded";
            request->send(SD, fileName, "application/octet-stream");
          } else if (strcmp(fileAction, "delete") == 0) {
            logmessage += " deleted";
            SD.remove(fileName);
            request->send(200, "text/plain", "Deleted File: " + String(fileName));
          } else {
            logmessage += " ERROR: invalid action param supplied";
            request->send(400, "text/plain", "ERROR: invalid action param supplied");
          }
          Serial.println(logmessage);
        }
      } else {
        request->send(400, "text/plain", "ERROR: name and action params required");
      }
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }
  });
}


void setupWiFi(bool Is_AP) {
  //setup credentials of the portal an http server port
  config.httpuser = default_httpuser;
  config.httppassword = default_httppassword;
  config.webserverporthttp = default_webserverporthttp;

  if(!Is_AP) { // Running in Wifi Client Mode
    config.ssid = default_ssid;
    config.wifipassword = default_wifipassword;
    
    WiFi.begin(config.ssid.c_str(), config.wifipassword.c_str());
    int i=0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      tft.print(".");
      if (i==80) break; //Couldn't connect to wifi, show error msg and restart ESP
      i++;
    }
    if (i==80) { // Couldn't connect to wifi, show error msg and restart ESP
      #ifdef HAS_SCREEN
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_RED, TFT_BLACK); // Set the font colour and the background colour
      tft.setCursor(0,0);
      tft.println("Couldn't connect to " + config.ssid + "\n->Restarting");
      #endif
      Serial.println("\n\nCouldn't connect to " + config.ssid + ". Restarting ESP in 5 secconds.");
      delay(1000); Serial.println("4 secconds");
      delay(1000); Serial.println("3 secconds");
      delay(1000); Serial.println("2 secconds");
      delay(1000); Serial.println("1 secconds");
      ESP.restart();
    }
  } 
  else { //Running in Access point mode
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_GATEWAY, AP_GATEWAY, IPAddress(255, 255, 255, 0));
    WiFi.softAP(apSsidName, "",6,0,1,false);
    Serial.print("IP: "); Serial.println(WiFi.softAPIP());
  } 
}


String readLineFromFile(File myFile) {
  String line = "";
  char character;

  while (myFile.available()) {
    character = myFile.read();
    if (character == ';') {
      break;
    }
    line += character;
  }
  return line;
}

void setup() {
  Serial.begin(115200);
  Serial.print("Firmware: "); Serial.println(FIRMWARE_VERSION);
  Serial.println("Booting ...");

  #ifdef HAS_SCREEN
    tft.begin();
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK); // Set the font colour and the background colour
    tft.setCursor(0,0);
    tft.setTextSize(1);
    tft.setRotation(ROTATION);
  #endif

  Serial.println("Mounting SD ...");  
  sdcardSPI.begin(SDCARD_SCK, SDCARD_MISO, SDCARD_MOSI, SDCARD_CS); 
  while (!SD.begin(SDCARD_CS, sdcardSPI)) { //excluded * -> poiter indicator -
    Serial.println("Failed to mount SDCARD\nTrying again in 1 sec...");

    #ifdef HAS_SCREEN
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0,0);    
    tft.println("Fail to mount SD-Card\nTrying again in 1 second.");
    #endif

    delay(1000);
  }
  Serial.print("SD Free: "); Serial.print(SD.totalBytes()/(1024*1024)); Serial.println(" Mb");
  Serial.print("SD Used: "); Serial.println(humanReadableSize(((2*(SD.cardSize() - SD.totalBytes())))));
  Serial.print("SD Total: "); Serial.print(SD.cardSize()/(1024*1024)); Serial.println(" Mb");

  Serial.println("Loading Configuration ...");
 
  if(SD.exists(fileconf)) {
    Serial.println("File Exists, reading " + fileconf);
    File file = SD.open(fileconf, FILE_READ);
    if(!file) {
      Serial.println("Fail to open " + fileconf + ". Creating a custom file.");
    } 
    else {
      default_ssid = readLineFromFile(file);
      default_wifipassword = readLineFromFile(file);
      default_httpuser = readLineFromFile(file);
      default_httppassword = readLineFromFile(file);
      file.close();
    }
  }
  else {
    Serial.println("File does't Exist, writing defaults on " + fileconf);
    File file = SD.open(fileconf, FILE_WRITE);
    file.print(default_ssid + ";" + default_wifipassword + ";" + default_httpuser + ";" + default_httppassword + ";\n");
    file.print("#" + default_ssid + ";" + default_wifipassword + ";ManagerUser;ManagerPassword;");
    //file.print();
    file.close();
  }

  // Choose wifi access mode
  bool mode_ap = true;

  #if defined (HAS_SCREEN) && defined(HAS_BTN)
  pinMode(BTN_PIN, INPUT);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  bool has_change = true;

  while(1) {
    if(digitalRead(BTN_PIN)== LOW) { // short press, change option
      delay(250);
      if(digitalRead(BTN_PIN)== LOW) break; // long press, stops While and start WiFi
      mode_ap=!mode_ap;
      has_change=true;
    }
    if(has_change) {
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0,0);
      tft.println("Choose access mode:");
      if(mode_ap)  tft.println("(x) Access Point"); 
      else tft.println("( ) Access Point");
      if(!mode_ap) tft.println("(x) My network\n"); 
      else tft.println("( ) My network\n");
      tft.setTextSize(1);
      tft.print("Short Press btn "); tft.print(BTN_ALIAS); tft.println(" to change.");
      tft.println("Long Press btn "); tft.print(BTN_ALIAS); tft.println(" to select.");
      tft.setTextSize(2);
      has_change=false;
    }    
  }
  #endif

  #ifdef HAS_SCREEN
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0,0);
  tft.println("Starting Network:");
  tft.println("SSID: " + default_ssid);
  tft.println("Pass: " + default_wifipassword + "\n");
  #endif


  Serial.print("\nConnecting to Wifi: ");
  setupWiFi(mode_ap);

  #ifdef HAS_SCREEN
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0,0);
  tft.println("Starting Network:");
  if(mode_ap)  { tft.print("SSID: " + WiFi.softAPSSID() + "\n\nIP: ");  tft.print(WiFi.softAPIP()); tft.print("\nhttp://esp.local\nUsr: "+ default_httpuser + "\nPwd: " + default_httppassword); }
  if(!mode_ap) { tft.print("IP: "); tft.println(WiFi.localIP()); tft.println("http://esp.local\nUsr: "+ default_httpuser + "\nPwd: " + default_httppassword); }

  #endif


  // configure web server
  Serial.println("Configuring Webserver ...");
  server = new AsyncWebServer(config.webserverporthttp);
  configureWebServer();

  // startup web server
  Serial.println("Starting Webserver ...");
  server->begin();
  Serial.println("\nAccess: http://esp.local/\n");
}

void loop() {
  // reboot if we've told it to reboot
  if (shouldReboot) {
    rebootESP("Web Admin Initiated Reboot");
  }
}