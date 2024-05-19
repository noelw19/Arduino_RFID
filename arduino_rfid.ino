/*
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read data from a PICC to serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 * 
 * Example sketch/program showing how to read data from a PICC (that is: a RFID Tag or Card) using a MFRC522 based RFID
 * Reader on the Arduino SPI interface.
 * 
 * When the Arduino and the MFRC522 module are connected (see the pin layout below), load this sketch into Arduino IDE
 * then verify/compile and upload it. To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M). When
 * you present a PICC (that is: a RFID Tag or Card) at reading distance of the MFRC522 Reader/PCD, the serial output
 * will show the ID/UID, type and any data blocks it can read. Note: you may see "Timeout in communication" messages
 * when removing the PICC from reading distance too early.
 * 
 * If your reader supports it, this sketch/program will read all the PICCs presented (that is: multiple tag reading).
 * So if you stack two or more PICCs on top of each other and present them to the reader, it will first output all
 * details of the first and then the next PICC. Note that this may take some time as all data blocks are dumped, so
 * keep the PICCs at reading distance until complete.
 * 
 * @license Released into the public domain.
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
 */

  // RFID pins
  // vcc : 3.3
  // rst : 9
  // gnd : gnd
  // mios : 12
  // mosi : 11
  // sck : 13
  // nss : 10

  // btn : 2

  // rbg lights
  // r : 6
  // g : 5
  // b : 4

  // if permission denied on port 
  // sudo chmod a+rw /dev/ttyACM0  



#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

#define NR_KNOWN_KEYS   8
// Known keys, see: https://code.google.com/p/mfcuk/wiki/MifareClassicDefaultKeys
byte knownKeys[NR_KNOWN_KEYS][MFRC522::MF_KEY_SIZE] =  {
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, // FF FF FF FF FF FF = factory default
    {0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5}, // A0 A1 A2 A3 A4 A5
    {0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5}, // B0 B1 B2 B3 B4 B5
    {0x4d, 0x3a, 0x99, 0xc3, 0x51, 0xdd}, // 4D 3A 99 C3 51 DD
    {0x1a, 0x98, 0x2c, 0x7e, 0x45, 0x9a}, // 1A 98 2C 7E 45 9A
    {0xd3, 0xf7, 0xd3, 0xf7, 0xd3, 0xf7}, // D3 F7 D3 F7 D3 F7
    {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}, // AA BB CC DD EE FF
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // 00 00 00 00 00 00
};

const int redPin = 6;
const int greenPin = 5;
const int bluePin = 4;

// 0 = auth, 1 = read, 2= write
int opState = 0;
const int BUTTON_PIN = 2;
int lastState = HIGH;
int currentState;  

int block = 2;
 
void setup() {
  Serial.begin(9600);

  // initialize the pushbutton pin as an pull-up input
  // the pull-up input pin will be HIGH when the switch is open and LOW when the switch is closed.
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	SPI.begin();			// Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522
	delay(4);				// Optional delay. Some board do need more time after init to be ready, see Readme
	mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
	Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

int hex_to_int(char c){
        int first = c / 16 - 3;
        int second = c % 16;
        int result = first*10 + second;
        if(result > 9) result--;
        return result;
}

int hex_to_ascii(char c, char d){
        int high = hex_to_int(c) * 16;
        int low = hex_to_int(d);
        return high+low;
}

/*
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i]);
    }
    
}

void readRfidData(byte *buffer, byte bufferSize) {

    for (byte i = 0; i < bufferSize; i++) {
        Serial.write(buffer[i]);
    }
    
}

bool blockHasData(byte *buffer, byte bufferSize) {
    
    for (byte i = 0; i < bufferSize; i++) {
        if(buffer[i]) {
          return true;
        } 
    }
    return false;
}
 
void loop() {

  currentState = digitalRead(BUTTON_PIN);
  buttonFunctions();

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
    lightOff();
		return;
	}

	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
    lightOff();
		return;
	}
  
  yellowLight();
  Serial.println("Reading");

  if(opState == 0) {
    Serial.println("authenticate");
    Authenticate();

  } else if (opState == 1) {
    // read
    Serial.println("Reading Memory blocks.");
    Serial.println();

    for(byte s = 1; s <= 65; s += 4) {
      if(s > 65) {
        break;
      }
      readCard(3, s);
    } 
    mfrc522.PICC_HaltA();       // Halt PICC
    mfrc522.PCD_StopCrypto1(); 
    Serial.println();
    Serial.println("Finished reading RFID memory blocks.");

  } else if (opState == 2) {
    // write
    Serial.println("write");
  } else if (opState == 3) {
    // write
    Serial.println("dump");
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    // Dump debug info about the card; PICC_HaltA() is automatically called
	  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
    
  }
  

  lightOff();
}

void readCard(byte sector, byte block) {
  MFRC522::MIFARE_Key key;
    for (byte k = 0; k < NR_KNOWN_KEYS; k++) {
        // Copy the known key into the MIFARE_Key structure
        for (byte i = 0; i < MFRC522::MF_KEY_SIZE; i++) {
            key.keyByte[i] = knownKeys[k][i];
        }
        // Try the key
        if (try_key(&key, sector, block)) {
            // Found and reported on the key and block,
            // no need to try other keys for this PICC
            Serial.println("success");
            // readCardWithKey(&key);
            break;
        } 
        
        // http://arduino.stackexchange.com/a/14316
        if ( ! mfrc522.PICC_IsNewCardPresent())
            break;
        if ( ! mfrc522.PICC_ReadCardSerial())
            break;
    }
}

bool try_key(MFRC522::MIFARE_Key *key, byte sector, byte block)
{
  
    bool result = false;
    byte buffer[18];
    MFRC522::StatusCode status;
    // Serial.println(F("Authenticating using key A..."));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        // Serial.print(F("PCD_Authenticate() failed: "));
        // Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }

    // Read block
    byte byteCount = sizeof(buffer);
    status = mfrc522.MIFARE_Read(block, buffer, &byteCount);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    else {
        // Successful read
        if(blockHasData(buffer, 16)) {

          result = true;
          Serial.print(F("Success with key:"));
          dump_byte_array((*key).keyByte, MFRC522::MF_KEY_SIZE);
          Serial.println();
          // Dump block data
          Serial.print(F("Data in Block ")); Serial.print(block); Serial.print(F(":"));
          readRfidData(buffer, 16);
          Serial.println();
        }
    }

     // Stop encryption on PCD
    return result;
}

void buttonFunctions() {
  if(lastState == HIGH && currentState == LOW) {
    // on single press
    pinkLight();
    Serial.println("Button Clicked");
    Serial.println(opState);

    // opStates
    // read
    // write
    // authenticate
    if(opState == 0) {
      Serial.println("Current Operation: Read");
      opState = 1;
    } else if (opState == 1) {
      Serial.println("Current Operation: Write");
      opState = 2;
    } else if (opState == 2) {
      Serial.println("Current Operation: Dump");
      opState = 3;
    } else if (opState == 3) {
      Serial.println("Current Operation: Authenticate");
      opState = 0;
    } else {
      Serial.println("Current Operation: Authenticate");
      opState = 0;
    }

    delay(400);
  }
  lastState = currentState;
}

// authentication
void Authenticate() {
    //Show UID on serial monitor
  Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  // 63 10 0F 1F
  // 04 49 30 D2 4C 75 80
  if (content.substring(1) == "63 10 0F 1F") //change here the UID of the card/cards that you want to give access
  {
    greenLight();
    Serial.println("Authorized access");
    Serial.println();
    delay(3000);
  }
 
 else   {
    redLight();
    Serial.println(" Access denied");
    delay(3000);
  }
}

void led(int r, int g, int b) {
  analogWrite(redPin, r);
  analogWrite(greenPin, g);
  analogWrite(bluePin, b);
}

void greenLight() {
  led(0, 255, 0);
}

void redLight() {
  led(255, 0, 0);
}

void yellowLight() {
  led(255, 255, 0);
}

void pinkLight() {
  led(255, 192, 160);
}

void lightOff() {
  led(0, 0, 0);
}
