#include <Arduino.h>
#include <Wire.h>
#include <Button2.h>
#include <L3G.h>
#include <Adafruit_LSM303_Accel.h>
#include <Adafruit_LSM303DLH_Mag.h>
#include <TFT_eSPI.h>
#include <SPI.h>

#ifndef TFT_DISPOFF
#define TFT_DISPOFF 0x28
#endif

#ifndef TFT_SLPIN
#define TFT_SLPIN   0x10
#endif

#define SDA 21
#define SCL 22

#define TFT_MOSI            19
#define TFT_SCLK            18
#define TFT_CS              5
#define TFT_DC              16
#define TFT_RST             23
#define TFT_BL 4  // Display backlight control pin
#define ADC_EN 14 //ADC_EN is the ADC detection enable port
#define ADC_PIN 34
#define VREF 1100

typedef struct	s_float3
{
	float		x;
	float		y;
	float		z;
}				t_float3;

typedef struct	s_sensors{
	t_float3	accel;
	t_float3	gyro;
	t_float3	mag;
}				t_sensors;

t_sensors sensors_data;
TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

L3G gyro;

Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);
Adafruit_LSM303DLH_Mag_Unified mag = Adafruit_LSM303DLH_Mag_Unified(12345);

double fmap(double x, double in_min, double in_max, double out_min, double out_max)
{
	const double dividend = out_max - out_min;
	const double divisor = in_max - in_min;
	const double delta = x - in_min;
	if (divisor == 0)
	{
		log_e("Invalid map input range, min == max");
		return -1; //AVR returns -1, SAM returns 0
	}
	return (delta * dividend + (divisor / 2.0)) / divisor + out_min;
}

void Scanner ()
{

    	pinMode(ADC_EN, OUTPUT);
	digitalWrite(ADC_EN, HIGH);
	tft.init();
	tft.setRotation(3);
	tft.fillScreen(TFT_BLACK);

	tft.setTextSize(2);
	tft.setTextFont(1);
	tft.setTextColor(TFT_GREEN);
	tft.setTextDatum(MC_DATUM);

	  tft.setCursor(0, 0);


  Serial.println ();
  Serial.println ("I2C scanner. Scanning ...");
  tft.println ("I2C Scanning ...");
  byte count = 0;

  for (byte i = 8; i < 120; i++)
  {
    Wire.beginTransmission (i);          // Begin I2C transmission Address (i)
    if (Wire.endTransmission () == 0)  // Receive 0 = success (ACK response) 
    {
      Serial.print ("Found address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);     // PCF8574 7 bit address
      Serial.println (")");
      tft.print ("Found at: ");
      tft.print (i, DEC);
      tft.print (" (0x");
      tft.print (i, HEX);     // PCF8574 7 bit address
      tft.println (")");



      count++;
    }
  }
  Serial.print ("Found ");      
  Serial.print (count, DEC);        // numbers of devices
  Serial.println (" device(s).");
  tft.print ("Found ");      
  tft.print (count, DEC);        // numbers of devices
  tft.println (" device(s).");

}

void setup() {
    delay(500);

	Serial.begin(115200);
	Wire.begin(SDA, SCL);
Scanner ();
    delay(10000);
  // 	pinMode(ADC_EN, OUTPUT);
	// digitalWrite(ADC_EN, HIGH);
	// tft.init();
	// tft.setRotation(0);
	// tft.fillScreen(TFT_BLACK);

	// tft.setTextSize(1);
	// tft.setTextFont(1);
	// tft.setTextColor(TFT_GREEN);
	// tft.setTextDatum(MC_DATUM);

	//   tft.setCursor(0, 0);

  //   delay(1000);

  // put your setup code here, to run once:

	if (!gyro.init())
	{
		Serial.print("Failed to autodetect gyro type!");
	  
      	pinMode(ADC_EN, OUTPUT);
	digitalWrite(ADC_EN, HIGH);
	tft.init();
	tft.setRotation(0);
	tft.fillScreen(TFT_BLACK);

	tft.setTextSize(3);
	tft.setTextFont(1);
	tft.setTextColor(TFT_GREEN);
	tft.setTextDatum(MC_DATUM);
    tft.setCursor(0, 70);
    tft.print("Failed to init gyro !\n");
    // tft.fillScreen(TFT_BLACK);
		
    
			delay(1000);
			tft.fillScreen(TFT_BLACK);

			ESP.restart();

	}
	gyro.enableDefault();

    	pinMode(ADC_EN, OUTPUT);
	digitalWrite(ADC_EN, HIGH);
	tft.init();
	tft.setRotation(0);
	tft.fillScreen(TFT_BLACK);

	tft.setTextSize(1);
	tft.setTextFont(1);
	tft.setTextColor(TFT_GREEN);
	tft.setTextDatum(MC_DATUM);

	  tft.setCursor(0, 0);

}

void drawBatteryLevel(TFT_eSprite *sprite, int x, int y, float voltage)
{
	uint32_t color1 = TFT_GREEN;
	uint32_t color2 = TFT_WHITE;
	uint32_t color3 = TFT_BLUE;
	uint32_t color4 = TFT_RED;

	if (voltage > 4.33)
	{
		(*sprite).fillRect(x, y, 30, 10, color3);
	}
	else if (voltage > 3.2)
	{
		(*sprite).fillRect(x, y, map((long)(voltage * 100), 320, 430, 0, 30), 10, color1);
		(*sprite).setCursor(x + 7, y + 1);
		(*sprite).setTextColor(TFT_DARKGREY);
		(*sprite).printf("%02ld%%", map((long)(voltage * 100), 320, 432, 0, 100));
	}
	else
	{
		(*sprite).fillRect(x, y, 30, 10, color4);
	}

	(*sprite).drawRect(x, y, 30, 10, color2);
}

void compassArraw(TFT_eSPI tft, TFT_eSprite * sprite, int x, int y, float angle)
{
	TFT_eSprite direction = TFT_eSprite(&tft);
	direction.setColorDepth(8);
	direction.createSprite(50, 20);
	direction.fillRect(5, 5, 40, 10, TFT_BLUE);
	direction.fillTriangle(40,0,50,10,40,20,TFT_BLUE);
	direction.setPivot(25, 10);
	// direction.setRotation(60);
	
	
	// direction.pushSprite(50, 50);
	TFT_eSprite directionBack = TFT_eSprite(&tft);
	directionBack.setColorDepth(8);
	directionBack.createSprite(50, 50);

	direction.pushRotated(&directionBack, angle);
	directionBack.pushToSprite((TFT_eSprite*)sprite,(int32_t) x,(int32_t) y);
	(*sprite).drawCircle(x + 25, y + 25, 30, TFT_WHITE);
}

void drawCursors(TFT_eSprite *sprite, int x, int y, int w, int h, int min, int max, int value, uint32_t color)
{
	if (value < (max - min) / 2 + min)
	{
		(*sprite).fillRect(x, y + (h / 2) - map(value, (max - min) / 2 + min, min, 0, h / 2), w, map(value, (max - min) / 2 + min, min, 0, h / 2), color);
	}
	else
	{
		(*sprite).fillRect(x, y + (h / 2), w, map(value, (max - min) / 2 + min, max, 0, h / 2), color);
	}
	(*sprite).drawRect(x, y, w, h, TFT_WHITE);
}

void drawSensorsActivity(TFT_eSPI tft, t_sensors sensors)
{
	static bool isCalibrated = false;
	static int calMinX = 0;
	static int calMaxX = 0;
	static int calMinY = 0;
	static int calMaxY = 0;


	TFT_eSprite drawing_sprite = TFT_eSprite(&tft);
	drawing_sprite.setColorDepth(8);
	drawing_sprite.createSprite(tft.width(), tft.height());

	drawing_sprite.fillSprite(TFT_BLACK);
	drawing_sprite.setTextSize(1);
	drawing_sprite.setTextFont(1);
	drawing_sprite.setTextColor(TFT_GREEN);
	drawing_sprite.setTextDatum(MC_DATUM);
	drawing_sprite.setCursor(0, 0);

	uint16_t v = analogRead(ADC_PIN);
	float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (VREF / 1000.0);
	drawing_sprite.setTextColor(TFT_RED);
	drawing_sprite.printf("Voltage : ");
	drawing_sprite.setTextColor(TFT_WHITE);
	drawing_sprite.printf("%.2fv\n\n", battery_voltage);
	drawBatteryLevel(&drawing_sprite, 100, 00, battery_voltage);
	drawing_sprite.setTextColor(TFT_WHITE);


	// sensors_event_t accel_event;
	// sensors_event_t mag_event;
	// accel.getEvent(&accel_event);
	// mag.getEvent(&mag_event);
	// // mag.getSensor(&event);
	// // gyro.getEvent(&event);
	// gyro.read();


	// Calculate the angle of the vector y,x

	// Normalize to 0-360


	if (isCalibrated == false)
	{
		calMinX = sensors.mag.x;
		calMaxX = sensors.mag.x + 1;
		calMinY = sensors.mag.y;
		calMaxY = sensors.mag.y + 1;
		isCalibrated = true;
	}
	else
	{
		if (sensors.mag.x < calMinX)
			calMinX = sensors.mag.x;
		else if (sensors.mag.x > calMaxX)
			calMaxX = sensors.mag.x;
		if (sensors.mag.y < calMinY)
			calMinY = sensors.mag.y;
		else if (sensors.mag.y > calMaxY)
			calMaxY = sensors.mag.y;
	}


	int mag_xcal = map(sensors.mag.x, calMinX, calMaxX, -1000, 1000);
	int mag_ycal = map(sensors.mag.y, calMinY, calMaxY, -1000, 1000);
	float heading = atan2((double)mag_xcal, (double)mag_ycal);
	// if (heading < 0) {
	// 	heading = 360 + heading;
	// }

	compassArraw(tft, &drawing_sprite, 45, 175, (-(int)(heading * 180 / PI)));

	// drawing_sprite.setCursor(2, 15);
	// drawing_sprite.printf("osc addr : /%d", oscAddress);


	drawing_sprite.setCursor(7, 30);
	drawing_sprite.printf("Accel");
	drawing_sprite.setCursor(4, 45);
	drawing_sprite.printf("X");
	drawing_sprite.setCursor(19, 45);
	drawing_sprite.printf("Y");
	drawing_sprite.setCursor(34, 45);
	drawing_sprite.printf("Z");

	drawing_sprite.setCursor(56, 30);
	drawing_sprite.printf("Gyro");

	drawing_sprite.setCursor(49, 45);
	drawing_sprite.printf("X");
	drawing_sprite.setCursor(64, 45);
	drawing_sprite.printf("Y");
	drawing_sprite.setCursor(79, 45);
	drawing_sprite.printf("Z");


	drawing_sprite.setCursor(105, 30);
	drawing_sprite.printf("Mag");

	drawing_sprite.setCursor(94, 45);
	drawing_sprite.printf("X");
	drawing_sprite.setCursor(109, 45);
	drawing_sprite.printf("Y");
	drawing_sprite.setCursor(124, 45);
	drawing_sprite.printf("Z");

	drawCursors(&drawing_sprite, 0, 60, 12, 100, -40, 40, sensors.accel.x, TFT_RED);
	drawCursors(&drawing_sprite, 15, 60, 12, 100, -40, 40, sensors.accel.y, TFT_RED);
	drawCursors(&drawing_sprite, 30, 60, 12, 100, -40, 40, sensors.accel.z, TFT_RED);

	drawCursors(&drawing_sprite, 45, 60, 12, 100, -37000, 37000, sensors.gyro.x, TFT_RED);
	drawCursors(&drawing_sprite, 60, 60, 12, 100, -37000, 37000, sensors.gyro.y, TFT_RED);
	drawCursors(&drawing_sprite, 75, 60, 12, 100, -37000, 37000, sensors.gyro.z, TFT_RED);

	drawCursors(&drawing_sprite, 90, 60, 12, 100, -1000, 1000, mag_xcal, TFT_RED);
	drawCursors(&drawing_sprite, 105, 60, 12, 100, -1000, 1000, mag_ycal, TFT_RED);
	drawCursors(&drawing_sprite, 120, 60, 12, 100, -200, 200, sensors.mag.z, TFT_RED);
	// event.acceleration.
	
	float gyroscope = map(sqrtf(sensors.gyro.x * sensors.gyro.x \
		 + sensors.gyro.y * sensors.gyro.y \
		 + sensors.gyro.z * sensors.gyro.z), 0, 37000, 0, 50);
	float acceleration = map(sqrtf(sensors.accel.x * sensors.accel.x \
		 + sensors.accel.y * sensors.accel.y \
		 + sensors.accel.z * sensors.accel.z), 0, 40, 0, 50);
	
	drawing_sprite.setCursor(5, 170);
	drawing_sprite.printf("Accel");
	drawing_sprite.fillRect(10, 235 - acceleration, 20, acceleration, TFT_RED);
	drawing_sprite.drawRect(10, 185, 20, 50, TFT_WHITE);
	
	drawing_sprite.setCursor(108, 170);
	drawing_sprite.printf("Gyro");
	drawing_sprite.fillRect(110, 235 - gyroscope, 20, gyroscope, TFT_RED);
	drawing_sprite.drawRect(110, 185, 20, 50, TFT_WHITE);

	
	Serial.print("G ");
	Serial.print("X: ");
	Serial.print((int) sensors.gyro.x);
	Serial.print(" Y: ");
	Serial.print((int) sensors.gyro.y);
	Serial.print(" Z: ");
	Serial.println((int) sensors.gyro.z);


	drawing_sprite.pushSprite(0, 0);
	drawing_sprite.deleteSprite();


	float accel_x = fmap(sensors.accel.x, -40, 40, -100, 100);
	float accel_y = fmap(sensors.accel.y, -40, 40, -100, 100);
	float accel_z = fmap(sensors.accel.z, -40, 40, -100, 100);
	float gyro_x = fmap(sensors.gyro.x, -37000, 37000, -100, 100);
	float gyro_y = fmap(sensors.gyro.y, -37000, 37000, -100, 100);
	float gyro_z = fmap(sensors.gyro.z, -37000, 37000, -100, 100);
	float magnet_x = fmap(sensors.mag.x, -100, 100, -100, 100);
	float magnet_y = fmap(sensors.mag.y, -100, 100, -100, 100);
	float magnet_z = fmap(sensors.mag.z, -100, 100, -100, 100);

	float gyro_normal = map(sqrtf(sensors.gyro.x * sensors.gyro.x \
		 + sensors.gyro.y * sensors.gyro.y \
		 + sensors.gyro.z * sensors.gyro.z), 0, 37000, 0, 100);
	float accel_normal = map(sqrtf(sensors.accel.x * sensors.accel.x \
		 + sensors.accel.y * sensors.accel.y \
		 + sensors.accel.z * sensors.accel.z), 0, 40, 0, 100);





}

void update_sensors(t_sensors *sensors)
{
	sensors_event_t accel_event;
	sensors_event_t mag_event;
	accel.getEvent(&accel_event);
	mag.getEvent(&mag_event);
	gyro.read();

	// sensors->accel.x = fmap(accel_event.acceleration.x, -40, 40, -100, 100);
	// sensors->accel.y = fmap(accel_event.acceleration.y, -40, 40, -100, 100);
	// sensors->accel.z = fmap(accel_event.acceleration.z, -40, 40, -100, 100);
	// sensors->gyro.x = fmap(gyro.g.x, -37000, 37000, -100, 100);
	// sensors->gyro.y = fmap(gyro.g.y, -37000, 37000, -100, 100);
	// sensors->gyro.z = fmap(gyro.g.z, -37000, 37000, -100, 100);
	// sensors->mag.x = fmap(mag_event.magnetic.x, -100, 100, -100, 100);
	// sensors->mag.y = fmap(mag_event.magnetic.y, -100, 100, -100, 100);
	// sensors->mag.z = fmap(mag_event.magnetic.z, -100, 100, -100, 100);

	sensors->accel.x = accel_event.acceleration.x;
	sensors->accel.y = accel_event.acceleration.y;
	sensors->accel.z = accel_event.acceleration.z;
	sensors->gyro.x = gyro.g.x;
	sensors->gyro.y = gyro.g.y;
	sensors->gyro.z = gyro.g.z;
	sensors->mag.x = mag_event.magnetic.x;
	sensors->mag.y = mag_event.magnetic.y;
	sensors->mag.z = mag_event.magnetic.z;


	// Serial.printf("Acc  : %02f | %02f | %02f\n", sensors->accel.x, sensors->accel.y, sensors->accel.z);
	// Serial.printf("Gyro : %02f | %02f | %02f\n", sensors->gyro.x, sensors->gyro.y, sensors->gyro.z);
	// Serial.printf("Mag  : %02f | %02f | %02f\n", sensors->mag.x, sensors->mag.y, sensors->mag.z);



}

void loop() {
  update_sensors(&sensors_data);
  drawSensorsActivity(tft, sensors_data);
  // put your main code here, to run repeatedly:
}