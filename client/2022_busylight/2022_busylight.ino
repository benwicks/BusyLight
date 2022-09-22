/*
  Repeating Web client

 Circuit:
 * LCD display attached to pins 4-9
 * Ethernet shield attached to pins 10-13
 */
#include <LiquidCrystal.h>
#include <SPI.h>
#include <Ethernet.h>

#define LED_RED_PIN 2
#define LED_GREEN_PIN 3

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 1, 17);
IPAddress myDns(192, 168, 1, 39);

EthernetClient client;

IPAddress server(192,168,1,40);

const int rs = 4, en = 5, d4 = 6, d5 = 7, d6 = 8, d7 = 9;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

bool isNewMessage = false;
bool shouldPrint = false;
int columnNumber = 0;
int rowNumber = 0;

unsigned long lastConnectionTime = 0;           // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10*1000;  // delay between updates, in milliseconds

void setup() {
//  Serial.begin(9600);
//  while (!Serial) {
//    ; // wait for serial port to connect. Needed for native USB port only
//  }

  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);

  digitalWrite(LED_RED_PIN, HIGH);

  lcd.begin(20, 4);
  lcd.setCursor(0, 0);
  // start the Ethernet connection:
  lcd.print("Init Ethernet");
  if (Ethernet.begin(mac) == 0) {
//    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      lcd.clear();
      lcd.print("Eth shield not found");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      lcd.clear();
      lcd.print("Eth disconnected");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
//    Serial.print("My IP address: ");
//    Serial.println(Ethernet.localIP());
  } else {
    digitalWrite(LED_GREEN_PIN, HIGH);
//    Serial.print("  DHCP assigned IP ");
//    Serial.println(Ethernet.localIP());
  }
  lcd.clear();
  lcd.print("IP:");
  lcd.setCursor(0, 1);
  lcd.print(Ethernet.localIP());

  // give the Ethernet shield a second to initialize:
  delay(500);

  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, LOW);
}

void loop() {
  if (isNewMessage) {
    lcd.clear();
    isNewMessage = false;
    rowNumber = -1;
    columnNumber = 0;
  }
  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  if (client.available()) {
    char c = client.read();
//    Serial.write(c);

    if (c == '{') {
      shouldPrint = true;
      return;
    } else if (c == '}') {
      shouldPrint = false;
      return;
    }
    if (shouldPrint) {
      if (rowNumber == -1 && columnNumber == 0) {
        if (c == 'R') {
          digitalWrite(LED_RED_PIN, HIGH);
          digitalWrite(LED_GREEN_PIN, LOW);
        } else if (c == 'G') {
          digitalWrite(LED_RED_PIN, LOW);
          digitalWrite(LED_GREEN_PIN, HIGH);
        } else if (c == 'B') {
          digitalWrite(LED_RED_PIN, HIGH);
          digitalWrite(LED_GREEN_PIN, HIGH);
        } else {
          digitalWrite(LED_RED_PIN, LOW);
          digitalWrite(LED_GREEN_PIN, LOW);
        }
        rowNumber = 0;
        return;
      }
      if (c == '\n') {
        rowNumber++;
        columnNumber = 0;
        return;
      }
      if (columnNumber > 19) {
        rowNumber++;
        columnNumber = 0;
        if (rowNumber > 3) {
          shouldPrint = false;
          return;
        }
      }
//      Serial.print("Char: ");
//      Serial.print(c);
//      Serial.print("Row #");
//      Serial.print(rowNumber);
//      Serial.print(", ");
//      Serial.print("Column #");
//      Serial.print(columnNumber);
//      Serial.println(')');
      lcd.setCursor(columnNumber, rowNumber);
      lcd.print(c);
      columnNumber++;
    }
  }

  // if ten seconds have passed since your last connection,
  // then connect again and send data:
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  }
}

// this method makes a HTTP connection to the server:
void httpRequest() {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  shouldPrint = false;

  // if there's a successful connection:
  if (client.connect(server, 80)) {
    isNewMessage = true;
    // send the HTTP GET request:
    client.println("GET /busylight/index.php HTTP/1.1");
    client.println("Host: 192.168.1.40");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  } else {
    // if you couldn't make a connection:
//    Serial.println("connection failed");
  }
}
