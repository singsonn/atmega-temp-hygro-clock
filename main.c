#define F_CPU 8000000UL // speed on mcu crystal (20Mhz)
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "DHT22int.h"
#include "rtc.h"
#include "twi.h"

void display_digit(uint8_t digit, uint8_t decimal_point_bit);
void ports_init(void);
void interrupts_init(void);
void timer_init(void);
void display_toggle(uint8_t toggle_bit);
void display_function(void);
void dht_reading_func(void);
void value_displayed_func(void);
void read_rtc(void);

uint8_t EEMEM DST = 1; // Variable for daylight saving time (0: no we are not in DST, 1: yes we are in DST)
uint8_t EEMEM REGION = 0; // Variable for region (0: US/CA, 1:EU)

volatile uint8_t toggle = 0;
volatile uint8_t dht_reading = 0;

volatile uint8_t value_displayed = 0; // 0 = temperature, 1 = humidity, 2 = time (hour:minutes), 3 = date (day.month), 4 = year

volatile uint8_t dp_value_one = 0;
volatile uint8_t dp_value_two = 0;
volatile uint8_t dp_value_three = 0;
volatile uint8_t dp_value_four = 0;

volatile uint8_t dp_dht_value_one = 0;
volatile uint8_t dp_dht_value_two = 0;
volatile uint8_t dp_dht_value_three = 0;
volatile uint8_t dp_dht_value_four = 0;

volatile uint8_t one = 14;
volatile uint8_t two = 14;
volatile uint8_t three = 14;
volatile uint8_t four = 14;

volatile uint8_t dht_temp_one = 14;
volatile uint8_t dht_temp_two = 14;
volatile uint8_t dht_temp_three = 14;
volatile uint8_t dht_temp_four = 14;

volatile uint8_t dht_humidity_one = 14;
volatile uint8_t dht_humidity_two = 14;
volatile uint8_t dht_humidity_three = 14;
volatile uint8_t dht_humidity_four = 14;

volatile uint8_t year_one = 2;
volatile uint8_t year_two = 0;
volatile uint8_t year_three = 16;
volatile uint8_t year_four = 16;

volatile uint8_t hour_one = 16;
volatile uint8_t hour_two = 16;
volatile uint8_t min_three = 16;
volatile uint8_t min_four = 16;

volatile uint8_t time_dp_value_two = 0;

volatile uint8_t day_of_week = 0;
volatile uint8_t month = 0;
volatile uint8_t day = 0;
volatile uint8_t hour = 0;
volatile uint8_t minute = 0;
volatile uint16_t year = 0;

volatile uint8_t day_one = 16;
volatile uint8_t day_two = 16;
volatile uint8_t day_three = 16;
volatile uint8_t day_four = 16;

volatile struct tm* t = NULL;

//c = 10;
//h = 11;
//e = 12;
//f = 13;
//- = 14;
//° = 15;
//(all off) = 16;

int main(void) { // main program

  ports_init();
  interrupts_init();
  timer_init();

  DHT22_Init();

  sei(); // Enable global interrupts

  read_rtc();

  while(1){ // loop forever
    value_displayed_func();
  }
}

//Interrupt Service Routine for INT1 : Toggle between functions (show temperature, humidity, time, date, year)
ISR(INT1_vect) {
  dht_reading = 0;
  value_displayed++;
  if (value_displayed >= 5){ // >= 5 normalement
    value_displayed = 0;
  }
  if (value_displayed == 0){ // Display temperature
    one = dht_temp_one;
    two = dht_temp_two;
    three = dht_temp_three;
    four = dht_temp_four;
    dp_value_one = dp_dht_value_one;
    dp_value_two = dp_dht_value_two;
    dp_value_three = dp_dht_value_three;
    dp_value_four = dp_dht_value_four;
  }else if (value_displayed == 1){ // Display humidity
    one = dht_humidity_one;
    two = dht_humidity_two;
    three = dht_humidity_three;
    four = dht_humidity_four;
    dp_value_one = dp_dht_value_one;
    dp_value_two = dp_dht_value_two;
    dp_value_three = dp_dht_value_three;
    dp_value_four = dp_dht_value_four;
  }else if (value_displayed == 2){ // Display time (hour.minutes)
    one = hour_one;
    two = hour_two;
    three = min_three;
    four = min_four;
    dp_value_one = 0;
    dp_value_two = time_dp_value_two;
    dp_value_three = 0;
    dp_value_four = 0;
  }else if (value_displayed == 3){ // Display date (day.month)
    one = day_one;
    two = day_two;
    three = day_three;
    four = day_four;
    dp_value_one = 0;
    dp_value_two = 1;
    dp_value_three = 0;
    dp_value_four = 0;
  }else if (value_displayed == 4){ // Display year
    one = year_one;
    two = year_two;
    three = year_three;
    four = year_four;
    dp_value_one = 0;
    dp_value_two = 0;
    dp_value_three = 0;
    dp_value_four = 0;
  }else{ // Default : display temperature
    one = dht_temp_one;
    two = dht_temp_two;
    three = dht_temp_three;
    four = dht_temp_four;
    dp_value_one = dp_dht_value_one;
    dp_value_two = dp_dht_value_two;
    dp_value_three = dp_dht_value_three;
    dp_value_four = dp_dht_value_four;
  }
}

ISR(TIMER1_COMPA_vect){
  if (value_displayed == 2){
    if ( dp_value_four == 0){
      dp_value_four = 1;
    }else{
      dp_value_four = 0;
    }
  }
}

ISR(TIMER0_COMPA_vect){
  display_function();
}

void display_digit(uint8_t digit, uint8_t decimal_point_bit) {
  switch(decimal_point_bit){
    case 0:
      PORTD &= ~(1<<PD0);
      break;
    case 1:
      PORTD |= (1<<PD0);
      break;
    default:
      PORTD &= ~(1<<PD0);
      break;
  }
  switch(digit){
    case 0: // Display Number 0
      PORTB |= (1<<PB1) | (1<<PB2);
      PORTC |= (1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3);
      PORTD &= ~(1<<PD4);
      break;
    case 1: // Display Number 1
      PORTB &= ~((1<<PB1) | (1<<PB2));
      PORTC |= (1<<PC1) | (1<<PC2);
      PORTC &= ~((1<<PC0) | (1<<PC3));
      PORTD &= ~(1<<PD4);
      break;
    case 2: // Display Number 2
      PORTB |= (1<<PB1);
      PORTB &= ~(1<<PB2);
      PORTC |= (1<<PC0) | (1<<PC1) | (1<<PC3);
      PORTC &= ~(1<<PC2);
      PORTD |= (1<<PD4);
      break;
    case 3: // Display Number 3
      PORTB &= ~((1<<PB1) | (1<<PB2));
      PORTC |= (1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3);
      PORTD |= (1<<PD4);
      break;
    case 4: // Display Number 4
      PORTB |= (1<<PB2);
      PORTB &= ~(1<<PB1);
      PORTC |= (1<<PC1) | (1<<PC2);
      PORTC &= ~((1<<PC0) | (1<<PC3));
      PORTD |= (1<<PD4);
      break;
    case 5: // Display Number 5
      PORTB |= (1<<PB2);
      PORTB &= ~(1<<PB1);
      PORTC |= (1<<PC0) | (1<<PC2) | (1<<PC3);
      PORTC &= ~(1<<PC1);
      PORTD |= (1<<PD4);
      break;
    case 6: // Display Number 6
      PORTB |= (1<<PB1) | (1<<PB2);
      PORTC |= (1<<PC0) | (1<<PC2) | (1<<PC3);
      PORTC &= ~(1<<PC1);
      PORTD |= (1<<PD4);
      break;
    case 7: // Display Number 7
      PORTB &= ~((1<<PB1) | (1<<PB2));
      PORTC |= (1<<PC0) | (1<<PC1) | (1<<PC2);
      PORTC &= ~(1<<PC3);
      PORTD &= ~(1<<PD4);
      break;
    case 8: // Display Number 8
      PORTB |= (1<<PB1) | (1<<PB2);
      PORTC |= (1<<PC0) | (1<<PC2) | (1<<PC1) | (1<<PC3);
      PORTD |= (1<<PD4);
      break;
    case 9: // Display Number 9
      PORTB |= (1<<PB2);
      PORTB &= ~(1<<PB1);
      PORTC |= (1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3);
      PORTD |= (1<<PD4);
      break;
    case 10: // Display char "C"
      PORTB |= (1<<PB1) | (1<<PB2);
      PORTC |= (1<<PC0) | (1<<PC3);
      PORTC &= ~((1<<PC1) | (1<<PC2));
      PORTD &= ~(1<<PD4);
      break;
    case 11: // Display char "H"
      PORTB |= (1<<PB1) | (1<<PB2);
      PORTC |= (1<<PC1) | (1<<PC2);
      PORTC &= ~((1<<PC0) | (1<<PC3));
      PORTD |= (1<<PD4);
      break;
    case 12: // Display char "E"
      PORTB |= (1<<PB1) | (1<<PB2);
      PORTC |= (1<<PC0) | (1<<PC3);
      PORTC &= ~((1<<PC1) | (1<<PC2));
      PORTD |= (1<<PD4);
      break;
    case 13: // Display char "F"
      PORTB |= (1<<PB1) | (1<<PB2);
      PORTC |= (1<<PC0);
      PORTC &= ~((1<<PC1) | (1<<PC2) | (1<<PC3));
      PORTD |= (1<<PD4);
      break;
    case 14: // Display char "-"
      PORTB &= ~((1<<PB1) | (1<<PB2));
      PORTC &= ~((1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3));
      PORTD |= (1<<PD4);
      break;
    case 15: // Display char "°"
      PORTB |= (1<<PB2);
      PORTB &= ~(1<<PB1);
      PORTC |= (1<<PC0) | (1<<PC1);
      PORTC &= ~((1<<PC2) | (1<<PC3));
      PORTD |= (1<<PD4);
      break;
    case 16: // Display all OFF
      PORTB &= ~((1<<PB1) | (1<<PB2));
      PORTC &= ~((1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3));
      PORTD &= ~(1<<PD4);
      break;
    default: // Display Number 8
      PORTB |= (1<<PB1) | (1<<PB2);
      PORTC |= (1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3);
      PORTD |= (1<<PD4);
      break;
  }
}

void ports_init(void){
  DDRB |= (1<<DDB0) | (1<<DDB1) | (1<<DDB2); // Set portB0-1-2 as output
  DDRC |= (1<<DDC0) | (1<<DDC1) | (1<<DDC2) | (1<<DDC3); // Set PortC0-1-2-3 as output
  DDRD |= (1<<DDD0) | (1<<DDD4) | (1<<DDD5) | (1<<DDD6) | (1<<DDD7); // Set PortD0-4-5-6-7 as output
  PORTB &= ~((1<<PB0) | (1<<PB1) | (1<<PB2)); // Initialize PORTB 0-1-2 at 0
  PORTC &= ~((1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3)); // Initialize PORTC0-1-2-3 at 0
  PORTD &= ~((1<<PD0) | (1<<PD4) | (1<<PD5) | (1<<PD6) | (1<<PD7)); // Initialize PORTD0-4-5-6-7 at 0
}

void interrupts_init(void){
  EICRA |= (1 << ISC11) | (1 << ISC10);    // set INT1 to trigger on RISING EDGE logic change
  EIMSK |= (1 << INT1);     // Turns on INT1 (for display toggle btw temp, humidity, time, date, year)
}

void timer_init(void){
  // Timer 1 for single dot toggle
  TCCR1B |= (1 << CS10) | (1 << CS12) | (1 << WGM12);
  OCR1A = 7811;
  TIMSK1 |= (1 << OCIE1A);
  TCNT1 = 0; // Initialize Timer 1 counter at 0

  // Timer 0 for 7-segment toggle
  TCCR0A |= (1 << WGM01);
  TCCR0B |= (1 << CS02);
  OCR0A = 50;
  TIMSK0 |= (1 << OCIE0A);
  TCNT0 = 0; // Initialize Timer 0 counter at 0
}

void display_toggle(uint8_t toggle_bit){
  switch(toggle_bit){
    case 0: // All 7-seg displays OFF
      PORTB &= ~(1<<PB0);
      PORTD &= ~((1<<PD5) | (1<<PD6) | (1<<PD7));
      break;
    case 1: // Top left ON, all others OFF
      PORTB |= (1<<PB0);
      PORTD &= ~((1<<PD5) | (1<<PD6) | (1<<PD7));
      break;
    case 2: // 2nd left (middle left) ON, all others OFF
      PORTB &= ~(1<<PB0);
      PORTD &= ~((1<<PD5) | (1<<PD6));
      PORTD |= (1<<PD7);
      break;
    case 3: // 2nd right (middle right) ON, all others OFF
      PORTB &= ~(1<<PB0);
      PORTD &= ~((1<<PD5) | (1<<PD7));
      PORTD |= (1<<PD6);
      break;
    case 4: // Top right ON, all others OFF
      PORTB &= ~(1<<PB0);
      PORTD &= ~((1<<PD6) | (1<<PD7));
      PORTD |= (1<<PD5);
      break;
    default: // All 7-seg displays OFF
      PORTB &= ~(1<<PB0);
      PORTD &= ~((1<<PD5) | (1<<PD6) | (1<<PD7));
      break;
  }
}

void display_function(void){
  if (toggle == 0){ // All Displays off
    display_toggle(toggle);
    toggle = 1;
  }else if (toggle == 1){ // Display 1 (most left) ON
    display_toggle(toggle);
    display_digit(one,dp_value_one);
    toggle = 2;
  }else if (toggle == 2){ // Display 2 (middle left) ON
    display_toggle(toggle);
    display_digit(two,dp_value_two);
    toggle = 3;
  }else if (toggle == 3){ // Display 3 (middle right) ON
    display_toggle(toggle);
    display_digit(three,dp_value_three);
    toggle = 4;
  }else if (toggle == 4){ // Display 4 (most right) ON
    display_toggle(toggle);
    display_digit(four,dp_value_four);
    toggle = 1;
  }else{ // If toggle is not equal any of 0,1,2,3,4, then make it equal to 0
    toggle = 0;
  }
}

void dht_reading_func(void){
  DHT22_STATE_t state;
  DHT22_DATA_t sensor_data;
  state = DHT22_StartReading();
  state = DHT22_CheckStatus(&sensor_data);
  if (state == DHT_DATA_READY){
     // Do something with the data.
     // sensor_data.temperature_integral
     // sensor_data.temperature_decimal
     // sensor_data.humidity_integral
     // sensor_data.humidity_decimal
     dht_temp_one = sensor_data.temperature_integral / 10;
     dht_temp_two = sensor_data.temperature_integral % 10;
     dht_temp_three = sensor_data.temperature_decimal;
     dht_temp_four = 15;
     dht_humidity_one = sensor_data.humidity_integral / 10;
     dht_humidity_two = sensor_data.humidity_integral % 10;
     dht_humidity_three = sensor_data.humidity_decimal;
     dht_humidity_four = 11;
     dp_dht_value_one = 0;
     dp_dht_value_two = 1;
     dp_dht_value_three = 0;
     dp_dht_value_four = 0;
     }
  else if (state == DHT_ERROR_CHECKSUM){
     // Do something if there is a Checksum error
     }
  else if (state == DHT_ERROR_NOT_RESPOND){
     // Do something if the sensor did not respond
   }
}

void value_displayed_func(void){
  if (value_displayed == 0){ // Display temperature
    if (dht_reading == 0){
      dht_reading_func();
      one = dht_temp_one;
      two = dht_temp_two;
      three = dht_temp_three;
      four = dht_temp_four;
      dp_value_one = dp_dht_value_one;
      dp_value_two = dp_dht_value_two;
      dp_value_three = dp_dht_value_three;
      dp_value_four = dp_dht_value_four;
    }
    dht_reading = 1;
    if (dht_reading >= 1){
      _delay_ms(2000);
      read_rtc();
      dht_reading = 0;
    }
  }else if (value_displayed == 1){ // Display humidity
    if (dht_reading == 0){
      dht_reading_func();
      one = dht_humidity_one;
      two = dht_humidity_two;
      three = dht_humidity_three;
      four = dht_humidity_four;
      dp_value_one = dp_dht_value_one;
      dp_value_two = dp_dht_value_two;
      dp_value_three = dp_dht_value_three;
      dp_value_four = dp_dht_value_four;
    }
    dht_reading = 1;
    if (dht_reading >= 1){
      _delay_ms(2000);
      read_rtc();
      dht_reading = 0;
    }
  }else if (value_displayed == 2){ // Display time (hour.minutes)
    read_rtc();
    one = hour_one;
    two = hour_two;
    three = min_three;
    four = min_four;
    dp_value_one = 0;
    dp_value_two = time_dp_value_two;
    dp_value_three = 0;
//    dp_value_four = 0;
    _delay_ms(2);
  }else if (value_displayed == 3){ // Display date (day.month)
    read_rtc();
    one = day_one;
    two = day_two;
    three = day_three;
    four = day_four;
    dp_value_one = 0;
    dp_value_two = 1;
    dp_value_three = 0;
    dp_value_four = 0;
    _delay_ms(100);
  }else if (value_displayed == 4){ // Display year
    read_rtc();
    one = year_one;
    two = year_two;
    three = year_three;
    four = year_four;
    dp_value_one = 0;
    dp_value_two = 0;
    dp_value_three = 0;
    dp_value_four = 0;
    _delay_ms(1000);
  }else{ // Default : display temperature
    if (dht_reading == 0){
      dht_reading_func();
      one = dht_temp_one;
      two = dht_temp_two;
      three = dht_temp_three;
      four = dht_temp_four;
      dp_value_one = dp_dht_value_one;
      dp_value_two = dp_dht_value_two;
      dp_value_three = dp_dht_value_three;
      dp_value_four = dp_dht_value_four;
    }
    dht_reading = 1;
    if (dht_reading >= 1){
      _delay_ms(2000);
      read_rtc();
      dht_reading = 0;
    }
  }
}

void read_rtc(void){
  t = rtc_get_time();
  hour = t->hour;
  hour_one = t->hour / 10;
  hour_two = t->hour % 10;
  minute = t->min;
  min_three = t->min / 10;
  min_four = t->min % 10;
  day = t->mday;
  day_one = t->mday / 10;
  day_two = t->mday % 10;
  month = t->mon;
  day_three = t->mon / 10;
  day_four = t->mon % 10;
  year = t->year;
  year = year - 1900;
  year_three = year / 10;
  year_four = year % 10;
  day_of_week = t->wday;
  time_dp_value_two = 1;
  if (REGION == 0){ // REGION = US/CA
    if (day_of_week == 7 && month == 3 && day >= 25 && day <=31 && hour ==2 && DST==0){ // Beginning of DST
      // set RTC clock +1 h
      t->hour = hour + 1;
      rtc_set_time(t);
      DST=1;
    }
    if (day_of_week == 7 && month == 10 && day >= 25 && day <=31 && hour == 3 && DST==1){ // End of DST
      // set RTC clock -1 h
      t->hour = hour - 1;
      rtc_set_time(t);
      DST=0;
    }
  }else { // REGION = EU

  }

}
