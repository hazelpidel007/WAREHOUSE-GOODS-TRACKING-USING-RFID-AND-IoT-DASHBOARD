
# Warehouse Goods Tracking Using RFID and IoT Dashboard

A low-cost RFID and IoT warehouse-tracking prototype that captures goods movement, validates RFID tags against a cloud inventory, records authorized transactions, flags unauthorized scans, and supports replenishment decisions through a web dashboard.

**Built with:** Arduino Uno, MFRC522 RFID Reader, ESP8266 Wi-Fi Module, AWS API Gateway, AWS Lambda, and Amazon DynamoDB.

---

## 📖 About the Project

Warehouse inventory operations can be affected by manual-entry errors, delayed stock updates, labour overhead, inventory shrinkage, and unexpected stockouts. Paper-based manifests and optical barcode scanners require manual handling and line-of-sight alignment, which can slow down item movement and make inventory discrepancies harder to identify.

This project explores a contactless alternative using **Radio Frequency Identification (RFID)**. A passive RFID tag is read by an MFRC522 reader operating at **13.56 MHz**. The Arduino Uno extracts the tag's unique identifier (UID), applies a duplicate-scan suppression interval, and forwards the scan details through an ESP8266 Wi-Fi module to an HTTPS endpoint hosted on AWS.

The cloud-side scan handler checks the device key and verifies whether the scanned tag is authorized. Authorized scans update the inventory record and create a transaction entry. Invalid or unauthorized scans are recorded in an alerts table without changing stock.

A browser-based dashboard is intended to present inventory levels, recent alerts, risk categories, and replenishment recommendations.

---

## 🎯 Project Objectives

1. Reduce manual inventory-entry effort through RFID-based identification.
2. Record incoming and outgoing stock movement in a cloud database.
3. Prevent repeated reads of the same tag from creating duplicate transactions within the configured debounce interval.
4. Validate scanned tags before allowing inventory updates.
5. Maintain a record of unauthorized scan attempts.
6. Provide immediate local feedback through an LCD and buzzer.
7. Present inventory and alert information through a web dashboard.
8. Demonstrate how inventory history can be used to estimate demand and replenishment requirements.

---

## ✨ Key Features

- ✅ **Contactless RFID Identification:** Reads passive MFRC522-compatible tags at 13.56 MHz.
- ✅ **Duplicate-Scan Suppression:** Ignores a repeated UID within a 3,000 ms debounce window.
- ✅ **Wi-Fi Connectivity:** Uses an ESP8266 module controlled through UART AT commands.
- ✅ **HTTPS Communication:** Sends scan requests to an AWS API Gateway endpoint over port 443.
- ✅ **Cloud-Side Authorization:** Checks the device secret and the tag's authorization status.
- ✅ **Inventory Updates:** Changes the stored stock quantity for accepted scan events.
- ✅ **Transaction History:** Records item movement, quantity, timestamp, batch, and device information.
- ✅ **Unauthorized-Scan Logging:** Stores invalid-key and unauthorized-tag events in an alerts table.
- ✅ **LCD and Buzzer Feedback:** Provides immediate accepted/denied scan notifications.
- ✅ **Web Dashboard:** Designed to display stock indicators, hardware status, risk ranking, and recent alerts.
- ✅ **Predictive Inventory Analytics:** Estimates burn rate, days to depletion, reorder point, and suggested replenishment quantity.

---

## 🏗 System Architecture

The project is organized into three logical layers.

```
┌───────────────────────────────────────────────────────────────┐
│                1. EDGE SENSING AND FEEDBACK                    │
│                                                               │
│  RFID Tag (13.56 MHz)                                         │
│          │                                                    │
│          ▼                                                    │
│  MFRC522 Reader ──SPI──> Arduino Uno                          │
│                              │                                │
│                   ┌──────────┴──────────┐                     │
│                   ▼                     ▼                     │
│                16×2 LCD               Buzzer                  │
│                              │                                │
│                              ▼                                │
│                       ESP8266 Wi-Fi                           │
│                       (UART AT commands)                      │
└──────────────────────────────┬────────────────────────────────┘
                               │
                         HTTPS / TLS
                           Port 443
                               │
┌──────────────────────────────▼────────────────────────────────┐
│               2. CLOUD INGESTION AND STORAGE                   │
│                                                               │
│  Amazon API Gateway ──> AWS Lambda ──> Amazon DynamoDB        │
│       REST API           Scan handler       ├── items         │
│                                              ├── transactions  │
│                                              └── alerts        │
└──────────────────────────────┬────────────────────────────────┘
                               │
                       REST API / JSON
                         Dashboard polling
                               │
┌──────────────────────────────▼────────────────────────────────┐
│                3. ANALYTICS AND PRESENTATION                   │
│                                                               │
│  Web Dashboard (HTML5 / CSS3 / JavaScript)                    │
│                                                               │
│  Stock Overview | Risk Ranking | Alerts | Replenishment       │
└───────────────────────────────────────────────────────────────┘
```

### 1. Edge Sensing and Feedback Layer

- MFRC522 reads the UID from a passive RFID tag.
- Arduino Uno processes the UID and handles duplicate-scan suppression.
- LCD displays scan status and the local stock counter.
- Buzzer provides audible feedback.
- ESP8266 provides wireless communication with the cloud.

### 2. Cloud Ingestion and Storage Layer

- Amazon API Gateway exposes the scan endpoint.
- AWS Lambda validates incoming requests.
- Amazon DynamoDB stores inventory, transaction history, and security alerts.

### 3. Analytics and Presentation Layer

- HTML, CSS, and JavaScript provide the dashboard interface.
- Inventory data is presented through stock indicators and risk categories.
- Transaction history can be used to estimate stock depletion and replenishment requirements.

### Edge-to-Cloud Request Sequence

1. RFID tag returns its UID to the MFRC522/Arduino over SPI.
2. Arduino sends `AT+CIPSTART` to ESP8266.
3. ESP8266 performs a TLS handshake on port 443.
4. A scan request containing tag, device, key, and action parameters is sent to API Gateway.
5. Lambda performs `get_item`, followed by stock update and transaction insertion in DynamoDB.
6. The response is returned through ESP8266.
7. LCD and buzzer provide feedback on the edge device.

---

## 🔧 Hardware Requirements

| Component                     | Role                                       |
| ----------------------------- | ------------------------------------------ |
| Arduino Uno R3 (ATmega328P)   | Main controller and scan logic             |
| MFRC522 RFID Reader           | Reads passive RFID tag UIDs at 13.56 MHz   |
| Passive RFID Tags/Cards       | Identifies warehouse items                 |
| ESP8266 Wi-Fi Module          | Wi-Fi and HTTPS communication              |
| 16×2 LCD                      | Displays scan result and stock information |
| Active Buzzer                 | Audible accepted/denied feedback           |
| Jumper Wires and Power Supply | Circuit connections and power              |

> **Important:** The MFRC522 is a 3.3 V device. The ESP8266 also requires a stable regulated 3.3 V supply capable of handling current peaks. Use appropriate logic-level conversion because ESP8266 RX is not 5 V tolerant.

---

## 💻 Software and Cloud Stack

| Layer                | Technology                 | Purpose                                  |
| -------------------- | -------------------------- | ---------------------------------------- |
| Edge Firmware        | Embedded C++ / Arduino IDE | RFID reading and device control          |
| RFID Interface       | MFRC522 Library            | Communication with RFID reader           |
| Display              | LiquidCrystal Library      | LCD output                               |
| Serial Communication | SoftwareSerial             | UART communication with ESP8266          |
| Cloud Compute        | AWS Lambda (Python)        | Scan validation and inventory processing |
| API                  | Amazon API Gateway         | HTTPS endpoint                           |
| Database             | Amazon DynamoDB            | Inventory, transactions, and alerts      |
| Dashboard            | HTML5, CSS3, JavaScript    | Browser-based inventory presentation     |
| Communication        | SPI, UART, HTTPS/TLS       | Device and cloud communication           |

### Arduino Libraries

Install the following libraries through Arduino IDE:

- `MFRC522`
- `LiquidCrystal`
- `SoftwareSerial`
- `SPI`

The AWS Lambda function uses `boto3`, available in the standard AWS Lambda Python runtime environment.

---

## 📌 Pin Mapping

The firmware defines the following connections:

| Device Signal                              | Arduino Uno Pin |
| ------------------------------------------ | --------------- |
| MFRC522 SS / SDA                           | D10             |
| MFRC522 SCK                                | D13             |
| MFRC522 MOSI                               | D11             |
| MFRC522 MISO                               | D12             |
| MFRC522 RST                                | D9              |
| LCD RS                                     | D2              |
| LCD EN                                     | D3              |
| LCD D4                                     | D4              |
| LCD D5                                     | D5              |
| LCD D6                                     | D6              |
| LCD D7                                     | D7              |
| Buzzer                                     | D8              |
| ESP8266 Serial RX / TX (original firmware) | D11 / D12       |

> **Hardware note:** The original firmware assigns D11 and D12 to both MFRC522 SPI and ESP8266 SoftwareSerial. This creates a pin conflict on the Arduino Uno. Reassign ESP8266 serial communication to two unused digital pins and update the firmware and wiring accordingly before hardware testing.

---

## ⚙️ Working Principle

### 1. RFID Identification

A passive RFID tag is presented to the MFRC522 reader. The reader obtains the tag UID and transfers it to Arduino Uno through SPI.

### 2. UID Processing and Duplicate Suppression

Arduino converts the UID bytes into a hexadecimal string and converts it to uppercase.

The firmware stores the previous UID and scan timestamp.

If the same UID is scanned again within the configured `DEBOUNCE_DELAY_MS` interval of 3,000 ms, the scan is ignored.

### 3. Cloud Request

For a new scan, Arduino creates a request containing:

- `tag` – RFID UID
- `dev` – Device identifier (`GATE-01`)
- `key` – Device secret
- `action` – Movement direction (`IN` by default)

ESP8266 sends the request to the configured AWS API Gateway host through a secure connection.

### 4. Cloud Validation

The Lambda handler checks the supplied device key and retrieves the corresponding tag record from DynamoDB.

- **Invalid or missing key:** An alert is written and an unauthorized response is returned.
- **Missing or unauthorized tag:** An alert is written and stock remains unchanged.
- **Authorized tag:** Inventory quantity is updated and a transaction record is created.

### 5. Local Feedback

Arduino interprets the returned response and updates the LCD and buzzer.

| Event            | Device Feedback             |
| ---------------- | --------------------------- |
| Accepted Scan    | Short beep + `ITEM ADDED!`  |
| Denied Scan      | Longer tone + `UNAUTH TAG!` |
| Waiting for Scan | `SCAN ITEM CARD`            |

The local `totalStock` variable is a running count of successful scans since the device started. It is not a complete copy of the cloud inventory total.

---

## ☁️ AWS Backend and Database Design

The cloud architecture uses three primary AWS services:

- Amazon API Gateway
- AWS Lambda
- Amazon DynamoDB

### API Endpoints

| Endpoint     | Purpose                                                    |
| ------------ | ---------------------------------------------------------- |
| `/scan`      | Receives RFID scan events from the edge device             |
| `/analytics` | Intended to provide dashboard inventory and analytics data |

The current firmware calls the scan resource using the `/prod/scan` path, where `prod` is the API Gateway stage.

The analytics endpoint requires its own deployed backend integration before it can provide live dashboard data.

### DynamoDB Schema

The design uses three DynamoDB tables.

| Table          | Purpose                 | Key Fields                                                                     |
| -------------- | ----------------------- | ------------------------------------------------------------------------------ |
| `items`        | Current inventory state | `tag_id`, `item_id`, `batch_id`, `current_stock`, `authorised`, `last_scan_ts` |
| `transactions` | Movement history        | `item_id`, `timestamp`, `movement`, `quantity`, `batch_id`, `device_id`        |
| `alerts`       | Security events         | `device_id`, `timestamp`, `tag_id`, `reason`                                   |

**Note:** The supplied Python Lambda code uses the attribute `authorised` (British spelling). DynamoDB attribute names are case-sensitive. Use the same spelling in your records or modify the code consistently.

### Lambda Scan Handler Logic

The Lambda handler follows this sequence:

```
# 1. Read query parameters
tag_id = qs.get("tag")
device = qs.get("dev", "GATE-01")
api_key = qs.get("key")
action = (qs.get("action") or "IN").upper()

# 2. Validate the device key and tag
if not tag_id or api_key != SECRET_KEY:
    # Log an alert and reject the request

# 3. Retrieve the item record
resp = items_table.get_item(Key={"tag_id": tag_id})
item = resp.get("Item")

if not item or not item.get("authorised", False):
    # Log an unauthorized-tag alert

# 4. Update stock and record the movement
# Update current_stock and last_scan_ts
# Write a transaction record

# 5. Return the scan result
# Example response: #IN,item-id,1#
```

The supplied Lambda implementation uses a DynamoDB `update_item` operation for stock modification and a separate `put_item` operation for transaction insertion. These are separate writes rather than one atomic database transaction.

---

## 📊 Predictive Inventory Analytics

The analytics module is designed to help identify inventory items that may require replenishment.

### Key Metrics

**1. Daily Burn Rate**

Estimates the average quantity consumed or moved out per day using recent outbound transactions.

**2. Days to Depletion**

Estimates the number of days remaining before stock reaches zero.

`Days to Depletion = Current Stock / Daily Burn Rate`

**3. Reorder Point (ROP)**

Determines the stock level at which replenishment should be initiated.

`ROP = (Average Daily Demand × Supplier Lead Time) + Safety Stock`

**4. Advised Replenishment**

Estimates the quantity required to restore stock to its target level.

`Advised Replenishment = max(0, Target Stock − Current Stock)`

**5. Risk Classification**

Items are categorized into:

- High Risk
- Medium Risk
- Low Risk

Classification is based on projected stock depletion, supplier lead time, and safety-stock conditions.

### Sample Risk Ledger Output

The following table represents illustrative seeded demonstration data.

| Item                  | Stock / Target | Burn (units/day) | Lead (days) | Days to Depletion | Advised Replenishment | Risk   |
| --------------------- | -------------- | ---------------- | ----------- | ----------------- | --------------------- | ------ |
| Lithium Battery Pack  | 120 / 150      | 4.00             | 3           | 30.0              | +30                   | Low    |
| Optical Sensor Module | 41 / 100       | 4.79             | 7           | 8.6               | +59                   | Medium |
| Microcontroller Unit  | 18 / 80        | 6.79             | 4           | 2.7               | +62                   | High   |

> ⚠️ **Data Note:** These analytics were described as being exercised using seeded demonstration transactions. They should not be interpreted as long-term warehouse operating data or a validated forecast.

---

## 🖥 Web Dashboard

The dashboard layer is intended to provide a browser-based overview of warehouse inventory and scan activity using HTML, CSS, and JavaScript.

### Dashboard Components

- 📦 **Stock Overview:** Current stock quantities and item-level indicators.
- 🔌 **Hardware Diagnostics:** Device connectivity and scan-system status.
- ⚠️ **Risk Ranking:** Items ordered according to replenishment urgency.
- 🛒 **Requisition Advice:** Suggested replenishment quantities.
- 🚨 **Security Alerts:** Recent unauthorized-tag and invalid-key events.
- 📈 **Recent Activity:** Recorded stock movement transactions.

The described design uses periodic API polling to refresh information without reloading the entire page.

The dashboard requires the corresponding frontend files and a deployed analytics/data endpoint to display live cloud information.

---

## 📈 Results and Validation

### Latency Benchmarks (50 Consecutive Scans)

The following latency figures were supplied for the project:

| Processing Stage                      | Mean Latency (ms) |
| ------------------------------------- | ----------------- |
| RFID Interrogation and UID Extraction | 22                |
| ESP8266 AT Command and TLS Handshake  | 235               |
| WAN Transit (Edge to AWS)             | 108               |
| API Gateway and Lambda                | 36                |
| DynamoDB Write                        | 11                |
| Response Downlink and LCD Update      | 18                |
| **Total End-to-End**                  | **430**           |

The ESP8266 AT-command and TLS-handshake stage is reported as the largest contributor to measured latency.

### Security and Functional Tests

| Test Case                    | Outcome                                  |
| ---------------------------- | ---------------------------------------- |
| Authorized Tag               | Accepted; stock updated                  |
| Unregistered Tag             | Rejected; alert raised; stock unchanged  |
| Invalid Device Key           | Rejected; alert raised                   |
| Repeat Scan Within 3 Seconds | Suppressed; no duplicate stock increment |

**Reported mean round-trip time: 430 ms across 50 consecutive scans.**

> Measurement figures are reported project values. Include the test setup and raw observations in the repository or project report for independent verification.

---

## 🚀 Getting Started

### Prerequisites

- Arduino Uno R3
- MFRC522 RFID Reader and compatible passive tags
- ESP8266 Wi-Fi Module with AT-command firmware
- 16×2 LCD
- Active Buzzer
- Arduino IDE
- MFRC522, LiquidCrystal, and SoftwareSerial libraries
- AWS Account with API Gateway, Lambda, and DynamoDB permissions
- Configured Wi-Fi network
- Deployed API Gateway endpoint

### 1. Clone the Repository

```
git clone https://github.com/<your-username>/warehouse-rfid-iot-tracker.git

cd warehouse-rfid-iot-tracker
```

### 2. Configure the Arduino Firmware

Open:

```
firmware/warehouse_tracker.ino
```

Configure your credentials locally:

```
const String WIFI_SSID = "<YOUR_WIFI_SSID>";
const String WIFI_PASS = "<YOUR_WIFI_PASSWORD>";
const String AWS_HOST = "<YOUR_API_GATEWAY_HOST>";
const String DEVICE_SECRET = "<YOUR_DEVICE_SECRET>";
const String DEVICE_ID = "GATE-01";
```

Verify the ESP8266 serial pin assignments and correct the SPI pin conflict before uploading.

> 🔐 **Never commit real Wi-Fi credentials, API keys, or device secrets to GitHub.**

### 3. Configure DynamoDB

Create the following tables:

```
items
transactions
alerts
```

Configure their partition and sort keys to match the Lambda handler.

Add inventory records containing:

- Tag UID
- Item ID
- Batch ID
- Current stock
- Authorization status
- Last scan timestamp

Ensure the authorization attribute uses the exact spelling expected by the Lambda code.

### 4. Deploy AWS Lambda

Place the Python handler in:

```
cloud/lambda_scan_handler.py
```

Configure the following environment variables:

```
ITEMS_TABLE=items
TRANSACTIONS_TABLE=transactions
ALERTS_TABLE=alerts
API_KEY=<YOUR_DEVICE_SECRET>
```

Provide the Lambda execution role with the required DynamoDB permissions.

Deploy the Lambda function and connect it to API Gateway.

### 5. Configure API Gateway

Create an HTTPS route for the scan resource and connect it to the Lambda function.

For example:

```
/prod/scan
```

Ensure the deployed stage and resource path match the path configured in Arduino firmware.

Configure CORS for browser-based dashboard requests where required.

### 6. Flash the Arduino

1. Open the firmware in Arduino IDE.
2. Select Arduino Uno as the target board.
3. Select the appropriate COM port.
4. Verify all hardware connections.
5. Compile and upload the firmware.
6. Open Serial Monitor at **9600 baud**.
7. Present an authorized RFID tag and observe the LCD, buzzer, and cloud records.

### 7. Launch the Dashboard

Place the dashboard files under:

```
dashboard/
├── index.html
├── styles.css
└── app.js
```

Configure the frontend to use your deployed API endpoint.

Serve the page locally or host it on a web server.

The dashboard requires an implemented analytics/data endpoint and frontend polling logic to display live cloud information.

---

## 📁 Repository Structure

```
warehouse-rfid-iot-tracker/
│
├── firmware/
│   └── warehouse_tracker.ino
│
├── cloud/
│   └── lambda_scan_handler.py
│
├── dashboard/
│   ├── index.html
│   ├── styles.css
│   └── app.js
│
├── docs/
│   ├── ECS2-Final-Project-Report.pdf
│   └── architecture.png
│
├── .gitignore
├── LICENSE
└── README.md
```

Add only files that are actually present in your repository.

---

## 🔐 Security and Deployment Notes

This project is an academic prototype. Before using a similar design in a real warehouse, consider the following:

- Never hard-code production credentials in firmware or commit them to GitHub.
- Avoid transmitting secrets in URL query parameters in production systems.
- Use secure authorization mechanisms and protected request headers.
- Apply least-privilege IAM permissions to the Lambda execution role.
- Store and rotate secrets using a managed secrets solution.
- Validate request methods, tag UIDs, device identity, and movement actions.
- Implement replay protection and idempotency controls.
- Consider DynamoDB transactions for reliable inventory and transaction updates.
- Add error handling for Wi-Fi disconnections, TLS failures, API timeouts, and database errors.
- Do not assume that an RFID UID alone provides cryptographic authentication of a tag.
- Verify all electrical connections and logic-level requirements before powering the hardware.

---

## 🔮 Future Scope

- 📡 **UHF RFID Integration:** Evaluate UHF RFID hardware for longer read ranges and bulk-item identification.
- 📉 **Advanced Demand Forecasting:** Use time-series forecasting to improve demand predictions.
- 🔔 **Real-Time Push Updates:** Explore WebSockets or MQTT instead of periodic dashboard polling.
- 📧 **Automated Supplier Notifications:** Generate purchase requisitions and supplier notifications when stock reaches a configured threshold.
- 🔐 **Enhanced Device Security:** Implement AWS Secrets Manager, KMS, stronger device identity, and key rotation.
- 📊 **Advanced Analytics:** Add item-level trend charts, supplier lead-time tracking, and configurable safety-stock policies.
- ☁️ **Improved Cloud Reliability:** Add offline buffering, retry logic, monitoring, and idempotency handling.

---

## 📚 References

1. K. Finkenzeller, *RFID Handbook: Fundamentals and Applications in Contactless Smart Cards, Radio Frequency Identification and Near-Field Communication*, 3rd ed., Wiley, 2010.
2. L. Atzori, A. Iera, and G. Morabito, "The Internet of Things: A survey," *Computer Networks*, vol. 54, no. 15, pp. 2787–2805, 2010.
3. Amazon Web Services, *AWS Well-Architected Framework — Serverless Applications Lens*.
4. M. Armbrust et al., "A view of cloud computing," *Communications of the ACM*, vol. 53, no. 4, pp. 50–58, 2010.
5. NXP Semiconductors, *MFRC522 Standard 3V and 5V RFID Transceiver IC Product Data Sheet*, Rev. 3.9, 2016.
6. Espressif Systems, *ESP8266EX Datasheet*.
7. E. A. Silver, D. F. Pyke, and D. J. Thomas, *Inventory and Production Management in Supply Chains*, 4th ed., CRC Press, 2017.

---

## 📄 License

This project is released for academic and educational purposes.

See the `LICENSE` file for details. Add a license file to the repository and ensure its terms match how you intend others to use the project.

---

## 🙏 Acknowledgements

- **Project Guide:** Prof. Amgothu Santhi
- **Department of CSE, VIT-AP University**
- **Engineering Clinics Project (ECS2003)** — Academic Year 2026–2027

### Team Members

| Registration No. | Student Name              |
| ---------------- | ------------------------- |
| 24BCD7175        | Chinni Vishnu Vardhan     |
| 24BCE8271        | Vadni Bhuvana Siva Sachin |
| 24BCD7097        | Bura Adithya              |
| 24BCE8386        | B Venkata Sai Reddy       |
| 25BCE7110        | Abheek Guha Ray           |
| 25BCA7127        | Debarpit Samal            |


