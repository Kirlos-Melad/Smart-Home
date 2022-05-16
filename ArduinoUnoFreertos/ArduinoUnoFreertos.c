#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "FreeRTOS.h"
#include "task.h"
#include "lcd.h"
#include "queue.h"

void Setup();

// Sensor Handlers
void Smoke_Sensor_Handler(void *pvParameters );
void Temp_Sensor_Handler(void *pvParameters );
void Water_Depth_Sensor_Handler(void *pvParameters );
void Readings_Handler(void *pvParameters );
int Analog_Read(uint8_t pin);
void Interrupt_Handler();

volatile unsigned long lastInterrupt;

#define byte char

// Sensors
#define buzzerPinB PORTB5
#define buttonPinD PORTD2

#define waterPinC PORTC4
#define tempPinC PORTC3
#define smokePinB PORTC2

#define TEMP_CRITICAL 50
#define SMOKE_CRITICAL 200
#define WATER_LEVEL_CRITICAL 200

int main(void)
{
	Setup();
	cli();
	attachInterrupt(0, Interrupt_Handler, 1);
	
	QueueHandle_t xQueue[3] = {0};
	for(int i = 0; i < 3; i++)
	xQueue[i] = xQueueCreate( 3, sizeof( void * ) );
	
	byte isReady = 1;
	for(int i = 0; i < 3; i++)
	isReady &= (xQueue[i] != NULL);
	if( isReady)
	{
		xTaskCreate(Smoke_Sensor_Handler,(const char*)"Smoke Sensor Handler", 256, xQueue, 4, NULL);
		xTaskCreate(Temp_Sensor_Handler,(const char*)"Temp Sensor Handler", 256, xQueue + 1, 4, NULL);
		xTaskCreate(Water_Depth_Sensor_Handler,(const char*)"Water Depth Sensor Handler", 240, xQueue + 2, 4, NULL);
		xTaskCreate(Readings_Handler,(const char*)"Readings Handler", 256 , xQueue, 3, NULL);

		sei();

		vTaskStartScheduler();
	}

	while(1);

	return 0;
}

void Setup()
{
	// Setup LCD
	LCD_Init();
	
	// Set ADC
	ADMUX |= (1 << REFS0);
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADEN);

	// Sensors modes
	DDRB = (1 << buzzerPinB);
	PORTB = 0;
	DDRD = (1 << buttonPinD);
	PORTD = 0;
	DDRC = 0;
}

int Analog_Read(uint8_t pin){
	ADMUX |= pin;
	ADCSRA |= (1 << ADSC);
	
	while(ADCSRA & (1 << ADSC));
	
	return ADC;
}

void Interrupt_Handler(){
	if(xTaskGetTickCountFromISR() - lastInterrupt > 500){
		// disable buzzer
		PORTB &= ~(1 << buzzerPinB);
		
		lastInterrupt = xTaskGetTickCountFromISR();
	}
}

void Readings_Handler(void *pvParameters ){
	QueueHandle_t* xQueue = (QueueHandle_t *)pvParameters;
	int const numberOfQs = 3;
	
	int Data[]={0,0,0};
	BaseType_t Status[]={0,0,0};
	int Critical[] = {SMOKE_CRITICAL, TEMP_CRITICAL, WATER_LEVEL_CRITICAL};
	char* sensorName[] = {"Smoke: ", "Temperature: ", "Water Level: "};
	
	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(1)
	{
		for(int i = 0; i < numberOfQs; i++){
			Status[i] = xQueueReceive(xQueue[i], Data + i, 0);
			
			if(Status[i] == pdPASS){
				Clear_LCD();
				byte isCritical = (Data[i] > Critical[i]);
				PORTB |= (isCritical << buzzerPinB);
				WriteDataString(sensorName[i]) ;
				WriteDataString(isCritical ? "Critical" : "Normal");
				vTaskDelayUntil(&xLastWakeTime, (1500 / portTICK_PERIOD_MS));
			}
		}
	}
}
void Smoke_Sensor_Handler( void *pvParameters){
	QueueHandle_t* xQueue = (QueueHandle_t *)pvParameters;
	
	TickType_t xLastWakeTime = xTaskGetTickCount();
	int smokeValue;
	while( 1 )
	{
		smokeValue = Analog_Read(smokePinB);
		xQueueSendToBack( *xQueue, &smokeValue, 0 );
		vTaskDelayUntil(&xLastWakeTime, (2000 / portTICK_PERIOD_MS));
	}
}

void Temp_Sensor_Handler( void *pvParameters ){
	QueueHandle_t* xQueue = (QueueHandle_t *)pvParameters;
	
	TickType_t xLastWakeTime = xTaskGetTickCount();
	float tempvalue;
	while( 1 )
	{
		tempvalue = (Analog_Read(tempPinC) * 4.88); /* Convert ADC value to equivalent voltage */
		tempvalue = (tempvalue/10); /* LM35 gives output of 10mv/°C */
		xQueueSendToBack( *xQueue, &tempvalue, 0 );
		vTaskDelayUntil(&xLastWakeTime, (2000 / portTICK_PERIOD_MS));
	}
}
void Water_Depth_Sensor_Handler( void *pvParameters ){
	int waterDepthValue=0;
	QueueHandle_t* xQueue = (QueueHandle_t *)pvParameters;
	
	TickType_t xLastWakeTime = xTaskGetTickCount();
	while( 1 )
	{
		waterDepthValue = Analog_Read(waterPinC);
		xQueueSendToBack( *xQueue, &waterDepthValue, 0 );
		vTaskDelayUntil(&xLastWakeTime, (2000 / portTICK_PERIOD_MS));
	}
}