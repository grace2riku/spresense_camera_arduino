#include "gpio.h"

const byte switch_1_pin = 7;
const byte switch_2_pin = 4;
const byte user_led_1_pin = 6;
const byte user_led_2_pin = 5;

bool capture_start = false;
bool exit_app = false;


static void gpio_switch_1_handler() {
  byte sw1_status = digitalRead(switch_1_pin);
  digitalWrite(user_led_1_pin, sw1_status);
  if (!sw1_status) exit_app = true;
}

static void gpio_switch_2_handler() {
  byte sw2_status = digitalRead(switch_2_pin);
  digitalWrite(user_led_2_pin, sw2_status);
  if (!sw2_status) capture_start = true;
}

void gpio_init() {
  pinMode(switch_1_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(switch_1_pin), gpio_switch_1_handler, CHANGE);
  pinMode(switch_2_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(switch_2_pin), gpio_switch_2_handler, CHANGE);

  pinMode(user_led_1_pin, OUTPUT);
  pinMode(user_led_2_pin, OUTPUT);  
}
