#include <avr/sleep.h> // see https://nongnu.org/avr-libc/user-manual/group__avr__sleep.html

// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include "RTClib.h"

#define INTERRUPT_PIN   (2)

RTC_DS3231 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};



void setup() {
  Serial.begin(115200); // start serial communicastion

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {  
    Serial.println("RTC lost power, lets set the time!");
    // If the RTC have lost power it will sets the RTC to the date & time this sketch was compiled in the following line
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  
  // Use the builtin led to indicate when arduino is asleep
  pinMode(LED_BUILTIN, OUTPUT);

  // set pin D2 to input using the builtin pullup resistor
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);

  // turn led on (not sleeping now)
  digitalWrite(LED_BUILTIN, HIGH); 

  Serial.println("Started");
  printDate();
}

void printDate() {
  DateTime now = rtc.now();

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
}

void wakeUp() {
  Serial.println("Interrupt received");
  sleep_disable();

  // remove the interrupt
  detachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN));
}


void goToSleep() {
  // Enable sleep mode (
  sleep_enable();

  // attach interrupt pin to D2, trigger when pin is high
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), wakeUp, HIGH);

  // full sleep
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  // turn led off
  digitalWrite(LED_BUILTIN, LOW);

  // wait 1 second
  Serial.println("Going to sleep now!");
  printDate();
  delay(1000); 

  // attach interrupt pin to D2, trigger when pin is low
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), wakeUp, LOW);

  // set an alarm for 30 seconds
  rtc.clearAlarm(1); // IMPORTANT!
  DateTime alarmTime (rtc.now().unixtime() + 30);
  if (rtc.setAlarm1(alarmTime, DS3231_A1_Date)) {
    // activate sleep mode
    sleep_cpu();

    // this code is run after device wakeup
    Serial.println("Wake up!");
    printDate();

    
    digitalWrite(LED_BUILTIN, HIGH);
    rtc.disableAlarm(1);
  }
}

void loop() {
  // wait 5 seconds then go to sleep
  delay(5000);
  goToSleep();
}
