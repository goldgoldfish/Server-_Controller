/*
 * Server_Controller
 * Ben Wedemire
 * 
 * Description: Not yet!
 */

//declaring pin variables
const int power_on_pin = 2; //Connects to the power on LED of the server
const int power_state_pin = 4; //LED displaying the server's power statues
const int power_button_pin = 10; //the server's power button

//declaring global variables
volatile boolean power_state = 0;
const byte func[3] = {89, 90, 91};

//declaring void functions
void power_state_change(void);
void clear_LCD(void);
void Serialdump(void);
void display_power_state(void);

//declaring returning functions
byte get_data(void);
byte do_cmmd(byte cmmd);
boolean start_server(void);
boolean shutdown_server(void);
boolean restart_server(void);

//byte check_cmmd(byte cmmd); Don't need this at the moment

void setup() {
  pinMode(power_on_pin, INPUT);
  pinMode(power_state_pin, OUTPUT);
  pinMode(power_button_pin, OUTPUT);

  Serial.begin(9600);

  attachInterrupt(digitalPinToInterrupt(power_on_pin), power_state_change, CHANGE);
} //end setup

void loop() {
  int power_LED_statues = 0; //var for the current statues of the power led
  byte cmmd = 0; //stores the value of any requested command
  if (digitalRead(power_on_pin)) {
    power_state = 1; //if the server power is on power state is true
  } //end if
  else {
    power_state = 0; //if the server power is off power state is false
    Serial.println("Server is Down");
  } //end else
  while (1){
    if (power_LED_statues != power_state){
      digitalWrite(power_state_pin, power_state); //upadate the power statues led
      power_LED_statues = power_state; //update power led statues
      display_power_state(); //Update the LCD with the server power statues
    } //end if
    cmmd = get_data(); //gets the most recent cmmd
    if (cmmd){
      clear_LCD();
      if (do_cmmd(cmmd)){ //check and do the cmmd
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
  if (digitalRead(power_on_pin)) power_state = 1; //if the server power is on power state is true
  else power_state = 0; //if the server power is off power state is false
} //end power_state_change

byte get_data(){
  byte temp = 0;
  if (Serial.available()) temp = Serial.read(); // get data from serial port
  return temp;
} //end get_data

byte do_cmmd(byte cmmd){
  if (cmmd == '1'){ //display the power state of the server
    display_power_state();
    return 1;
  } //end if
  else if (cmmd == '2'){ //turn on the server
    if (!power_state){ //check power statues of server
      if (start_server()){//attempt to start server
        clear_LCD();
        Serial.println("Starting Server");
      } //end if
      else {
        clear_LCD();
        Serial.println("Server Not Started");
      } //end else
    } //end if
    else{
      clear_LCD();
      Serial.println("Server Already Started");
    } //end else
    return 1;
  } //end else if 
  else if (cmmd == '3'){ //soft power off the server
    if (power_state){ //check power statues of server
      if (shutdown_server()){//attempt to shutdown server
      } //end if
      else { //could not turn off server
        clear_LCD();
        Serial.println("Server Not Off");
      } //end else
    } //end if
    else{
      clear_LCD();
      Serial.println("Server Already Off");
    } //end else
    return 1;
  } //end else if 
  else if (cmmd == '4'){ //restart the server
    restart_server();
    return 1;
  } //end else if
  else if (cmmd == '9'){ //hard power off the server
    
  } //end else if
  else return 0;
} //end do_cmmd

void clear_LCD(void){
  Serial.write(0xFE); //clear the LCD
  Serial.write(0x51);
  delay(25); //minimum delay for the LCD to consistantly finish clearing
} //end clear_LCD

//byte check_cmmd(byte cmmd){
//  byte i = 0;
//  while(func[i]){ //while the array still can be checked
//    if(cmmd == func[i]){ //if a value matches one in the array
//      return 1; //return 1 if one found
//    } //end if
//    i++;
//  }// end while
//  return 0; //return 0 if none found
//} //end check_cmmd


void Serialdump(void){ //This doesn't work right now
  byte temp_dump = 0;
  while(Serial.available()){
    temp_dump = Serial.read();
  } //end while
} //end Serialdump

void display_power_state(void){
  if (power_state){
    clear_LCD();
    Serial.println("Server is Up");
  } //end if
  else if (!power_state){
    clear_LCD();
    Serial.println("Server is Down");
  } //end else if
  else {
    clear_LCD();
    Serial.println("Error");
  } //end else
} //end display_power_state

boolean start_server(void){
  digitalWrite(power_button_pin, HIGH); //attempt to start the server
  delay(200); 
  digitalWrite(power_button_pin, LOW); //release button
  int iter = 0;
  while (!power_state){ //wait for server to come up
    if (iter == 500) return 0;
    delay(10);
    iter++;
  } //end while
  return 1;
} //end start_server

boolean shutdown_server(void){
  digitalWrite(power_button_pin, HIGH); //attempt to shutdown the server
  delay(200); 
  digitalWrite(power_button_pin, LOW); //release button
  int iter = 0;
  clear_LCD();
  Serial.println("Please Wait");
  while (power_state){ //wait for server to shutdown
    if (iter == 5000) return 0;
    delay(10);
    iter++;
  } //end while
  return 1;
} //end shutdown_server

boolean restart_server(void){
  if (!power_state){
    clear_LCD();
    Serial.println("Server is Already Off");
  } //end if
  else{
    if (shutdown_server()){
      clear_LCD();
      Serial.println("Server is Down");
      if (start_server()){
        clear_LCD();
        Serial.println("Restart Complete");
        Serial.println("Server is Up");
      } //end if
      else{
        clear_LCD();
        Serial.println("Error in Starting Server");
      } //end else
    } //end if
    else{
      clear_LCD();
      Serial.println("Error in Server Shutdown");
    } //end else
  } //end else
} //end restart_server
