/*************************************
FileName:      Sensor_Presion_v01.ino
Dependencies:
Processor:     ATmega32u4
Hardware:      Arduino Pro Micro
Compiler:
Company:       INGER
Created:	   01/05/2015 19:25:26
Author:		   Bruno
**************************************
File Description:

Change History:
Rev   Date         Description
---   ----------   ---------------
1.0   01/05/2015   Initial release.
1.1   25/05/2015   Prueba de hardware

*************************************/

// Se importan las librerías
#include <SFE_BMP180.h>
#include <Wire.h>
//Se declara una instancia de la librería
SFE_BMP180 pressure;

// Defines
#define LQ0_0 5		// Indicador salida rele
#define Err   6		// Indicador de ERROR
#define Q0_0  8		// Salida de potencia
#define Run   9		// Indicador RUN/STOP
#define Cero  15	// Ajuste de cero
#define Pote  A3	// Ajuste de sensibilidad

//Se declaran las variables. Es necesario tomar en cuenta una presión inicial
//esta será la presión que se tome en cuenta en el cálculo de la diferencia de altura
double PresionBase;
//Leeremos presión y temperatura. Calcularemos la diferencia de altura
double Presion = 0;
double Altura = 0;
double Temperatura = 0;
char status;

int Pulsador = 0;
int  Potes = 0;

void setup()
{
	Serial.begin(9600);
	//Se inicia el sensor y se hace una lectura inicial
	SensorStart();
	pinMode(LQ0_0, OUTPUT);
	pinMode(Err, OUTPUT);
	pinMode(Q0_0, OUTPUT);
	pinMode(Run, OUTPUT);
	pinMode(Cero, INPUT);
}
void loop()
{
	Pulsador = digitalRead(Cero);
	Potes = analogRead(Pote);
	//Se hace lectura del sensor
	ReadSensor();
	//Se imprimen las variables
	Serial.println(" ////// ");
	Serial.print("Temperatura: ");
	Serial.print(Temperatura);
	Serial.println(" grados C");
	Serial.print("Presion: ");
	Serial.print(Presion);
	Serial.println(" milibares");
	Serial.print("Altura relativa: ");
	Serial.print(Altura);
	Serial.println(" metros");
	//delay(2000);
	digitalWrite(LQ0_0, HIGH);   // sets the LED on
	delay(200);                  // waits for a second
	digitalWrite(LQ0_0, LOW);    // sets the LED off
	delay(200);
	digitalWrite(Err, HIGH);   // sets the LED on
	delay(200);                  // waits for a second
	digitalWrite(Err, LOW);    // sets the LED off
	delay(200);
	digitalWrite(Q0_0, HIGH);   // sets the LED on
	delay(1000);                  // waits for a second
	digitalWrite(Q0_0, LOW);    // sets the LED off
	delay(1000);
	digitalWrite(Run, HIGH);   // sets the LED on
	delay(200);                  // waits for a second
	digitalWrite(Run, LOW);    // sets the LED off
	delay(200);
	Serial.print("Pulsador: ");
	Serial.println(Pulsador);
	Serial.print("Pote: ");
	Serial.println(Potes);
}
void SensorStart()
{
	//Secuencia de inicio del sensor
	if (pressure.begin())
	Serial.println("BMP180 init success");
	else
	{
		Serial.println("BMP180 init fail (disconnected?)\n\n");
		while (1);
	}
	//Se inicia la lectura de temperatura
	status = pressure.startTemperature();
	if (status != 0)
	{
		delay(status);
		//Se lee una temperatura inicial
		status = pressure.getTemperature(Temperatura);
		if (status != 0)
		{
			//Se inicia la lectura de presiones
			status = pressure.startPressure(3);
			if (status != 0)
			{
				delay(status);
				//Se lee la presión inicial incidente sobre el sensor en la primera ejecución
				status = pressure.getPressure(PresionBase, Temperatura);
			}
		}
	}
}
void ReadSensor()
{
	//En este método se hacen las lecturas de presión y temperatura y se calcula la altura
	//Se inicia la lectura de temperatura
	status = pressure.startTemperature();
	if (status != 0)
	{
		delay(status);
		//Se realiza la lectura de temperatura
		status = pressure.getTemperature(Temperatura);
		if (status != 0)
		{
			//Se inicia la lectura de presión
			status = pressure.startPressure(3);
			if (status != 0)
			{
				delay(status);
				//Se lleva a cabo la lectura de presión,
				//considerando la temperatura que afecta el desempeño del sensor
				status = pressure.getPressure(Presion, Temperatura);
				if (status != 0)
				{
					//Se hace el cálculo de la altura en base a la presión leída en el Setup
					Altura = pressure.altitude(Presion, PresionBase);
				}
				else Serial.println("error en la lectura de presion\n");
			}
			else Serial.println("error iniciando la lectura de presion\n");
		}
		else Serial.println("error en la lectura de temperatura\n");
	}
	else Serial.println("error iniciando la lectura de temperatura\n");
}
