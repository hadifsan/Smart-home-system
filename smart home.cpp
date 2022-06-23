#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); 

#define DHTPIN 4
#define DHTTYPE DHT22     // DHT 11

DHT dht(DHTPIN, DHTTYPE);

#include <UniversalTelegramBot.h>
// Library for interacting with the Telegram API
// Search for "Telegegram" in the Library manager and install
// The universal Telegram library
// https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot


#include <ArduinoJson.h>
// Library used for parsing Json from the API responses
// NOTE: There is a breaking change in the 6.x.x version,
// install the 5.x.x version instead
// Search for "Arduino Json" in the Arduino Library manager
// https://github.com/bblanchon/ArduinoJson

//------- Replace the following! ------

char ssid[] = "Hadif";         // your network SSID (name)
char password[] = "12345678"; // your network password
String on_symbol="✅ ";  // ON Indicator
String off_symbol="☑ "; // OFF indicator.
#define TELEGRAM_BOT_TOKEN "1633312161:AAENBw13pcqh89gUS4Aoj429N1xLbldi2dg"


//------- ---------------------- ------


// This is the Wifi client that supports HTTPS
WiFiClientSecure client;
UniversalTelegramBot bot(TELEGRAM_BOT_TOKEN, client);

#define LED_PIN 14 
#define LED_PIN1 25



int delayBetweenChecks = 1000;
unsigned long lastTimeChecked;   //last time messages' scan has been done

unsigned long lightTimerExpires;
boolean lightTimerActive = false;

void setup() {
  Serial.begin(115200);
  dht.begin();

  // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();

  // Set WiFi to station mode and disconnect from an AP if it was Previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  pinMode(14, OUTPUT);
  pinMode(25, OUTPUT);
  digitalWrite(14, LOW);
  digitalWrite(25, LOW);


  // attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  lcd.setCursor(0,0);
  lcd.print("Connecting:");
  lcd.setCursor(11,0);
  lcd.print(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    lcd.setCursor(0,1);
    lcd.print(".........");
    delay(500);
  }

  lcd.clear();
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
   lcd.setCursor(0,0);
  lcd.print("Wifi Connected!");
  delay(500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IP ADDRESS:");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  
  // Only required on 2.5 Beta
  //client.setInsecure();

  
  // longPoll keeps the request to Telegram open for the given amount of seconds if there are no messages
  // This hugely improves response time of the bot, but is only really suitable for projects
  // where the the initial interaction comes from Telegram as the requests will block the loop for
  // the length of the long poll
  bot.longPoll = 60;
}

void handleNewMessages(int numNewMessages) {

  for (int i = 0; i < numNewMessages; i++) {

    // If the type is a "callback_query", a inline keyboard button was pressed
    if (bot.messages[i].type ==  F("callback_query")) {
      String chat_id = String(bot.messages[i].chat_id);
      String text = bot.messages[i].text;
      Serial.print("Call back button pressed with text: ");
      Serial.println(text);


      if (text == F("ON1")) {
        digitalWrite(LED_PIN, HIGH);
        bot.sendMessage(chat_id, "LED 1 TURNED ON");

      } 
        else if (text == F("OFF1")) {
        digitalWrite(LED_PIN, LOW);
        bot.sendMessage(chat_id, "LED 1 TURNED OFF");
        
      }
        else if (text == F("ON2")) {
        digitalWrite(LED_PIN1, HIGH);
        bot.sendMessage(chat_id, "LED 2 TURNED ON");
        
        } else if (text == F("OFF2")) {
        digitalWrite(LED_PIN1, LOW);
        bot.sendMessage(chat_id, "LED 2 TURNED OFF");
        
      } else if (text.startsWith("TIME")) {
        text.replace("TIME", "");
        int timeRequested = text.toInt();
        
        digitalWrite(LED_PIN, HIGH);
        lightTimerActive = true;
        lightTimerExpires = millis() + (timeRequested * 1000 * 60);
      }


    } else {
      String chat_id = String(bot.messages[i].chat_id);
      String text = bot.messages[i].text;

      if (text == F("/switch")) {

        // Keyboard Json is an array of arrays.
        // The size of the main array is how many row options the keyboard has
        // The size of the sub arrays is how many coloums that row has

        // "The Text" property is what shows up in the keyboard
        // The "callback_data" property is the text that gets sent when pressed  
        
        String keyboardJson = F("[[{ \"text\" : \"ON led 1\", \"callback_data\" : \"ON1\" },{ \"text\" : \"OFF led 1\", \"callback_data\" : \"OFF1\" },{ \"text\" : \"ON led 2\", \"callback_data\" : \"ON2\" },{ \"text\" : \"OFF led 2\", \"callback_data\" : \"OFF2\" }],");
        keyboardJson += F("[{ \"text\" : \"10 Mins\", \"callback_data\" : \"TIME10\" }, { \"text\" : \"20 Mins\", \"callback_data\" : \"TIME20\" }, { \"text\" : \"30 Mins\", \"callback_data\" : \"TIME30\" }]]");
        bot.sendMessageWithInlineKeyboard(chat_id, "LED LIGHTS", "", keyboardJson);
      }

     
     if (text == F("/temperature")){
          float t = dht.readTemperature()-2;
          if (isnan(t)) {    
          bot.sendMessage(chat_id,"Failed to read from DHT sensor!");
          }
          else {
            String temp = "temperature now: ";
          temp += float(t);
          temp += "*C\n";

          bot.sendMessage(chat_id, temp,"");  
          }     
      }

          
    if (text == "/humidity") {
        float h = dht.readHumidity();
       String temp = "humidity now: ";
       temp += float(h);
       temp += " %";
     
      bot.sendMessage(chat_id,temp, "");
    }

      // When a user first uses a bot they will send a "/start" command
      // So this is a good place to let the users know what commands are available

      if (text == F("/start")) {

       bot.sendMessage(chat_id, "Welcome, this is Smart Home program \n\n");
       bot.sendMessage(chat_id, "For lightings control please type /switch.\n");
       bot.sendMessage(chat_id, "For temperature reading please type /temperature.\n");
        bot.sendMessage(chat_id,"For humidity reading please type /humidity\n", "Markdown");
        
      }
    }
  }
}

void loop() {

//act as main switch from house
  if (Serial.available()>0){
    char comdata = char(Serial.read());
    if(comdata=='Y'){
      Serial.println("YELLOW LED is ON");
      digitalWrite(25,HIGH);
    }
    else if (comdata == 'G'){
      Serial.println("green LED is ON");
      digitalWrite(14,HIGH);
    }
    else if (comdata == 'X'){
      Serial.println("YELLOW LED is OFF");
      digitalWrite(25, LOW);
    }
    else if (comdata == 'O'){
      Serial.println("Green LED is OFF");
      digitalWrite(14,LOW);
    }
  }


  if (millis() > lastTimeChecked + delayBetweenChecks)  {

    // getUpdates returns 1 if there is a new message from Telegram
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    if (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
    }

    lastTimeChecked = millis();

    if (lightTimerActive && millis() > lightTimerExpires) {
      lightTimerActive = false;
      digitalWrite(LED_PIN, LOW);
    }
  }
}
