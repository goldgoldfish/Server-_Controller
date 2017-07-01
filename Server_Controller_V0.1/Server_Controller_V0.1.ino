/*
 * Server_Controller
 * Ben Wedemire
 * 
 * Description: Not yet!
 */

//declaring pin variables
const int power_on_pin = 2;
const int power_state_pin = 4;
const int power_button_pin = 10;

//declaring global variables
volatile boolean power_state = 0;

//declaring void functions
void power_state_change();

void setup() {
  pinMode(power_on_pin, INPUT);
  pinMode(power_state_pin, OUTPUT);
  pinMode(power_button_pin, OUTPUT);

  Serial.begin(9600);

  attachInterrupt(digitalPinToInterrupt(power_on_pin), power_state_change, CHANGE);
  Serial.println("Done Setup");
} //end setup

void loop() {
  int power_LED_statues = 0; //var for the current statues of the power led
  while(1) {
    if (power_LED_statues != power_state) {
      digitalWrite(power_state_pin, power_state); //upadate the power statues led
      power_LED_statues = power_state; //update power led statues
      if (power_state) Serial.println("Server is Powered");
      else if (!power_state) Serial.println("Server is Powered Down");
      else Serial.println("Error");
    } //end if
  } //end while
} //end loop

void power_state_change(){
  if(digitalRead(power_on_pin)) power_state = 1; //if the server power is on power state is true
  else power_state = 0; //if the server power is off power state is false
} //end power_state_change

