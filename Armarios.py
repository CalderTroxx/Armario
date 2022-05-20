##########LIBRERIAS##########

import mysql.connector #Importamos la librera con la cual nos comunicaremos con la base de datos.
import serial #Importamos esta libreria para leer el puerto USB de la compu y asi leer los datos q nos ingresan.
import time 

##########CONEXION MYSQL##########

conexion=mysql.connector.connect( #Establecemos conexion con la base de datos (mysql).
    user="root", 
    password="45266203",  
    host="localhost",      #Declaramos los datos de la base de datos creada.
    database="armario",
    port="3306"
)

##########CONEXION ARDUINO##########

serialArduino=serial.Serial("COM4",9600) #Establecemos conexion con el puerto USB, al cual el arduino esta conectado.
time.sleep(1) #Esta funcion lo q hara es esperar 1 segundo para establecer la conexion.

cursor1=conexion.cursor() #Hacemos conexion con la base de datos y asi dsp procedemos a mandarle datos.

##########LECTURAS DE DATOS##########
ProfesorAnterior = ""
HerramientaAnterior = ""
EventoAnterior = ""

while(1):
    Profesor=serialArduino.readline()
    Herramienta=serialArduino.readline()
    Evento=serialArduino.readline()

    if(Profesor != ProfesorAnterior or Herramienta != HerramientaAnterior or Evento != EventoAnterior):
        ProfesorAnterior = Profesor
        HerramientaAnterior = Herramienta
        EventoAnterior = Evento
        print("Cambio")
        Profesor=Profesor.decode('ascii') #Decoficamos el dato,binario, al codigo ascii.
        Profesor = Profesor.strip()
        match Profesor:
            case "01":
                Profesor = "Franco Valverde"
            case "02":
                Profesor = "Persona 2"
            case "03":
                Profesor = "Persona 3"
            case "04":
                Profesor = "Persona 4"
            case "05":
                Profesor = "Persona 5"

        Herramienta=Herramienta.decode('ascii') 
        Herramienta = Herramienta.strip()
        match Herramienta:
            case "01":
                Herramienta = "Osciloscopio"
            case "02":
                Herramienta = "Estacion de Soldado"
            case "03":
                Herramienta = "Soldador"
            case "04":
                Herramienta = "Estaño"
            case "05":
                Herramienta = "Flux"

        Evento=Evento.decode('ascii')
        Evento = Evento.strip()
        if Evento == "0":
            Estado = "Armario Cerrado"
        elif Evento == "1":
            Estado ="Armario Abierto"

##########EJECUCION DE DATOS##########

    
        sql="insert into datos(Profesor, Herramientas, Evento) values (%s,%s,%s)" #En esta funcion declaramos lo q debe ver en las columnas de la base de dato y el tipo de dato (%s).
        datos=(Profesor, Herramienta, Evento) #Utilizamos a dato para mandar a la base de dato los datos recibidor y guardados anteriormente.
        cursor1.execute(sql, datos) #Ejecutamos sql y datos.

    conexion.commit() #Establecemos a conexion como establecida.
    #conexion.close() #Cerramos la conexion.