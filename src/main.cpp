#include <Arduino.h>
#include <SoftwareSerial.h>
#include <avr/wdt.h>
#include "IRremote.h"
#include "DHT.h"

int DHT_PIN = 2;
int RECV_PIN = 11;
int PIR_PIN = 10;

IRrecv irrecv(RECV_PIN);
SoftwareSerial mySerial(8, 7);
DHT dht;

const String BOARD_NAME = "einstein";
const int BOARD_VERSION = 0;

const bool MDEBUG = true;

int pir_default_state = 0;
const int DHT_UPDATE_TIME = 10 * 1000; 
unsigned long dht_last_update = 0;

String message[10];

float humidity = 0.0;
float temperature = 0.0;

decode_results results;

void get_ir();
void update_dht();
void verify_pir();
void sync_response();
String json_builder(String action, String argument);
bool parse_command(String s);
void serial_flush();
void debug(const char* c);

void setup(){
    wdt_enable(WDTO_8S);

    pinMode(PIR_PIN, INPUT);

    mySerial.begin(19200);
    Serial.begin(19200);
    irrecv.enableIRIn();
    dht.setup(DHT_PIN); // data pin 2
}

void loop(){
    get_ir();
    update_dht();
    verify_pir();

    String buffer_string;

    if(mySerial.available()){
        buffer_string  = mySerial.readStringUntil('\n');
        Serial.print("D: Recived: ");
        Serial.println(buffer_string);
    }

    if(!buffer_string.equals("")){
        if(parse_command(buffer_string)){
            debug("I'm a command !");

            if(message[0] != "0"){
                mySerial.println(json_builder("error", "Incompatible version"));
            }else if(message[1] == "server"){
                if(message[2] == "sync"){
                    mySerial.flush();
                    sync_response();
                }else{
                    mySerial.println(json_builder("error", "Command not valid"));
                }
            }else{
                mySerial.println(json_builder("error", "Sender not recognized"));
            }
        }

        buffer_string = "";
    }

    delay(1);
    wdt_reset();
}

void get_ir(){
    if (irrecv.decode(&results)) {
        Serial.println(results.value, HEX);
        mySerial.println(json_builder("IR", String(results.value)));
        irrecv.resume(); // Receive the next value
    }
}

void update_dht(){
    if(millis() - dht_last_update > DHT_UPDATE_TIME){
        dht_last_update = millis();
        humidity = dht.getHumidity();
        temperature = dht.getTemperature();
    }
}

void verify_pir(){

}

void sync_response(){
    mySerial.println(json_builder("TEMP", String(temperature)));
    mySerial.println(json_builder("HUMIDITY", String(humidity)));
    mySerial.println(json_builder("PIR", digitalRead(PIR_PIN) == 1 ? "1" : "0"));
}

String json_builder(String action, String argument){
    String string = "{\"version\":";
    string.concat(BOARD_VERSION);
    string.concat(", \"name\" : \"");
    string.concat(BOARD_NAME);
    string.concat("\", \"action\" : \"");
    string.concat(action);
    string.concat("\", \"argument\" : \"");
    string.concat(argument);
    string.concat("\"}");

    return string;
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
