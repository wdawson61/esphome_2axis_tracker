#include <WiFi.h>
#include <WebServer.h>
#include <HardwareSerial.h>

// WiFi credentials - CHANGE THESE
const char* ssid = "NG24";
const char* password = "deadbeefba";

// RS485 pins for XIAO ESP32C6 with SeeedStudio RS485 Expansion Board
// Based on actual schematic verification
// Using UART1 (ESP32-C6 supports flexible pin mapping via GPIO Matrix)
// IMPORTANT: XIAO ESP32C6 pin labels don't match GPIO numbers!
// UART pin assignments: TX=GPIO22 (D4), RX=GPIO23 (D5), DE=GPIO2 (D2)

#define RS485_TX D4  // D4 on XIAO ESP32C6 = GPIO22 - Connected to DI (Driver Input) on RS485 chip
#define RS485_RX D5  // D5 on XIAO ESP32C6 = GPIO23 - Connected to RO (Receiver Output) on RS485 chip
#define RS485_DE D2   // D2 on XIAO ESP32C6 = GPIO2 - Connected to DE/RE on RS485 chip (hardware connection)

// User LED on XIAO ESP32C6 (for visual feedback)
#define USER_LED LED_BUILTIN  // Built-in LED

// Alternative pin configurations (commented out)
// ESP32-C3 option:
//#define RS485_RX 20  // GPIO20
//#define RS485_TX 21  // GPIO21
//#define RS485_DE 10  // GPIO10

// ESP32 DevKit V1 option:
// #define RS485_RX 16  // RX2
// #define RS485_TX 17  // TX2
// #define RS485_DE 4   // GPIO4

// Create serial port for RS485 using UART1
// UART0 = USB Serial (debug), UART1 = RS485
HardwareSerial RS485Serial(1);

// Web server on port 80
WebServer server(80);

// Current baud rate
long currentBaudRate = 9600;

// Store communication history
#define MAX_HISTORY 50
String commHistory[MAX_HISTORY];
int historyIndex = 0;

void setup() {
  // Initialize USB debug serial (UART0) at 115200 baud
  // ESP32-C6 USB Serial is on GPIO16 (D+) and GPIO17 (D-) - do NOT remap UART0
  Serial.begin(115200);
  Serial.setDebugOutput(true);  // Enable debug output
  
  // Wait for USB Serial to be ready (important for ESP32-C6)
  // Timeout after 3 seconds to not block if no serial monitor attached
  unsigned long startWait = millis();
  while (!Serial && (millis() - startWait < 3000)) {
    delay(10);
  }
  
  delay(500);  // Additional settling time
  
  // Send initial debug output to confirm Serial is working
  Serial.println();
  Serial.println("=================================");
  Serial.println("XIAO ESP32C6 Modbus Web Interface");
  Serial.println("=================================");
  Serial.println("Board: XIAO ESP32C6");
  Serial.println("RS485: SeeedStudio RS485 Expansion Board");
  Serial.printf("USB Serial Baud: 115200\n");
  Serial.println();
  
  // Initialize RS485 serial on UART1 with custom pin mapping
  // ESP32-C6 UART1: TX=GPIO22 (D4), RX=GPIO23 (D5)
  // Note: setPins() parameters are (RX, TX, RTS, CTS) - must call BEFORE begin()
  // Using -1 for RTS/CTS as we're using DE pin for direction control instead

  Serial.printf("Setting UART1 pins: RX=GPIO%d (D5), TX=GPIO%d (D4)\n", RS485_RX, RS485_TX);

  if (!RS485Serial.setPins(RS485_RX, RS485_TX, -1, -1)) {
    Serial.println("ERROR: Failed to set UART1 pins!");
  } else {
    Serial.println("UART1 pins set successfully");
  }

  RS485Serial.begin(currentBaudRate, SERIAL_8N1,RS485_RX, RS485_TX);
  Serial.printf("UART1 initialized at %ld baud\n", currentBaudRate);
  
  // Configure DE/RE pin (Driver Enable / Receiver Enable)
  // Ensure pin is always driven (never floating)
  pinMode(RS485_DE, OUTPUT);
  digitalWrite(RS485_DE, LOW); // Start in receive mode (LOW = receive, HIGH = transmit)
  
  // Configure User LED for visual feedback
  // NOTE: LED on XIAO ESP32C6 is ACTIVE LOW (LOW=ON, HIGH=OFF)
  pinMode(USER_LED, OUTPUT);
  digitalWrite(USER_LED, HIGH); // LED off initially (HIGH = OFF for active-low LED)
  
  Serial.println("RS485 initialized:");
  Serial.printf("  UART: UART1\n");
  Serial.printf("  TX Pin: D4 = GPIO%d\n", RS485_TX);
  Serial.printf("  RX Pin: D5 = GPIO%d\n", RS485_RX);
  Serial.printf("  DE Pin: D2 = GPIO%d\n", RS485_DE);
  Serial.printf("  Baud Rate: %ld\n", currentBaudRate);
  Serial.println("  USB Debug: UART0 (preserved)");
  
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
  server.on("/baudrate", HTTP_GET, handleGetBaudRate);
  server.on("/baudrate", HTTP_POST, handleSetBaudRate);
  
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();
}

// Calculate Modbus CRC16
uint16_t calculateCRC16(uint8_t *data, int length) {
  uint16_t crc = 0xFFFF;
  
  for (int i = 0; i < length; i++) {
    crc ^= (uint16_t)data[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 0x0001) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  
  return crc;
}

// Serve the main HTML page
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Tracker Modbus Tester Widget</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
            background-color: #f0f0f0;
            color: #333;
            transition: background-color 0.3s, color 0.3s;
        }
        body.dark-mode {
            background-color: #1e1e1e;
            color: #e0e0e0;
        }
        .container {
            background-color: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            transition: background-color 0.3s, color 0.3s;
        }
        body.dark-mode .container {
            background-color: #2d2d2d;
            color: #e0e0e0;
            box-shadow: 0 2px 4px rgba(0,0,0,0.3);
        }
        .dark-mode-toggle {
            position: absolute;
            top: 20px;
            right: 20px;
            background-color: #4CAF50;
            color: white;
            padding: 8px 16px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 14px;
        }
        .dark-mode-toggle:hover {
            background-color: #45a049;
        }
        h1 {
            color: #333;
            text-align: center;
        }
        body.dark-mode h1 {
            color: #e0e0e0;
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
        input[type="text"], input[type="number"], select {
            width: 100%;
            padding: 8px;
            border: 1px solid #ddd;
            border-radius: 4px;
            box-sizing: border-box;
            font-family: monospace;
            background-color: white;
            color: #333;
            transition: background-color 0.3s, color 0.3s;
        }
        body.dark-mode input[type="text"], body.dark-mode input[type="number"], body.dark-mode select {
            background-color: #3d3d3d;
            color: #e0e0e0;
            border: 1px solid #555;
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
            transition: background-color 0.3s, color 0.3s;
        }
        body.dark-mode #history {
            background-color: #3d3d3d;
            border: 1px solid #555;
            color: #e0e0e0;
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
        body.dark-mode .status-success {
            background-color: #1e5631;
            color: #90ee90;
        }
        body.dark-mode .status-error {
            background-color: #5a2c2c;
            color: #ff9999;
        }
    </style>
</head>
<body>
    <button class="dark-mode-toggle" onclick="toggleDarkMode()">Dark Mode</button>
    <div class="container">
        <h1>Modbus RS485 Test Widget</h1>
        <p style="text-align: center; color: #888; font-size: 14px; margin-top: -10px;">
            XIAO ESP32C6 + SeeedStudio RS485
        </p>
        
        <div class="form-group">
            <label for="baudRate">Baud Rate:</label>
            <select id="baudRate" onchange="changeBaudRate()">
                <option value="1200">1200</option>
                <option value="2400">2400</option>
                <option value="4800">4800</option>
                <option value="9600" selected>9600</option>
                <option value="19200">19200</option>
                <option value="38400">38400</option>
                <option value="57600">57600</option>
                <option value="115200">115200</option>
            </select>
        </div>
        
        <div class="form-group">
            <label for="slaveAddr">Slave Address (decimal):</label>
            <input type="number" id="slaveAddr" min="1" max="247" value="1" placeholder="1-247">
        </div>
        
        <div class="form-group">
            <label for="hexString">Modbus Command (hex, no spaces):</label>
            <input type="text" id="hexString" placeholder="03000A0001" 
                   pattern="[0-9A-Fa-f]*" title="Hex characters only">
            <small style="color: #888; display: block; margin-top: 5px;">
                Enter function code and data only (no slave address or CRC). Example: 03000A0001 = Read 1 register at 0x000A
            </small>
        </div>
        
        <div class="form-group" style="display: flex; align-items: center; margin-bottom: 15px;">
            <input type="checkbox" id="autoRepeat" style="width: auto; margin-right: 10px;">
            <label for="autoRepeat" style="margin: 0; color: #888;">Auto-repeat every 2 seconds (for debugging)</label>
        </div>
        
        <button onclick="sendCommand()">Send Command</button>
        <div id="repeatStatus" style="margin-top: 10px; font-size: 14px; color: #ff6600; font-weight: bold; display: none;">
            Auto-repeating: <span id="repeatCount">0</span> times
        </div>
        
        <div id="status"></div>
        
        <h2>Communication History</h2>
        <div id="history">
            <div style="color: #888;">Waiting for commands...</div>
        </div>
    </div>

    <script>
        // Auto-repeat state
        let autoRepeatInterval = null;
        let autoRepeatCount = 0;
        let lastCommand = { slave: null, hex: null };
        
        // Poll for history updates every 2 seconds
        setInterval(updateHistory, 2000);
        
        // Initialize dark mode from localStorage
        if (localStorage.getItem('darkMode') === 'true') {
            document.body.classList.add('dark-mode');
            updateDarkModeButton();
        }
        
        // Load current baud rate on page load
        fetch('/baudrate')
        .then(response => response.json())
        .then(data => {
            document.getElementById('baudRate').value = data.baudRate;
        })
        .catch(error => {
            console.error('Error loading baud rate:', error);
        });
        
        // Handle auto-repeat checkbox change
        document.getElementById('autoRepeat').addEventListener('change', function() {
            if (this.checked) {
                // Validate that we have a last command to repeat
                if (!lastCommand.slave || !lastCommand.hex) {
                    showStatus('Send a command first before enabling auto-repeat', false);
                    this.checked = false;
                    return;
                }
                startAutoRepeat();
            } else {
                stopAutoRepeat();
            }
        });
        
        function changeBaudRate() {
            const baudRate = document.getElementById('baudRate').value;
            
            fetch('/baudrate', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: `baudRate=${baudRate}`
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    showStatus(`Baud rate changed to ${baudRate}`, true);
                } else {
                    showStatus('Error changing baud rate: ' + data.message, false);
                }
            })
            .catch(error => {
                showStatus('Communication error: ' + error, false);
            });
        }
        
        function startAutoRepeat() {
            autoRepeatCount = 0;
            const repeatStatus = document.getElementById('repeatStatus');
            repeatStatus.style.display = 'block';
            
            autoRepeatInterval = setInterval(() => {
                autoRepeatCount++;
                document.getElementById('repeatCount').textContent = autoRepeatCount;
                
                // Send the last command
                fetch('/send', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/x-www-form-urlencoded',
                    },
                    body: `slave=${lastCommand.slave}&hex=${lastCommand.hex}`
                })
                .then(response => response.json())
                .then(data => {
                    if (data.success) {
                        updateHistory();
                    }
                })
                .catch(error => {
                    console.error('Auto-repeat error:', error);
                });
            }, 2000);
        }
        
        function stopAutoRepeat() {
            if (autoRepeatInterval) {
                clearInterval(autoRepeatInterval);
                autoRepeatInterval = null;
            }
            document.getElementById('repeatStatus').style.display = 'none';
            autoRepeatCount = 0;
        }
        
        function toggleDarkMode() {
            document.body.classList.toggle('dark-mode');
            const isDarkMode = document.body.classList.contains('dark-mode');
            localStorage.setItem('darkMode', isDarkMode);
            updateDarkModeButton();
        }
        
        function updateDarkModeButton() {
            const btn = document.querySelector('.dark-mode-toggle');
            if (document.body.classList.contains('dark-mode')) {
                btn.textContent = 'Light Mode';
            } else {
                btn.textContent = 'Dark Mode';
            }
        }
        
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
            
            // Store last command for auto-repeat
            lastCommand = { slave: slaveAddr, hex: hexString };
            
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
  
  // Convert hex string to bytes (this becomes the payload after slave address)
  int payloadLen = hexString.length() / 2;
  uint8_t modbusFrame[258];  // Slave addr + payload + 2 byte CRC
  
  // Insert slave address as first byte
  modbusFrame[0] = (uint8_t)slaveAddr;
  
  // Parse hex string into the frame starting at position 1
  for (int i = 0; i < payloadLen; i++) {
    String byteStr = hexString.substring(i * 2, i * 2 + 2);
    modbusFrame[i + 1] = (uint8_t)strtol(byteStr.c_str(), NULL, 16);
  }
  
  // Calculate and append CRC16
  int frameLen = payloadLen + 1;  // slave address + payload
  uint16_t crc = calculateCRC16(modbusFrame, frameLen);
  modbusFrame[frameLen] = crc & 0xFF;        // CRC Low byte
  modbusFrame[frameLen + 1] = (crc >> 8) & 0xFF;  // CRC High byte
  frameLen += 2;  // Total length including CRC
  
  // Add to history - sent
  String sentStr = "SENT to [" + String(slaveAddr) + "]: ";
  for (int i = 0; i < frameLen; i++) {
    if (modbusFrame[i] < 0x10) sentStr += "0";
    sentStr += String(modbusFrame[i], HEX);
    sentStr += " ";
  }
  addToHistory(sentStr, "sent");
  
  // Enable transmit mode and turn on LED
  digitalWrite(RS485_DE, HIGH);  // Assert DE (transmit mode)
  digitalWrite(USER_LED, LOW);   // LED on during transmission (active LOW)
  delayMicroseconds(50);  // Minimal delay for DE to stabilize (50 microseconds)

  // Send data
  RS485Serial.write(modbusFrame, frameLen);
  RS485Serial.flush();  // Wait for all bytes to be transmitted

  // Switch to receive mode IMMEDIATELY after transmission completes
  // Critical: No delay here! RS485 slave may respond within microseconds
  digitalWrite(RS485_DE, LOW);   // De-assert DE (receive mode)
  digitalWrite(USER_LED, HIGH);  // LED off during reception (active LOW)

  // Clear any echo or garbage in RX buffer from TX mode
  int clearedBytes = 0;
  while (RS485Serial.available()) {
    RS485Serial.read();
    clearedBytes++;
  }
  if (clearedBytes > 0) {
    Serial.printf("Cleared %d bytes from RX buffer after TX\n", clearedBytes);
  }
  delay(10); // Longer delay after clearing buffer

  // Wait for response with proper inter-byte timeout
  // Overall timeout: 1000ms, inter-byte timeout: 200ms (very conservative)
  unsigned long startTime = millis();
  unsigned long lastByteTime = millis();
  uint8_t responseBuffer[256];
  int receivedCount = 0;
  bool firstByteReceived = false;

  while (millis() - startTime < 1000) {
    if (RS485Serial.available()) {
      responseBuffer[receivedCount] = RS485Serial.read();
      unsigned long byteTime = millis() - startTime;
      Serial.printf("RX byte %d: 0x%02X at %lums\n", receivedCount, responseBuffer[receivedCount], byteTime);
      receivedCount++;
      lastByteTime = millis(); // Track last byte received time
      firstByteReceived = true;
      if (receivedCount >= 256) break; // Prevent buffer overflow
    } else if (firstByteReceived) {
      // Check inter-byte timeout only after first byte received
      // Increased to 200ms to be very conservative
      unsigned long gapTime = millis() - lastByteTime;
      if (gapTime > 200) {
        Serial.printf("Inter-byte timeout after %lums gap\n", gapTime);
        break; // Message complete - no more bytes coming
      }
    }
  }
  
  if (receivedCount > 0) {
    String receivedStr = "RECV from [" + String(slaveAddr) + "] (" + String(receivedCount) + " bytes): ";

    // Check if this is a Function 03/04 response for special formatting
    bool isRegisterResponse = false;
    uint8_t byteCount = 0;
    if (receivedCount >= 5 && (responseBuffer[1] == 0x03 || responseBuffer[1] == 0x04)) {
      byteCount = responseBuffer[2];
      if ((3 + byteCount + 2) == receivedCount) {
        isRegisterResponse = true;
      }
    }

    // Display raw hex bytes with grouped formatting for register responses
    for (int i = 0; i < receivedCount; i++) {
      // For register responses, group the data bytes as 16-bit values
      if (isRegisterResponse && i >= 3 && i < (3 + byteCount)) {
        // We're in the data section
        int dataIndex = i - 3;
        if (dataIndex % 2 == 0) {
          // Start of a 16-bit register (high byte)
          if (responseBuffer[i] < 0x10) receivedStr += "0";
          receivedStr += String(responseBuffer[i], HEX);
          // Add low byte immediately
          if (responseBuffer[i + 1] < 0x10) receivedStr += "0";
          receivedStr += String(responseBuffer[i + 1], HEX);
          receivedStr += " ";
          i++; // Skip the low byte since we already processed it
        }
      } else if (isRegisterResponse && i == (receivedCount - 2)) {
        // CRC bytes - group them together
        if (responseBuffer[i] < 0x10) receivedStr += "0";
        receivedStr += String(responseBuffer[i], HEX);
        if (responseBuffer[i + 1] < 0x10) receivedStr += "0";
        receivedStr += String(responseBuffer[i + 1], HEX);
        i++; // Skip the second CRC byte
      } else {
        // Regular byte display (slave addr, function code, byte count)
        if (responseBuffer[i] < 0x10) receivedStr += "0";
        receivedStr += String(responseBuffer[i], HEX);
        receivedStr += " ";
      }
    }

    // Validate CRC if we have at least 4 bytes (slave + func + at least 1 data byte + 2 CRC bytes)
    if (receivedCount >= 4) {
      uint16_t receivedCRC = responseBuffer[receivedCount - 2] | (responseBuffer[receivedCount - 1] << 8);
      uint16_t calculatedCRC = calculateCRC16(responseBuffer, receivedCount - 2);

      if (receivedCRC == calculatedCRC) {
        receivedStr += "✓ CRC OK";
      } else {
        receivedStr += "✗ CRC ERROR (expected: " + String(calculatedCRC, HEX) + ", got: " + String(receivedCRC, HEX) + ")";
      }
    } else {
      receivedStr += "✗ Response too short (min 4 bytes)";
    }

    addToHistory(receivedStr, "received");
  } else {
    addToHistory("No response from slave " + String(slaveAddr) + " (timeout)", "error");
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

// Handle get baud rate request
void handleGetBaudRate() {
  String json = "{\"baudRate\":" + String(currentBaudRate) + "}";
  server.send(200, "application/json", json);
}

// Handle set baud rate request
void handleSetBaudRate() {
  if (!server.hasArg("baudRate")) {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Missing baudRate parameter\"}");
    return;
  }
  
  long newBaudRate = server.arg("baudRate").toInt();
  
  // Validate baud rate
  if (newBaudRate < 300 || newBaudRate > 115200) {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid baud rate\"}");
    return;
  }
  
  // Reinitialize serial with new baud rate
  RS485Serial.end();
  delay(100);
  RS485Serial.setPins(RS485_RX, RS485_TX, -1, -1);  // Must set pins before begin()
  RS485Serial.begin(newBaudRate, SERIAL_8N1);
  currentBaudRate = newBaudRate;
  
  String message = "Baud rate changed to " + String(newBaudRate);
  addToHistory(message, "sent");
  
  server.send(200, "application/json", "{\"success\":true}");
}
