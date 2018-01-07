
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GPS.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

// Set to false to display time in 12 hour format, or true to use 24 hour:
#define TIME_24_HOUR      false

// neopixels ---------------------------------------------

#define PIN            6
#define NUMPIXELS      12
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
int HOUR_OFFSET = 0;

// button ---------------------------------------------
int buttonInput = 10;
int buttonState = 0;

Adafruit_GPS gps(&Serial1);

uint32_t flora = pixels.Color(0, 8, 2, 0);
uint32_t grapefruit = pixels.Color(20, 2, 2, 0);
uint32_t flame = pixels.Color(20, 4, 0, 0);
uint32_t cobalt = pixels.Color(0, 2, 8, 0);


void setup() {

  // neopixels
  pixels.begin(); // This initializes the NeoPixel library.

  // button
  pinMode(buttonInput, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.println("Clock starting!");

  // Setup the GPS using a 9600 baud connection (the default for most
  // GPS modules).
  gps.begin(9600);

  // Configure GPS to onlu output minimum data (location, time, fix).
  gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);

  // Use a 1 hz, once a second, update rate.
  gps.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);

  // Enable the interrupt to parse GPS data.
  enableGPSInterrupt();
}

void loop() {
  buttonState = digitalRead(buttonInput);
  if (buttonState == LOW) {
    HOUR_OFFSET += 1;
    if (HOUR_OFFSET > 23) {
      HOUR_OFFSET = 0;
    }
    setTime();
    delay(500);
  }
  if (buttonState == HIGH) {
    setTime();
  }
}
void setTime() {

  // Check if GPS has new data and parse it.
  if (gps.newNMEAreceived()) {
    gps.parse(gps.lastNMEA());
  }
  Serial.println("------");
  Serial.println(gps.hour);
  Serial.println(HOUR_OFFSET);
  Serial.println("------");
  // Grab the current hours, minutes, seconds from the GPS.
  // This will only be set once the GPS has a fix!  Make sure to add
  int hours = gps.hour + HOUR_OFFSET;
  if (hours < 0) {
    hours = 24 + hours;
  }
  if (hours > 23) {
    hours = hours - 24;
  }
  int minutes = gps.minute;
  minutes = (minutes * 12) / 60;
  minutes = minutes - 1;
  int seconds = gps.seconds;

  // Do 24 hour to 12 hour format conversion when required.
  if (!TIME_24_HOUR) {
    // Handle when hours are past 12 by subtracting 12 hours (1200 value).
    if (hours > 12) {
      hours -= 12;
    }
    // Handle hour 0 (midnight) being shown as 12.
    else if (hours == 0) {
      hours += 12;
    }

    Serial.println("------");
    Serial.println(hours);
    Serial.println(minutes);
  }

  if (minutes == -1) {
    for (int i = 0; i < NUMPIXELS; i++) {

      // no data - display blue
      pixels.setPixelColor(i, cobalt);
      pixels.show(); // This sends the updated pixel color to the hardware.
    }
  } else {
    for (int i = 0; i < NUMPIXELS; i++) {

      // unused pixels are blue
      pixels.setPixelColor(i, cobalt);
    }
    for (int i = 0; i < hours; i++) {

      // hours are pink
      pixels.setPixelColor(i, grapefruit);
    }

    // minute pixel is green
    pixels.setPixelColor(minutes, flame);

    // blink the 12
    if (seconds % 2 == 0) {
      pixels.setPixelColor(11, flora);
    }
    pixels.show(); // This sends the updated pixel color to the hardware.
  }
}

SIGNAL(TIMER0_COMPA_vect) {

  // Use a timer interrupt once a millisecond to check for new GPS data.
  // This piggybacks on Arduino's internal clock timer for the millis()
  // function.
  gps.read();
}

void enableGPSInterrupt() {

  // Function to enable the timer interrupt that will parse GPS data.
  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle and call the "Compare A" function above
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
}
