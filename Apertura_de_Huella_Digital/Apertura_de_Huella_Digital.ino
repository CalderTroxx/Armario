//EEPROM
#include <EEPROM.h>

//RFID
#include <SPI.h>
#include <MFRC522.h>

//Transmisor RF
#include <VirtualWire.h>

//LCD
#include <LiquidCrystal.h>

//SENSOR DE HUELLA
#include <Adafruit_Fingerprint.h>
SoftwareSerial mySerial(4, 5);     // Crear Serial para Sensor  Rx, TX del Arduino
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);  // Crea el objeto Finger comunicacion pin 2 , 3

//RELOJ
#include "RTClib.h"
RTC_DS3231 rtc;

//RFID
#define RST_PIN 9
#define SS_PIN  10
MFRC522 mfrc522 (SS_PIN, RST_PIN);

//REEMPLAZO DE DELAY
#define spamtimer(t) for (static uint32_t SpamTimer; (uint32_t) (millis() - SpamTimer) >= (t); SpamTimer = millis())

//AGREGAR ID AUTOMATICAMENTE
uint8_t id;
int num, count;
bool muestra;

//MODO CARGAR HUELLA
boolean boton_setup = 0;

//APERTURA DE ARMARIO
int cerradura = 6;
unsigned long time_espera = 0, tiempo_actual = 0;
bool validation = 0;

//LCD
const int rs = 8, en = 7, d4 = A0, d5 = A1, d6 = A2, d7 = A3;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//EEPROM
int posicion = 0;

//SENSOR DE PUERTA
bool estado;

//MODULO RFID
byte LecturaUID[4]; // crea array para almacenar el UID leido
byte Osciloscopio[4] = {0xDE, 0xD7, 0x3C, 0xC9} ;
byte EstacionSoldado[4] = {0x1E, 0x66, 0x3C, 0xC9} ;
byte Soldador[4] = {0x3E, 0xD7, 0x3B, 0xC9} ;
byte Estano[4] = {0x0E, 0x7A, 0x3D, 0xC9} ;
byte Flux[4] = {0xFE, 0x67, 0x3D, 0xC9} ;

//DATOS A TRANSMITIR
String Profesor = "";
String Herramienta = "";
String Evento = "";

//TRANSMICION RF
String origen = "AR";
String destino = "RP";
char buf[VW_MAX_MESSAGE_LEN];
String str = "";
bool CT = 0;

bool stateOsci = 1;
bool stateEstacion = 1;
bool stateSoldador = 1;
bool stateEstano = 1;
bool stateFlux = 1;
/////////////////SETUP/////////////////////

void setup() {
  //MODULOS PARA TRANSMITIR
  //vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(2000);   // Bits per sec
  vw_set_tx_pin(1);
  //RFID
  SPI.begin();
  mfrc522.PCD_Init();
  //LCD
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  //BOTON
  attachInterrupt(digitalPinToInterrupt(2), boton, RISING);
  //SENSOR DE PUERTA
  pinMode(3, INPUT);
  //CERRADURA
  pinMode(6, OUTPUT); //Puerto de cerradura como salida
  Serial.begin(9600);
  //SENSOR DE HUELLA
  //Serial.println("Sistema de apertura con huella dactilar");
  finger.begin(57600);  // inicializa comunicacion con sensor a 57600 Baudios
  delay(5);
  if (finger.verifyPassword()) delay(1);//Serial.println("Detectado un sensor de huella!"); //Se muestra en el monitor serie.
  else { //Si no se encuentra al detector de huellas como si estuviese conectado...
    //Serial.println("No hay comunicacion con el sensor de huella");
    //Serial.println("Revise las conexiones");
    delay(1);
    while (1) { //Se quedara en este mientras y no saldra de aqui hasta que se conecte el detector de huellas.
      delay(1);
    }
  }
  finger.getTemplateCount();
  //Serial.print("El sensor contiene "); Serial.print(finger.templateCount); Serial.println(" plantillas");
  //Serial.println("Esperando por una huella valida...");
  //LEE ULTIMA ID CARGADA
  EEPROM.get(posicion, num);
}

/////////////////////////////////////////////////////////////////////

void readnumber() { // FUNCION QUE AUMENTA EL ID AUTOMATICAMENTE
  num = num + 1;
  EEPROM.put(posicion, num);
  muestra = 1;
}

///////////////////////////////////////////////////////////////////

void boton() { //FUNCION QUE DETECTA EL BOTON
  if (millis() - tiempo_actual > 150) {
    boton_setup = 1;
  }
  tiempo_actual = millis();
}

//////////////////////////////////////////////////////////////////////////

void loop() {
  estado = digitalRead(3);
  spamtimer(50) {//TIEMPO ENTRE LECTURAS
    if (boton_setup == 1 /*and validation == 1*/) //ENTRA EN CARGAR HUELLA SI SE PULSA EL BOTON
    {
      lcd.setCursor(0, 0);
      lcd.print("CARGAR HUELLA   ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
      readnumber();//CARGA ID
      id = num;
      boton_setup = 0;//SALE DE LA FUNCION DE CARGAR ID
      while (!  Grabacion_Huellas() ); //Llama a la funcion en donde se graban las huellas dactilares
    }
    else { //Si no se pulso ningun boton o si se deja de pulsar...
      lcd.setCursor(0, 0);
      lcd.print("ESPERANDO HUELLA");
      lcd.setCursor(0, 1);
      lcd.print("                ");//LIMPIA EL RENGLON DE ABAJO DEL LCD PARA QUE DESAPAREZCA LA ID
      if (muestra == 1) {
        count = count + 1;//CONTADOR PARA QUE DEJE DE MOSTRAR LA ID DESPUES DE UN MOMENTO
        lcd.setCursor(0, 1);
        lcd.print("   LA ID ES " + String (id));//IMPRIME ID
      }

      if (count > 3) {//CUANDO EL CONTADOR LLEGA A 3 DEJA DE MOSTRAR LA ID
        muestra = 0;//CANCELA LA MUESTRA
        count = 0;//REINICIA EL CONTADOR
      }

      Confirmacion_Huella(); //Llama a la funcion en la que se ingresan las huellas ya grabadas
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t Grabacion_Huellas() {//FUNCION PARA GRABAR LAS HUELLAS
  uint8_t p = finger.getImage();//LEE LA HUELLA
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p)
    {
      case FINGERPRINT_OK:
        lcd.setCursor(0, 1);
        lcd.print("HUELLA TOMADA   ");//PRIMER TOMA DE HUELLA
        //Serial.println("Huella tomada");
        break;
    }
  }


  p = finger.image2Tz(1);
  /* switch (p) {
     case FINGERPRINT_OK:
       Serial.println("Huella convertida");
       break;
     case FINGERPRINT_IMAGEMESS:
       Serial.println("Huella demasiado desordenada");
       return p;
     case FINGERPRINT_PACKETRECIEVEERR:
       Serial.println("Error de comunicacion");
       return p;
     case FINGERPRINT_FEATUREFAIL:
       Serial.println("No se pudieron encontrar las funciones de huellas dactilares");
       return p;
     case FINGERPRINT_INVALIDIMAGE:
       Serial.println("No se pudieron encontrar las funciones de huellas dactilares");
       return p;
     default:
       Serial.println("Error desconocido");
       return p;
    }*/
  lcd.setCursor(0, 1);
  lcd.print("QUITE EL DEDO   ");//SE RETIRA EL DEDO CUANDO EL TEXTO LO INDIQUE
  //Serial.println("Quite el dedo");
  delay(2000);//ESPERA PARA QUE PUEDA LEERSE LA INSTRUCCION DE RETIRAR EL DEDO
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  //Serial.print("ID "); Serial.println(id);
  p = -1;
  lcd.setCursor(0, 1);
  lcd.print("INGRESE HUELLA  ");//SEGUNDA LECTURA DE HUELLA
  //Serial.println("Coloque el mismo dedo de nuevo");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p)
    {
      case FINGERPRINT_OK:
        lcd.setCursor(0, 0);
        lcd.print("HUELLA CARGADA  ");//LA HUELLA SE CARGO EXITOSAMENTE
        //Serial.println("Huella tomada");
        break;
        /* case FINGERPRINT_NOFINGER:
           Serial.print(".");
           break;
          case FINGERPRINT_PACKETRECIEVEERR:
           Serial.println("Error de comunicacion");
           break;
          case FINGERPRINT_IMAGEFAIL:
           Serial.println("Error de huella");
           break;
          default:
           Serial.println("Error desconocido");
           break;*/
    }
  }


  p = finger.image2Tz(2);
  /*  switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Huella convertida");
        break;
      case FINGERPRINT_IMAGEMESS:
        Serial.println("Huella demasiado desordenada");
        return p;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Error de comunicacion");
        return p;
      case FINGERPRINT_FEATUREFAIL:
        Serial.println("No se pudieron encontrar las funciones de huellas dactilares");
        return p;
      case FINGERPRINT_INVALIDIMAGE:
        Serial.println("No se pudieron encontrar las funciones de huellas dactilares");
        return p;
      default:
        Serial.println("Error desconocido");
        return p;
    }*/



  //Serial.print("Creando modelo para #");  Serial.println(id);

  p = finger.createModel();
  /* if (p == FINGERPRINT_OK) {
     Serial.println("¡Las huellas coinciden!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR) {
     Serial.println("Error de comunicacion");
     return p;
    }
    else if (p == FINGERPRINT_ENROLLMISMATCH) {
     Serial.println("Las huellas dactilares no coinciden");
     return p;
    }
    else {
     Serial.println("Error de comunicacion");
     return p;
    }*/

  //Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  /* if (p == FINGERPRINT_OK) {
     Serial.println("Almacenado!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR) {
     Serial.println("Error de comunicacion");
     return p;
    }
    else if (p == FINGERPRINT_BADLOCATION) {
     Serial.println("No se pudo almacenar en esa ubicación");
     return p;
    }
    else if (p == FINGERPRINT_FLASHERR) {
     Serial.println("Error al escribir en flash");
     return p;
    }
    else {
     Serial.println("Error desconocido");
     return p;
    }*/
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Confirmacion_Huella() {//ABRE EL ARMARIO CON LAS HUELLAS
  uint8_t p = finger.getImage();//LEE LA HUELLA
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();

  if (finger.confidence > 50) {

    // Si hay coincidencias de huella
    //Serial.print("ID #"); Serial.print(finger.fingerID);
    //Serial.print("Coincidencia del "); Serial.println(finger.confidence);
    Profesor = finger.fingerID;

    validation = 1;
    abrir_cerradura();//ABRE EL ARMARIO
    return finger.fingerID;
  }


  else {
    lcd.setCursor(0, 0);
    lcd.print("NO COINCIDE");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    //Serial.println("Huella sin coincidencia");//LA HUELLA NO COINCIDE
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void abrir_cerradura() {//FUNCION PARA ABRIR EL ARMARIO
  int tiempo = 0;
  bool flag = 0;
  digitalWrite(cerradura, HIGH);//Pone en estado alto el pin 5, es decir que se abre la cerradura
  while (digitalRead(3) == 0 && flag == 0) {
    lcd.setCursor(0, 0);
    lcd.print(" ABRA LA PUERTA ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    tiempo++;
    if (tiempo == 1000) {
      lcd.setCursor(0, 0);
      lcd.print(" VUELVA A ABRIR ");
      digitalWrite(cerradura, LOW);
      flag = 1;
      tiempo = 0;
    }
  }
  while (digitalRead(3) == 1) {
    lcd.setCursor(0, 0);
    lcd.print(" PUERTA ABIERTA ");
    MATERIALES();//FUNCION DE CONTROL DE HERRAMIENTAS
    if (CT == 1){
    Transmitir();
    }
  }
  digitalWrite(cerradura, LOW); //Cierra la puerta
  validation = 0;
}

/////////////////////////////////////////////////////////////////////////////////

void MATERIALES() { //FUNCION DE DETECCION DE MATERIALES
  if (mfrc522.PICC_IsNewCardPresent()) { //PREGUNTA SI HAY UN LLAVERO FRENTE AL RFID
    //Serial.println("hola");
    CT = 1;
    if (mfrc522.PICC_ReadCardSerial()) { //PREGUNTA SI SE LEE UN LLAVERO
      //Serial.print("UID:");       // muestra texto UID:
      for (byte i = 0; i < mfrc522.uid.size; i++) { // bucle recorre de a un byte por vez el UID
        if (mfrc522.uid.uidByte[i] < 0x10) {  // si el byte leido es menor a 0x10
          //Serial.print(" 0");       // imprime espacio en blanco y numero cero
        }
        else {          // sino
          //Serial.print(" ");        // imprime un espacio en blanco
        }
        //Serial.print(mfrc522.uid.uidByte[i], HEX);    // imprime el byte del UID leido en hexadecimal
        LecturaUID[i] = mfrc522.uid.uidByte[i];   // almacena en array el byte del UID leido
      }

      //Serial.print("\t");         // imprime un espacio de tabulacion

      if (comparaUID(LecturaUID, Osciloscopio)) {

        //Serial.println("Osciloscopio");
        Herramienta = "01";
        Evento = stateOsci;
        stateOsci = !stateOsci;
      }
      else if (comparaUID(LecturaUID, EstacionSoldado)) {
        //Serial.println("Estacion de Soldado");
        Herramienta = "02";
        Evento = stateEstacion;
        stateEstacion = !stateEstacion;
      }
      else if (comparaUID(LecturaUID, Soldador)) {

        //Serial.println("Soldador");
        Herramienta = "03";
        Evento = stateSoldador;
        stateSoldador = !stateSoldador;
      }
      else if (comparaUID(LecturaUID, Estano)) {
        //Serial.println("Estaño");
        Herramienta = "04";
        Evento = stateEstano;
        stateEstano = !stateEstano;
      }
      else if (comparaUID(LecturaUID, Flux)) {

        //Serial.println("Flux");
        Herramienta = "05";
        Evento = stateFlux;
        stateFlux = !stateFlux;
      }
      mfrc522.PICC_HaltA();     // detiene comunicacion con tarjeta
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////

boolean comparaUID(byte lectura[], byte usuario[]) { // FUNCION QUE COMPARA EL UID
  for (byte i = 0; i < mfrc522.uid.size; i++) { // bucle recorre de a un byte por vez el UID
    if (lectura[i] != usuario[i])       // si byte de UID leido es distinto a usuario
      return (false);         // retorna falso
  }
  return (true);          // si los 4 bytes coinciden retorna verdadero
}

void Transmitir() {
  Serial.println(Profesor);
  Serial.println(Herramienta);
  Serial.println(Evento);

  
  str = origen + destino + "i" + String(Profesor);
  str.toCharArray(buf, sizeof(buf));
  vw_send((uint8_t *)buf, strlen(buf));
  vw_wait_tx();
  //Serial.print("HOLA");

  str = origen + destino + "j" + String(Herramienta);
  str.toCharArray(buf, sizeof(buf));
  vw_send((uint8_t *)buf, strlen(buf));
  vw_wait_tx();

  str = origen + destino + "k" + String(Evento);
  str.toCharArray(buf, sizeof(buf));
  vw_send((uint8_t *)buf, strlen(buf));
  vw_wait_tx();
  CT = 0;
  }

//////////////////////////////////////////////////////FIN///////////////////////////////////////////////////////////////////////
