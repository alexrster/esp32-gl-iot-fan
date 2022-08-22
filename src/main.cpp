#include <Arduino.h>
#include <IRremote.h>
#include "SwitchRelay.h"

#define IR_PIN 15

#define IR_CMD_DELAY 330
#define IR_CMD_TRESHOLD 100

IRrecv irrecv(IR_PIN);
decode_results results;
uint64_t ir_prev_value = 0;
uint8_t last_relay_number = 0;
bool current_state = false;

SwitchRelayPin relay1(5, 0);
SwitchRelayPin relay2(16, 0);
SwitchRelayPin relay3(19, 0);
SwitchRelayPin relay4(21, 0);

unsigned long 
  last_ir_cmd = 0,
  now = 0;

void setRelay(uint8_t relay_number, bool state = true) {
  if (last_relay_number == relay_number && state == current_state) return;

  current_state = state;
  
  relay1.setOff();
  relay2.setOff();
  relay3.setOff();
  relay4.setOff();

  if (state) {
    last_relay_number = relay_number;
    delay(50);

    switch (relay_number) {
      case 1: relay1.setOn(); break;
      case 2: relay2.setOn(); break;
      case 3: relay3.setOn(); break;
      case 4: relay4.setOn(); break;
    }
  }
}

void setup() {
  log_d("INIT");
  pinMode(IR_PIN, INPUT);

  irrecv.enableIRIn();

  log_d("READY!");
}

void loop() {
  now = millis();

  if (now - last_ir_cmd > IR_CMD_DELAY && irrecv.decode(&results)) {
    if (now - last_ir_cmd - IR_CMD_DELAY < IR_CMD_TRESHOLD && results.value == ir_prev_value) {
      irrecv.resume();
      return;
    }
    last_ir_cmd = now;
    ir_prev_value = results.value;
    log_d("IR code: %#08x", results.value);

    switch (results.value) {
      case 0xE13DDA28:
      case 0xE0E020DF: log_d("FAN SPEED: 1"); setRelay(1); break;                   // 1
      case 0xAD586662:
      case 0xE0E0A05F: log_d("FAN SPEED: 2"); setRelay(3); break;                   // 2
      case 0x273009C4:
      case 0xE0E0609F: log_d("FAN SPEED: 3"); setRelay(4); break;                   // 3
      // case 0xF5999288:
      // case 0xE0E010EF: log_d("FAN SPEED: 4"); setRelay(4); break;                   // 4
      // case 0x731A3E02:
      // case 0xE0E0906F: log_d("FAN SPEED: 5"); setRelay(1, false); break;            // 5
      case 0xE0E040BF:
      case 0xF4BA2988:                                                              // PWR
        log_d("FAN SPEED: [Toggle]"); 
        setRelay(last_relay_number ? last_relay_number : 1, !current_state); 
        break;
    }

    irrecv.resume();
  }

  delay(1);
}
