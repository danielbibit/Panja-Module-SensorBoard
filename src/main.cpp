#include <Arduino.h>
#include <SoftwareSerial.h>
#include <avr/wdt.h>
#include "IRremote.h"
#include "DHT.h"

IRrecv irrecv(RECV_PIN);
SoftwareSerial mySerial(8, 7);
DHT dht;

const String BOARD_NAME = "";
const int BOARD_VERSION = 0;

const bool MDEBUG = true;

int RECV_PIN = 11;

String generate_status();
bool parse_command(String s);
void serial_flush();
void debug(const char* c);

String message[10];

void setup(){
    wdt_enable(WDTO_8S);

    mySerial.begin(19200);
    Serial.begin(19200);
    dht.setup(2); // data pin 2
}

void loop(){
    float humidity = dht.getHumidity();
    float temperature = dht.getTemperature();
    
    String buffer_string;

    if(mySerial.available()){
        buffer_string  = mySerial.readStringUntil('\n');
        Serial.print("D: Recived: ");
        Serial.println(buffer_string);
    }

    delay(1);
    wdt_reset();
}

String generate_status(){
    return "";
}

bool parse_command(String s){
    int n = 0;

    for(int i = 0; i < 10; i++){
        message[i] = "";
    }

    for(int i = 0; i < (int)s.length(); i++){
        if(s.charAt(i) == ';'){
            n += 1;
        }else{
            message[n].concat(s.charAt(i));
        }
    }

    return n >= 3 ? true : false;
}

void serial_flush(){
    while(mySerial.available() > 0){
        char t = mySerial.read();
    }
}

void debug(const char* c){
    if(MDEBUG){
        Serial.print("D: ");
        Serial.println(c);
    }
}
