#include "BoatControllerWifi.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "FFat.h"
#include <iostream>
#include <sstream>
#include "BoatControllerDefines.h"
#include <Update.h>
#include <ArduinoJson.h>
#include "BoatControllerConfig.h"

extern BoatControllerConfigClass bContConfig;
extern DynamicJsonDocument config;
extern JsonArray Servos;
extern String firmwareUpdateFile;


const char header_html[] PROGMEM = "<!DOCTYPE html><html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8'/><meta name='viewport' content='width=device-width, initial-scale=1, minimum-scale=1.0, shrink-to-fit=no'><title>GreenPonik.com - WebView</title></head><body>";
const char footer_html[] PROGMEM = "</body></html>";
const char update_html[] PROGMEM = "<h1>Only .bin file</h1><form method='POST' action='/updt' enctype='multipart/form-data'><input type='file' name='update' required><input type='submit' value='Run Update'></form>";


bool filereading = false;
bool espShouldReboot = false;
String postData = "";
String updateHost = "";
String updatePath = "";

AsyncWebServer* server;
AsyncWebSocket wsHTTPInput("/HTTPInput");

// handles uploads to the filserver
void handleUpload(AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
	if (!index) {
		Serial.println((String) "UploadStart: " + filename);
		// open the file on first call and store the file handle in the request object
		request->_tempFile = FFat.open("/" +  filename, "w");
		if (!request->_tempFile) {
			Serial.println("- failed to open file for writing");
			return;
		}
	}
	if (len) {
		// stream the incoming chunk to the opened file
		request->_tempFile.write(data, len);
	}
	if (final) {
		Serial.println((String) "UploadEnd: " + filename + "," + index + len);
		// close the file handle as the upload is now done
		request->_tempFile.close();
		request->send(200, "text/plain", "File Uploaded !");
	}
}


void listDir(fs::FS& fs, const char* dirname) {
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if (!root) {
        Serial.println("- failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.print("  DIR : ");
            Serial.println(file.name());
        }
        else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void readFile(fs::FS& fs, const char* path) {
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if (!file || file.isDirectory()) {
        Serial.println("- failed to open file for reading");
        return;
    }

    Serial.println("- read from file:");
    while (file.available()) {
        Serial.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS& fs, const char* path, const char* message) {
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("- failed to open file for writing");
        return;
    }
    if (file.print(message)) {
        Serial.println("- file written");
    }
    else {
        Serial.println("- write failed");
    }
    file.close();
}

void onHTTPInputWebSocketEvent(AsyncWebSocket* server,
    AsyncWebSocketClient* client,
    AwsEventType type,
    void* arg,
    uint8_t* data,
    size_t len)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
    case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
        AwsFrameInfo* info;
        info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
        {
            std::string myData = "";
            myData.assign((char*)data, len);
            std::istringstream ss(myData);
            std::string key, value;
            std::getline(ss, key, ',');
            std::getline(ss, value, ',');
            Serial.printf("Key [%s] Value[%s]\n", key.c_str(), value.c_str());
            int valueInt = atoi(value.c_str());
			int servoType=1;
			
            if (key == "Bearing")
				servoType = 2;

			for (JsonVariant value : Servos) {
				if (value["type"].as<int>() == servoType) {
					value["target"].set(valueInt);
				}
			}                       
        }
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    default:
        break;
    }
}

String humanReadableSize(const size_t bytes) {
	if (bytes < 1024)
		return String(bytes) + " B";
	else if (bytes < (1024 * 1024))
		return String(bytes / 1024.0) + " KB";
	else if (bytes < (1024 * 1024 * 1024))
		return String(bytes / 1024.0 / 1024.0) + " MB";
	else
		return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}


// list all of the files, if ishtml=true, return html rather than simple text
String listFiles(bool ishtml) {
	String returnText = "";
	Serial.println("Listing files stored on FFat");
	File root = FFat.open("/");
	File foundfile = root.openNextFile();
	if (ishtml) {
		returnText += "<table><tr><th align='left'>Name</th><th align='left'>Size</th><th></th><th></th></tr>";
	}
	while (foundfile) {
		if (ishtml) {
			returnText += "<tr align='left'><td>" + String(foundfile.name()) + "</td><td>" + humanReadableSize(foundfile.size()) + "</td>";
			returnText += "<td><button onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'download\')\">Download</button>";
			returnText += "<td><button onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'delete\')\">Delete</button></tr>";
		}
		else {
			returnText += "File: " + String(foundfile.name()) + " Size: " + humanReadableSize(foundfile.size()) + "\n";
		}
		foundfile = root.openNextFile();
	}
	if (ishtml) {
		returnText += "</table>";
	}
	root.close();
	foundfile.close();
	return returnText;
}


void notFound(AsyncWebServerRequest* request) {
	String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
	Serial.println(logmessage);
	request->send(404, "text/plain", "Not found");
}

void BoatControllerWififClass::init() {
	
	listDir(FFat, "/");
	IPAddress IP;
	String staMode = config["Params"]["STAMode"].as<String>();
	staMode.toLowerCase();
	if (staMode.equals("true"))
		Serial.println("Station Mode");
	else
		Serial.println("AP Mode");

	if (staMode.equals("true")) {
		WiFi.mode(WIFI_STA);
		WiFi.begin(config["Params"]["STAName"].as<String>().c_str(), config["Params"]["STAPwd"].as<String>().c_str());
		Serial.print("Connecting to WiFi ..");
		int retries=0;
		while (WiFi.status() != WL_CONNECTED && retries<10) {
			retries++;
			Serial.print('.');
			delay(1000);
		}
		IP = WiFi.localIP();
		Serial.println(WiFi.localIP());
	}
	
	if (!WiFi.isConnected()) {
		WiFi.mode(WIFI_AP);
		WiFi.softAP(config["Params"]["APName"].as<String>().c_str(), config["Params"]["APPass"].as<String>().c_str());
		Serial.print("Using AP Mode ..");
		IP = WiFi.softAPIP();
	}
	
	Serial.print("IP address: ");
	Serial.println(IP);
	server = new AsyncWebServer(80);
	
	server->serveStatic("/", FFat, "/").setDefaultFile("index.html");
	
	wsHTTPInput.onEvent(onHTTPInputWebSocketEvent);
	
	// run handleUpload function when any file is uploaded
	server->on("/upload", HTTP_POST, [](AsyncWebServerRequest* request) { request->send(200); }, handleUpload);
	
	server->on("/update", HTTP_POST, [](AsyncWebServerRequest* request) {
  		espShouldReboot = !Update.hasError();
  		AsyncWebServerResponse *response = request->beginResponse(200, "text/html", espShouldReboot ? "<h1><strong>Update DONE</strong></h1><br><a href='/'>Return Home</a>" : "<h1><strong>Update FAILED</strong></h1><br><a href='/updt'>Retry?</a>");
  		response->addHeader("Connection", "close");
  		request->send(response); }, [](AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
		if (!index)
		{
			Serial.printf("Update Start: %s\n", filename.c_str());
			if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000))
			{
				Update.printError(Serial);
			}
		}
		if (!Update.hasError())
		{
			if (Update.write(data, len) != len)
			{
				Update.printError(Serial);
			}
		}
		if (final)
		{
			if (Update.end(true))
			{
				Serial.printf("Update Success: %uB\n", index + len);
			}
			else
			{
				Update.printError(Serial);
			}
		} });

	server->on("/listfiles", HTTP_GET, [](AsyncWebServerRequest* request) {
		String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
		logmessage += " Auth: Success";
		Serial.println(logmessage);
		request->send(200, "text/plain", listFiles(true));
	});
	
	server->on("/ConfigSection", HTTP_GET, [](AsyncWebServerRequest* request) {
		const char* param = request->getParam("Section")->value().c_str();
		String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
		String body;
		serializeJson(config[param],body);
		request->send(200, "text/json", body);
	});
	
	server->onRequestBody([](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
			if ((request->url() == "/SaveConfigSection") &&	(request->method() == HTTP_POST)) {
			
				Serial.printf("len = %d index = %d total = %d\n", len, index, total);
				if (index == 0)
					postData = "";

				for (int i = 0; i < len;i++) {
					postData += (char)data[i];
				}

				if (len + index == total) {
					Serial.println("Data: ");
					Serial.println(postData);
					Serial.println("Length: ");
					Serial.println(request->contentLength());

					StaticJsonDocument<4096> doc;
					DeserializationError error = deserializeJson(doc, postData);
					if (error) {
						request->send(400, "text/plain", "Fehlerhafte JSON Struktur");
						doc.clear();
					}
					else {
						const char* param = request->getParam("Section")->value().c_str();
						JsonObject obj = doc.as<JsonObject>();
						if (!obj)
						{
							JsonArray arr = doc.as<JsonArray>();
							config[param].set(arr);						
						}
						else
							config[param].set(obj);						

						
						bContConfig.saveConfig();
						request->send(200, "application/json", "{ \"status\": 0 }");
					}
				
				}
				
			}		
			
		});

	server->on("/reboot", HTTP_GET, [](AsyncWebServerRequest* request) { ESP.restart(); });

	server->on("/file", HTTP_GET, [](AsyncWebServerRequest* request) {
		String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
		logmessage += " Auth: Success";
		Serial.println(logmessage);

		if (request->hasParam("name") && request->hasParam("action")) {
			const char* fileName = request->getParam("name")->value().c_str();
			const char* fileAction = request->getParam("action")->value().c_str();

			logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url() + "?name=" + String(fileName) + "&action=" + String(fileAction);

			if (!FFat.exists(fileName)) {
				Serial.println(logmessage + " ERROR: file does not exist");
				request->send(400, "text/plain", "ERROR: file does not exist");
			}
			else {
				Serial.println(logmessage + " file exists");
				if (strcmp(fileAction, "download") == 0) {
					logmessage += " downloaded";
					request->send(FFat, fileName, "application/octet-stream");
				}
				else if (strcmp(fileAction, "delete") == 0) {
					logmessage += " deleted";
					FFat.remove(fileName);
					request->send(200, "text/plain", "Deleted File: " + String(fileName));
				}
				else {
					logmessage += " ERROR: invalid action param supplied";
					request->send(400, "text/plain", "ERROR: invalid action param supplied");
				}
				Serial.println(logmessage);
			}
		}
		else {
			request->send(400, "text/plain", "ERROR: name and action params required");
		}
	});

    server->addHandler(&wsHTTPInput);
    server->begin();
    Serial.println("HTTP server started");

}

void BoatControllerWififClass::doWork()
{
    wsHTTPInput.cleanupClients();
}

void BoatControllerWififClass::sendSocketMessage(String msg)
{
    wsHTTPInput.textAll(msg);
}
