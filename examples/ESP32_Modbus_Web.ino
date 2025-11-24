#include <WiFi.h>
#include <WebServer.h>
#include <HardwareSerial.h>

// WiFi credentials - CHANGE THESE
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// RS485 pins for ESP32-C3
#define RS485_RX 20  // GPIO20
#define RS485_TX 21  // GPIO21
#define RS485_DE 10  // GPIO10 - Driver Enable (DE/RE pins)

// Create serial port for RS485
HardwareSerial RS485Serial(1);

// Web server on port 80
WebServer server(80);

// Store communication history
#define MAX_HISTORY 50
String commHistory[MAX_HISTORY];
int historyIndex = 0;

void setup() {
  // Initialize debug serial
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nESP32-C3 Modbus Web Interface");
  
  // Initialize RS485 serial
  RS485Serial.begin(9600, SERIAL_8N1, RS485_RX, RS485_TX);
  
  // Configure DE pin
  pinMode(RS485_DE, OUTPUT);
  digitalWrite(RS485_DE, LOW); // Receive mode by default
  
  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/send", HTTP_POST, handleSend);
  server.on("/history", HTTP_GET, handleHistory);
  
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();
}

// Serve the main HTML page
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Modbus Interface</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
            background-color: #f0f0f0;
        }
        .container {
            background-color: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
            text-align: center;
        }
        .form-group {
            margin-bottom: 15px;
        }
        label {
            display: block;
            margin-bottom: 5px;
            font-weight: bold;
            color: #555;
        }
        input[type="text"], input[type="number"] {
            width: 100%;
            padding: 8px;
            border: 1px solid #ddd;
            border-radius: 4px;
            box-sizing: border-box;
            font-family: monospace;
        }
        button {
            background-color: #4CAF50;
            color: white;
            padding: 10px 20px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 16px;
            width: 100%;
        }
        button:hover {
            background-color: #45a049;
        }
        #history {
            margin-top: 20px;
            padding: 15px;
            background-color: #f9f9f9;
            border: 1px solid #ddd;
            border-radius: 4px;
            max-height: 400px;
            overflow-y: auto;
            font-family: monospace;
            font-size: 12px;
        }
        .history-item {
            margin-bottom: 10px;
            padding: 8px;
            background-color: white;
            border-left: 3px solid #4CAF50;
        }
        .sent {
            color: #0066cc;
        }
        .received {
            color: #006600;
        }
        .error {
            color: #cc0000;
            border-left-color: #cc0000;
        }
        .timestamp {
            color: #888;
            font-size: 10px;
        }
        #status {
            margin-top: 10px;
            padding: 10px;
            border-radius: 4px;
            display: none;
        }
        .status-success {
            background-color: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }
        .status-error {
            background-color: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔌 Modbus RS485 Interface</h1>
        
        <div class="form-group">
            <label for="slaveAddr">Slave Address (decimal):</label>
            <input type="number" id="slaveAddr" min="1" max="247" value="1" placeholder="1-247">
        </div>
        
        <div class="form-group">
            <label for="hexString">Modbus Command (hex, no spaces):</label>
            <input type="text" id="hexString" placeholder="030000000AC5CD" 
                   pattern="[0-9A-Fa-f]*" title="Hex characters only">
        </div>
        
        <button onclick="sendCommand()">Send Command</button>
        
        <div id="status"></div>
        
        <h2>Communication History</h2>
        <div id="history">
            <div style="color: #888;">Waiting for commands...</div>
        </div>
    </div>

    <script>
        // Poll for history updates every 2 seconds
        setInterval(updateHistory, 2000);
        
        function sendCommand() {
            const slaveAddr = document.getElementById('slaveAddr').value;
            const hexString = document.getElementById('hexString').value.trim();
            
            // Validate inputs
            if (!slaveAddr || slaveAddr < 1 || slaveAddr > 247) {
                showStatus('Please enter a valid slave address (1-247)', false);
                return;
            }
            
            if (!hexString || !/^[0-9A-Fa-f]+$/.test(hexString)) {
                showStatus('Please enter a valid hex string', false);
                return;
            }
            
            if (hexString.length % 2 !== 0) {
                showStatus('Hex string must have even number of characters', false);
                return;
            }
            
            // Send POST request
            fetch('/send', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: `slave=${slaveAddr}&hex=${hexString}`
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    showStatus('Command sent successfully!', true);
                    updateHistory();
                } else {
                    showStatus('Error: ' + data.message, false);
                }
            })
            .catch(error => {
                showStatus('Communication error: ' + error, false);
            });
        }
        
        function updateHistory() {
            fetch('/history')
            .then(response => response.json())
            .then(data => {
                const historyDiv = document.getElementById('history');
                if (data.history && data.history.length > 0) {
                    historyDiv.innerHTML = data.history.map(item => 
                        `<div class="history-item ${item.type}">${item.text}</div>`
                    ).join('');
                    // Auto-scroll to bottom
                    historyDiv.scrollTop = historyDiv.scrollHeight;
                }
            })
            .catch(error => {
                console.error('Error fetching history:', error);
            });
        }
        
        function showStatus(message, isSuccess) {
            const statusDiv = document.getElementById('status');
            statusDiv.textContent = message;
            statusDiv.className = isSuccess ? 'status-success' : 'status-error';
            statusDiv.style.display = 'block';
            
            setTimeout(() => {
                statusDiv.style.display = 'none';
            }, 5000);
        }
        
        // Allow Enter key to send
        document.getElementById('hexString').addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                sendCommand();
            }
        });
        
        // Initial history load
        updateHistory();
    </script>
</body>
</html>
)rawliteral";
  
  server.send(200, "text/html", html);
}

// Handle command sending
void handleSend() {
  if (!server.hasArg("slave") || !server.hasArg("hex")) {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Missing parameters\"}");
    return;
  }
  
  int slaveAddr = server.arg("slave").toInt();
  String hexString = server.arg("hex");
  
  // Validate slave address
  if (slaveAddr < 1 || slaveAddr > 247) {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid slave address\"}");
    return;
  }
  
  // Convert hex string to bytes
  int len = hexString.length() / 2;
  uint8_t data[256];
  
  for (int i = 0; i < len; i++) {
    String byteStr = hexString.substring(i * 2, i * 2 + 2);
    data[i] = (uint8_t)strtol(byteStr.c_str(), NULL, 16);
  }
  
  // Add to history - sent
  String sentStr = "SENT to [" + String(slaveAddr) + "]: ";
  for (int i = 0; i < len; i++) {
    if (data[i] < 0x10) sentStr += "0";
    sentStr += String(data[i], HEX);
    sentStr += " ";
  }
  addToHistory(sentStr, "sent");
  
  // Enable transmit mode
  digitalWrite(RS485_DE, HIGH);
  delay(10);
  
  // Send data
  RS485Serial.write(data, len);
  RS485Serial.flush();
  
  // Switch to receive mode
  delay(10);
  digitalWrite(RS485_DE, LOW);
  
  // Wait for response (timeout 1000ms)
  unsigned long startTime = millis();
  String receivedStr = "";
  int receivedCount = 0;
  
  while (millis() - startTime < 1000) {
    if (RS485Serial.available()) {
      uint8_t b = RS485Serial.read();
      if (receivedCount == 0) {
        receivedStr = "RECV from [" + String(slaveAddr) + "]: ";
      }
      if (b < 0x10) receivedStr += "0";
      receivedStr += String(b, HEX);
      receivedStr += " ";
      receivedCount++;
      startTime = millis(); // Reset timeout on each byte
    }
  }
  
  if (receivedCount > 0) {
    addToHistory(receivedStr, "received");
  } else {
    addToHistory("No response from slave " + String(slaveAddr), "error");
  }
  
  server.send(200, "application/json", "{\"success\":true}");
}

// Handle history request
void handleHistory() {
  String json = "{\"history\":[";
  
  bool first = true;
  for (int i = 0; i < MAX_HISTORY; i++) {
    int idx = (historyIndex + i) % MAX_HISTORY;
    if (commHistory[idx].length() > 0) {
      if (!first) json += ",";
      
      String type = "sent";
      if (commHistory[idx].startsWith("RECV")) type = "received";
      else if (commHistory[idx].startsWith("No response")) type = "error";
      
      json += "{\"text\":\"" + commHistory[idx] + "\",\"type\":\"" + type + "\"}";
      first = false;
    }
  }
  
  json += "]}";
  server.send(200, "application/json", json);
}

// Add message to circular history buffer
void addToHistory(String message, String type) {
  commHistory[historyIndex] = message;
  historyIndex = (historyIndex + 1) % MAX_HISTORY;
  
  Serial.println(message);
}
