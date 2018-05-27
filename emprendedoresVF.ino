#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>

//Declaremos los pines CE y el CSN
#define CE_PIN 9
#define CSN_PIN 10

SoftwareSerial SIM900(7,8);
SoftwareSerial BT(4,5); //4 RX, 5 TX.

byte direccion[5] ={'c','a','n','a','l'};
RF24 radio(CE_PIN, CSN_PIN);
int green = 2; //led indica dispositivo configurado.
int red = 3;   //led indica dispositivo no configurado.
//int cerrar = 6;//led indica cerrar valvula.
int gasVal = A0;
int pushB = 6;
int primerInicio = 0;
char cadena[255]; //cadena del mensaje recibido para la configuracion.7hh
String configTel = "AT+CMGS=\"+52";
String finConfig = "\"";

struct datConf{
  char ubicacion[30];
  char telefono[50];
  char user[20];
  char password[20];
};

datConf datosE,datosS;

int eeAddress1 = 0,eeAddress2 = 0;

void setup() {
  SIM900.begin(19200);
  Serial.begin(19200);
  BT.begin(9600);
  radio.begin();
  pinMode(green,OUTPUT);
  pinMode(red,OUTPUT);
  pinMode(gasVal,INPUT);
  pinMode(pushB,INPUT_PULLUP);
  delay(1000);
  delay(15000);
  radio.openWritingPipe(direccion);
  primerInicio = configurado();
}

void loop() {
  int leePush = digitalRead(pushB);
  if(!primerInicio){
    configura();
  }
  else if(BT.available()){
    String cadena;
    cadena=BT.readString();
    Serial.println(cadena);
    separa(cadena);
  }
  else if(leePush == 0){
    char datos[16] = "apaga";
    radio.write(datos,sizeof(datos));
    delay(200);
  }
  else{
    sensorGas();
  }  
}

void mensaje_sms(String cel){
  Serial.println("Enviando SMS...");
  SIM900.print("AT+CMGF=1\r");
  delay(1000);
  Serial.println(cel);
  SIM900.println(configTel+cel+finConfig);          
  delay(1000);
  //String ubica(datoSal.ubicacion);
  SIM900.println("Alerta de fuga de gas");// Texto del SMS
  delay(100);
  SIM900.println((char)26);
  delay(100);
  SIM900.println();
  delay(5000);
  Serial.println("SMS enviado");
}

void sensorGas(){
  datosMemEEPROM();
  int gasValRead;
  double gasPercentage;
  gasValRead= analogRead(gasVal);
  gasPercentage = (gasValRead/1024.0);
  gasPercentage *= 100;
  if (gasPercentage >= 20){
    Serial.println(datosS.telefono);
    String numero(datosS.telefono);
    char datos[16] = "gas";
    radio.write(datos,sizeof(datos));
    mensaje_sms(numero);
  }
}

void datosMemEEPROM(){
  eeAddress2 = sizeof(datConf)+1;
  EEPROM.get(eeAddress2,datosS);
  delay(8000);  
}

int configurado(){
  byte valor;
  valor = EEPROM.read(0);
  if(valor == 1){
    digitalWrite(green,HIGH);
    digitalWrite(red,LOW);
    return 1;
  }
  else{
    digitalWrite(red,HIGH);
    digitalWrite(green,LOW);
    return 0;
  }
}

void configura(){
  String cadena;
  if(BT.available()){
    cadena=BT.readString();
    Serial.println(cadena);
    separa(cadena);
  }
}

void separa(String cadena){
  char *ptr;
  char dato[190];
  int eeAddress0 = 0;
  
  strcpy(dato,cadena.c_str());
  ptr = strtok(dato,",");
  strcpy(datosE.ubicacion,ptr);
  ptr = strtok(NULL,",");
  strcpy(datosE.telefono,ptr);
  ptr = strtok(NULL,",");
  strcpy(datosE.user,ptr);
  ptr = strtok(NULL,",");
  strcpy(datosE.password,ptr);
  eeAddress1 = sizeof(datConf)+1;
  EEPROM.put(eeAddress1,datosE);
  eeAddress2 = sizeof(datConf)+1;
  EEPROM.get(eeAddress2,datosS);
  digitalWrite(green,HIGH);
  digitalWrite(red,LOW);
  primerInicio = 1;
  EEPROM.write(eeAddress0,1);
}

