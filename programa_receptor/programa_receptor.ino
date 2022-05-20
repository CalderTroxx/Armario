#include <VirtualWire.h>
String origen = "";
String Profesor = "", Herramienta = "", Texto = "";
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  vw_setup(2000);
  vw_set_rx_pin(0);
  vw_rx_start();
}

void loop() {
  // put your main code here, to run repeatedly:
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  if (vw_get_message((uint8_t *)buf, &buflen)) {
    Serial.println("Recibiendo");
    int i;
    String DatoCadena = "";
    if ((char)buf[2] == 'R' && (char)buf[3] == 'P') {
      String origen = "";
      origen.concat((char)buf[0]);
      origen.concat((char)buf[1]);
      Serial.println(String(origen));
      if ((char)buf[4] == 'i') { //verifica el inicio de trama
        for (i = 5; i < buflen; i++) DatoCadena.concat((char)buf[i]);
        Profesor.concat(buf[i]);
        Serial.println(Profesor);
      }
      if ((char)buf[4] == 'j') { //verifica el inicio de trama
        for (i = 5; i < buflen; i++) DatoCadena.concat((char)buf[i]);
        Herramienta.concat(buf[i]);
        Serial.println(Herramienta);
      }
      if ((char)buf[4] == 'k') { //verifica el inicio de trama
        for (i = 5; i < buflen; i++) DatoCadena.concat((char)buf[i]);
        Texto.concat(buf[i]);
        Serial.println(Texto);
      }
    }
  }
}
