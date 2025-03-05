#include "BoatControllerWifi.h"
#include "BoatControllerServos.h"
#include "BoatControllerDefines.h"
#include "BoatControllerConfig.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "FFat.h"
#include <iostream>
#include <sstream>
#include <Update.h>
#include <ArduinoJson.h>
#include <ssl_client.h>
#include <WiFiClientSecure.h>
#include <ESP32httpUpdate.h>
#include <esp_task_wdt.h>

extern BoatControllerConfigClass bContConfig;
extern BoatControllerServosClass bContServos;
extern DynamicJsonDocument config;
extern JsonArray Servos;
extern JsonArray Channels;
extern JsonArray ServoTypes;
extern String firmwareUpdateFile;
extern void WriteDebug(String msg);
extern bool debugServos;
extern bool debugRCChannels;
extern bool debugCompass;
extern bool debugTracking;

const char header_html[] PROGMEM = "<!DOCTYPE html><html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8'/><meta name='viewport' content='width=device-width, initial-scale=1, minimum-scale=1.0, shrink-to-fit=no'><title>GreenPonik.com - WebView</title></head><body>";
const char footer_html[] PROGMEM = "</body></html>";
const char update_html[] PROGMEM = "<h1>Only .bin file</h1><form method='POST' action='/updt' enctype='multipart/form-data'><input type='file' name='update' required><input type='submit' value='Run Update'></form>";


bool filereading = false;
bool espShouldReboot = false;
String postData = "";

AsyncWebServer* server;
AsyncWebSocket wsHTTPInput("/HTTPInput");

bool getUpdateFileFromServer() {
	Serial.printf("Checking for updates...\n");
	t_httpUpdate_return ret = ESPhttpUpdate.update(String(WebUpdateHost) + String(WebUpdatePath));

	switch (ret) {
	case HTTP_UPDATE_FAILED:
		Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
		break;

	case HTTP_UPDATE_NO_UPDATES:
		Serial.println("HTTP_UPDATE_NO_UPDATES\n");
		break;

	case HTTP_UPDATE_OK:
		Serial.println("HTTP_UPDATE_OK\n");
		break;
	}
}

void FormatFFAT() {
	WriteDebug("Formatting FFAT Drive...");
	FFat.end();
	if (FFat.format()) {
		if (FFat.begin()) {
			bContConfig.saveConfig();
			WriteDebug("Formatting Complete!");
		}
		else
			WriteDebug("Formated drive but did not save config!");
	}
	else
		WriteDebug("Formatting Failed!");
}
	
void UpdateWebPages() {
	WriteDebug("Updating Web Pages...");	
	
	HTTPClient http;
	uint8_t buff[512] = { 0 };
						
	http.begin(String(WebUpdateHost) + String(WebFilesUpdateManifest));
	int httpResponseCode = http.GET();
	
	if (httpResponseCode > 0) {
		String jsonBuffer;
		jsonBuffer = http.getString();
		DynamicJsonDocument doc(1024); 
		DeserializationError error = deserializeJson(doc, jsonBuffer);
		JsonArray files = doc["Files"];
		for (JsonVariant file : files) {
			esp_task_wdt_init(30, false);

			String fileName = "/" +  file.as<String>();
			String sourceFile = String(WebUpdateHost)  + doc["Source"].as<String>() + fileName;
			WriteDebug("Downloading " + fileName);
		
			http.begin(sourceFile);
			int httpCode = http.GET();
			if (httpCode > 0) {
				int totalLength = http.getSize();
				WiFiClient* stream = http.getStreamPtr();

				int len = totalLength;
				File wfile = FFat.open(fileName.c_str(), FILE_WRITE);
				if (!wfile) {
					WriteDebug("Could not create file " + fileName + "!");					
				}
				else {
					while (http.connected() && (len > 0 || len == -1)) {
						// get available data size
						size_t size = stream->available();
						if (size) {
							// read up to 128 byte
							int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
							wfile.write(buff, c);
							if (len > 0) {
								len -= c;
							}
						}
						float Percent = 100 * (totalLength - len) / totalLength;
						delay(1);
					}
					wfile.close();
				}				
			}
			vTaskDelay(5);
		
		}
		WriteDebug("Update Completed!");
		
	}
	else {
		Serial.print("Error code: ");
		Serial.println(httpResponseCode);
	}
	
	http.end();

}

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
	char msg[200];
        
    switch (type)
    {
    case WS_EVT_CONNECT:
		sprintf(msg,"WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
		WriteDebug(msg);
        break;
    case WS_EVT_DISCONNECT:
		sprintf(msg, "WebSocket client #%u disconnected\n", client->id());
		WriteDebug(msg);
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
			
			for (JsonVariant channel : Channels) {
				if (channel["channel"].as<String>().equals(key.c_str())) {
					int valueInt = atoi(value.c_str());
					sprintf(msg, "Key [%s] Value[%d]", key.c_str(), valueInt);
					WriteDebug(msg);			
					JsonVariant servoType = ServoTypes[Servos[channel["servo"].as<int>()]["type"].as<int>()];
					int min = servoType["min"].as<int>();
					int max = servoType["max"].as<int>();

					valueInt = map(valueInt, -100, 100, min, max);
					Serial.printf("%s Channel [%d] Servo [%d]  Min [%d] Max[%d] Deg[%d]\n", Servos[channel["servo"]]["dscn"].as<String>(),channel["ID"].as<int>(), min, max, valueInt);
					bContServos.moveServo(atoi(key.c_str()), atoi(key.c_str()), valueInt);					
				}
			}                       

			if (key == "debugServos")
				debugServos = value == "true";
			if (key == "debugChannels")
				debugRCChannels = value == "true";
			if (key == "debugCompass")
				debugCompass = value == "true";
			if (key == "debugTracking")
				debugTracking = value == "true";


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
		returnText += "<div class='fileList'><table><tr><th align='left'>Name</th><th align='left'>Size</th><th>Action</th></tr>";
	}
	while (foundfile) {
		if (ishtml) {
			returnText += "<tr align='left'><td>" + String(foundfile.name()) + "</td><td align='center'>" + humanReadableSize(foundfile.size()) + "</td>";
			returnText += "<td><button onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'download\')\">Download</button>";
			returnText += "<button onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'delete\')\">Delete</button></td></tr>";
		}
		else {
			returnText += "File: " + String(foundfile.name()) + " Size: " + humanReadableSize(foundfile.size()) + "\n";
		}
		foundfile = root.openNextFile();
	}
	if (ishtml) {
		returnText += "</table></div>";
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
	String staMode = config["Params"]["STAMode"].as<String>();
	staMode.toLowerCase();
	String autoUpdate = config["Params"]["AutoUpdate"].as<String>();
	autoUpdate.toLowerCase();

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
		this->IP = WiFi.localIP();		
		if (WiFi.isConnected() && autoUpdate.equals("true")) {
			getUpdateFileFromServer();
		}
	}
	
	if (!WiFi.isConnected()) {
		WiFi.mode(WIFI_AP);
		WiFi.softAP(config["Params"]["APName"].as<String>().c_str(), config["Params"]["APPass"].as<String>().c_str());
		Serial.print("Using AP Mode ..");
		this->IP = WiFi.softAPIP();
	}
	
	Serial.print("IP address: ");
	this->ControllerIPAddress = this->IP.toString();
	Serial.println(this->ControllerIPAddress);
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

					StaticJsonDocument<8192> doc;
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

	server->on("/reboot", HTTP_GET, [](AsyncWebServerRequest* request) { 
		request->send(200, "text/plain", "Device Reboot issued!");
		ESP.restart(); 		
	});
	server->on("/format", HTTP_GET, [](AsyncWebServerRequest* request) { FormatFFAT(); });
	
	server->on("/updateweb", HTTP_GET, [](AsyncWebServerRequest* request) { 
		request->send(200, "application/json", "{ \"status\": 0 }");
		delay(10);
		UpdateWebPages(); }
	);

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
	if (this->bUpdateWebPages==true) {		
		Serial.println("Queued Web Page Update");		
		this->bUpdateWebPages = false;		
	}
}

void BoatControllerWififClass::sendSocketMessage(String msg)
{
	wsHTTPInput.textAll(msg.c_str(), msg.length());
}

void BoatControllerWififClass::UpdateDeviceWebPages() {
	this->bUpdateWebPages = true;	
}
