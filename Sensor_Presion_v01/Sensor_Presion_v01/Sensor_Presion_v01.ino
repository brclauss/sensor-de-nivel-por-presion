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
1.1   25/05/2015   Prueba de hardware.
1.2   15/06/2015   Lectura de presion, calculo de altura y comparaci�n con valor seteado.
1.3   22/06/2015   Envio de datos via puerto serie.

*************************************/

// Se importan las librer�as
#include <SFE_BMP180.h>
#include <Wire.h>

// Se declara una instancia de la librer�a
SFE_BMP180 pressure;

// Defines
#define LQ0_0 5		// Indicador salida rele
#define Err   6		// Indicador de ERROR
#define Q0_0  8		// Salida de potencia
#define Run   9		// Indicador RUN/STOP
#define Cero  15	// Ajuste de cero
#define Pote  A3	// Ajuste de sensibilidad

// Se declaran las variables. Es necesario tomar en cuenta una presi�n inicial
// esta ser� la presi�n que se tome en cuenta en el c�lculo de la diferencia de altura
double PresionBase;

// Leeremos presi�n y temperatura. Calcularemos la diferencia de altura
double Presion = 0;
double Altura = 0;
double Temperatura = 0;
char status;

// Variables para la aplicaci�n
double d_Presion_Base = 0;
double d_Presion = 0;
double d_Altura = 0;

int Pulsador = 0;
int Potes = 0;

byte b_Leer_Base = 1;

// Timers en ms
unsigned long T_Dato_Serie = 1000;

// Variables para Timer
boolean M_Timer_1 = 1;
unsigned long M_Time_1 = 0;
unsigned long TIMER_01 = 0;  // Variable to hold elapsed time for Timer 1


// Prototipos de Funciones
unsigned long timerPulse(boolean &timeractive, unsigned long &timeactual, unsigned long &timerState, unsigned long timerPeriod);	// Temporizador de pulso

void setup()
{
	Serial.begin(9600);				// Inicializa puerto serie
	SensorStart();					// Se inicia el sensor y se hace una lectura inicial
	pinMode(LQ0_0, OUTPUT);			// Inicializa salidas digitales
	pinMode(Err, OUTPUT);
	pinMode(Q0_0, OUTPUT);
	pinMode(Run, OUTPUT);
	pinMode(Cero, INPUT);			// Inicializa entradas digitales
}

void loop()
{
	digitalWrite(Run, HIGH);						// Enciende led de RUN
	if (b_Leer_Base)
	{
		ReadSensor();								// Se hace lectura del sensor
		d_Presion_Base = Presion;
		b_Leer_Base = 0;
	}
	ReadSensor();									// Se hace lectura del sensor
	d_Presion = Presion;
	Potes = analogRead(Pote);						// Se lee seteo de potenciometro
	Pulsador = digitalRead(Cero);					// Se lee estado del pulsador
	d_Altura = (d_Presion - d_Presion_Base)*10.2;	// Calculo de altura
	if (d_Altura >= Potes)							// Compara altura seteada con altura medida
	{
		digitalWrite(Q0_0, HIGH);					// Activa salida
		digitalWrite(LQ0_0, HIGH);					// Enciende led de salida Q0_0
	}
	else
	{
		digitalWrite(Q0_0, LOW);					// Desactiva salida
		digitalWrite(LQ0_0, LOW);					// Apaga led de salida Q0_0
	}
	
	if (M_Timer_1 == 1)
	{
		Serial.println(d_Presion);						// Transmite dato de presion medida
		Serial.println(d_Altura);						// Transmite dato de presion medida
	}
	timerPulse(M_Timer_1, M_Time_1, TIMER_01, T_Dato_Serie);
}

// Funciones locales
void SensorStart()
{
	// Secuencia de inicio del sensor
	if (pressure.begin())
	Serial.println("BMP180 init success");
	else
	{
		Serial.println("BMP180 init fail (disconnected?)\n\n");
		while (1);
	}
	// Se inicia la lectura de temperatura
	status = pressure.startTemperature();
	if (status != 0)
	{
		delay(status);
		// Se lee una temperatura inicial
		status = pressure.getTemperature(Temperatura);
		if (status != 0)
		{
			// Se inicia la lectura de presiones
			status = pressure.startPressure(3);
			if (status != 0)
			{
				delay(status);
				// Se lee la presi�n inicial incidente sobre el sensor en la primera ejecuci�n
				status = pressure.getPressure(PresionBase, Temperatura);
			}
		}
	}
}

void ReadSensor()
{
	// En este m�todo se hacen las lecturas de presi�n y temperatura y se calcula la altura
	// Se inicia la lectura de temperatura
	status = pressure.startTemperature();
	if (status != 0)
	{
		delay(status);
		// Se realiza la lectura de temperatura
		status = pressure.getTemperature(Temperatura);
		if (status != 0)
		{
			// Se inicia la lectura de presi�n
			status = pressure.startPressure(3);
			if (status != 0)
			{
				delay(status);
				// Se lleva a cabo la lectura de presi�n,
				// considerando la temperatura que afecta el desempe�o del sensor
				status = pressure.getPressure(Presion, Temperatura);
				if (status != 0)
				{
					// Se hace el c�lculo de la altura en base a la presi�n le�da en el Setup
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

// Temporizador tipo pulso
unsigned long timerPulse(boolean &timeractive, unsigned long &timeactual, unsigned long &timerState, unsigned long timerPeriod)
{
	if ((timeractive == 0) & (timerState == 0))				// Timer is either not triggered or finished
	{
		timerState = 0;
		timeactual = 0;										// Clear timerState (0 = 'not started')
	}
	else                                                    // Timer is enabled
	{
		if (timerState == 0)								// Timer hasn't started counting yet
		{
			timerState = millis();							// Set timerState to current time in milliseconds
			timeractive = 0;
			timeactual = 0;									// Result = 'not finished' (0)
		}
		else                                                // Timer is active and counting
		{
			if (millis() > (timerState + timerPeriod))		// Timer has finished
			{
				timeractive = 1;							// Pulse = 'finished' (0)
				timerState = 0;
			}
			else
			{												// Timer has not finished
				timeractive = 0;							// Pulse = 'Active' (1)
			}
			timeactual = (millis() - timerState) / 1000;
		}
	}
	return(timeactual);										// Return result (1 = 'finished',
	// 0 = 'not started' / 'not finished')
}
