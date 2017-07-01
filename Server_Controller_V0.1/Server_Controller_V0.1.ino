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
const byte func[3] = {89, 90, 91};

//declaring void functions
void power_state_change(void);
void clear_LCD(void);
void Serialdump(void);

//declaring returning functions
byte get_data(void);
byte do_cmmd(byte cmmd);
byte check_cmmd(byte cmmd);

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
  byte cmmd = 0; //stores the value of any requested command
  if(digitalRead(power_on_pin)) {
    power_state = 1; //if the server power is on power state is true
  } //end if
  else {
    power_state = 0; //if the server power is off power state is false
    Serial.println("Server is Down");
  } //end else
  while(1) {
    if (power_LED_statues != power_state) {
      digitalWrite(power_state_pin, power_state); //upadate the power statues led
      power_LED_statues = power_state; //update power led statues
      if (power_state) {
        clear_LCD();
        Serial.println("Server is Up");
      } //end if
      else if (!power_state) {
        clear_LCD();
        Serial.println("Server is Down");
      } //end else if
      else {
        clear_LCD();
        Serial.println("Error");
      } //end else
    } //end if
    cmmd = get_data(); //gets the most recent cmmd
    if (cmmd) {
      clear_LCD();
      if (do_cmmd(cmmd)) { //check and do the cmmd
        Serial.println("Command Executed"); //display outcome of do_cmmd
      } //end if
      else {
        Serial.println("Error in Commnd!");
      } //end else
    } //end if
    //Serialdump(); //dump the rest of the serial buffer
  } //end while
} //end loop

void power_state_change(void){
  if(digitalRead(power_on_pin)) power_state = 1; //if the server power is on power state is true
  else power_state = 0; //if the server power is off power state is false
} //end power_state_change

byte get_data(){
  byte temp = 0;
  if(Serial.available()) temp = Serial.read(); // get data from serial port
  return temp;
} //end get_data

byte do_cmmd(byte cmmd){
  if (check_cmmd(cmmd)){ //check if command is valid
    return 1;
  } //end if
  else return 0;
} //end do_cmmd

void clear_LCD(void){
  Serial.write(0xFE); //clear the LCD
  Serial.write(0x51);
  delay(15); //minimum delay for the LCD to finish clearing
} //end clear_LCD

byte check_cmmd(byte cmmd){
  byte i = 0;
  while(func[i]){ //while the array still can be checked
    if(cmmd == func[i]){ //if a value matches one in the array
      return 1; //return 1 if one found
    } //end if
    i++;
  }// end while
  return 0; //return 0 if none found
} //end check_cmmd

void Serialdump(void){ //This doesn't work right now
  byte temp_dump = 0;
  while(Serial.available()){
    temp_dump = Serial.read();
  } //end while
} //end Serialdump

