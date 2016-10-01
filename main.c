#define F_CPU 8000000UL // speed on mcu crystal (20Mhz)
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void display_digit(uint8_t digit, uint8_t decimal_point_bit);
void ports_init(void);
void interrupts_init(void);
void timer_init(void);
void display_toggle(uint8_t toggle_bit);
void display_function(void);

volatile uint8_t tot_overflow;

volatile uint8_t toggle = 0;

volatile uint8_t value_displayed = 0; // 0 = temperature, 1 = humidity, 2 = time (hour:minutes), 3 = date (day.month), 4 = year

volatile uint8_t dp_off = 0;
volatile uint8_t dp_on = 1;

volatile uint8_t zero = 0;
volatile uint8_t one = 1;
volatile uint8_t two = 2;
volatile uint8_t three = 3;
volatile uint8_t four = 4;
volatile uint8_t five = 5;
volatile uint8_t six = 6;
volatile uint8_t seven = 7;
volatile uint8_t eight = 8;
volatile uint8_t nine = 9;
volatile uint8_t c = 10;
volatile uint8_t h = 11;

int main(void) { // main program

  ports_init();
  interrupts_init();
  timer_init();

  sei(); // Enable global interrupts

  while(1){ // loop forever
  }
}

//Interrupt Service Routine for INT1 : Toggle between functions (show temperature, humidity, time, date, year)
ISR(INT1_vect) {
  value_displayed++;
  if (value_displayed == 5){
    value_displayed = 0;
  }
}

//ISR(TIMER1_OVF_vect){
ISR(TIMER1_COMPA_vect){
//  tot_overflow++;
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
  TCCR1B |= (1 << CS10) | (1 << WGM12);
  OCR1A = 15000;
  TIMSK1 |= (1 << OCIE1A);
//  TCCR1B |= (1 << CS10); // Set up timer with prescaler Fcpu/1 (no prescaler)
  TCNT1 = 0; // Initialize Timer 1 counter at 0
//  TIMSK1 |= (1 << TOIE1); // Enable overflow interrupt
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
    display_digit(one,dp_off);
    toggle = 2;
  }else if (toggle == 2){ // Display 2 (middle left) ON
    display_toggle(toggle);
    display_digit(two,dp_on);
    toggle = 3;
  }else if (toggle == 3){ // Display 3 (middle right) ON
    display_toggle(toggle);
    display_digit(three,dp_off);
    toggle = 4;
  }else if (toggle == 4){ // Display 4 (most right) ON
    display_toggle(toggle);
    display_digit(h,dp_off);
    toggle = 1;
  }else{ // If toggle is not equal any of 0,1,2,3,4, then make it equal to 0
    toggle = 0;
  }
}
