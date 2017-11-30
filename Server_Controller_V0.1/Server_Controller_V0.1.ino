/*
 * Server_Controller
 * Ben Wedemire
 * 
 * Description: Not yet!
 */

#include <SPI.h>
#include <Ethernet.h>

//declaring pin variables
const int power_on_pin = 2; //Connects to the power on LED of the server
const int power_state_pin = 5; //LED displaying the server's power statues
const int power_button_pin = 10; //the server's power button

//declaring global variables
volatile boolean power_state = 0;
byte force_power_off = 0; //var for the controller to not try to turn on the server

//declaring void functions
void power_state_change(void);
void clear_LCD(void);
void Serialdump(void);
void display_power_state(void);
void disable_controller(void);
void enable_controller(void);

//declaring returning functions
byte get_data(void);
byte do_cmmd(byte cmmd);
boolean start_server(void);
boolean shutdown_server(void);
boolean restart_server(void);

//setup ethernet properties
byte mac[] = { 0x90, 0xA2, 0xDA, 0x11, 0x1F, 0x1A };
IPAddress ip(192, 168, 0, 50);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

EthernetServer server(80);

void setup() {
  pinMode(power_on_pin, INPUT);
  pinMode(power_state_pin, OUTPUT);
  pinMode(power_button_pin, OUTPUT);

  Serial.begin(9600);
  /*
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Serial connection open");
  */
  
  attachInterrupt(digitalPinToInterrupt(power_on_pin), power_state_change, CHANGE);

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip, gateway, subnet);
  Serial.println("Ethernet Started");

  server.begin();
  Serial.println("Server Started");
 
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
} //end setup

void loop() { 
  byte timeout = 0; //var for if the max startup attempts have been reached
  byte power_LED_statues = 0; //var for the current statues of the power led
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
      if(power_LED_statues) timeout = 0; //reset timeout flag
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
    if (!power_LED_statues && !force_power_off && !timeout && !power_state){ //power on server if needed
      byte startup_attempts = 0; //number of attempts to start the server
      clear_LCD();
      Serial.println("Attempting Startup");
      attempt_startup_again:
      ether();
      if (start_server());
      else{
        startup_attempts++;
        if(startup_attempts >= 5) {
          clear_LCD();
          Serial.println("Too Many Attempts");
          timeout = 1;
        }
        else goto attempt_startup_again;
      } //end else
    } //end if
    ether();
  } //end while
} //end loop

void power_state_change(void){
  if (digitalRead(power_on_pin)) {
    power_state = 1; //if the server power is on power state is true
    force_power_off = 0; //controller back to normal state
    //Serial.println("Force off: 0");
  } //end if
  else {
    power_state = 0; //if the server power is off power state is false
  } //end else
} //end power_state_change

byte get_data(){
  byte temp = 0;
  if (Serial.available()) temp = Serial.read(); // get data from serial port
  return temp;
} //end get_data

byte do_cmmd(byte cmmd){
  if (cmmd == '1'){ //display the power state of the server
    display_power_state();
    goto good_cmmd;
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
    goto good_cmmd;
  } //end else if 
  else if (cmmd == '3'){ //soft power off the server
    force_power_off = 1; //tell controller to not turn on server
    //Serial.println("Force off: 1");
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
    goto good_cmmd;
  } //end else if 
  else if (cmmd == '4'){ //restart the server
    restart_server();
    goto good_cmmd;
  } //end else if
  else if (cmmd == '7'){ //disable controller
    disable_controller();
    goto good_cmmd;
  } //end else if
  else if (cmmd == '8'){ //enable controller
    enable_controller();
    goto good_cmmd;
  } //end else if
  else if (cmmd == '9'){ //hard power off the server
    force_power_off = 1; //tell controller to not turn on the server
    //Serial.println("Force off: 1");
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
    goto good_cmmd;
  } //end else if
  else {
    Serialdump(); //dump the rest of the serial buffer
    return 0;
  } //end else
  good_cmmd:
  Serialdump(); //dump the rest of the serial buffer
  return 1;
} //end do_cmmd

void clear_LCD(void){
  Serial.write(0xFE); //clear the LCD
  Serial.write(0x51);
  delay(25); //minimum delay for the LCD to consistantly finish clearing
} //end clear_LCD

void display_power_state(void){
  if (power_state){ //check if server is on
    clear_LCD();
    Serial.println("Server is Up");
  } //end if
  else if (!power_state){ //check if server is off
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
  unsigned long shutdown_time_start = millis(); //get the time that shutdown starts
  unsigned long shutdown_time_curr = shutdown_time_start;
  const unsigned long time_delay_soft = 60000; //the amount of time that shutdown will be attempted (ms)
  while (power_state && shutdown_time_curr < (shutdown_time_start + time_delay_soft)){ //wait for server to shutdown
    shutdown_time_curr = millis();
  } //end while
  if (!power_state) return 1;
  else return 0;
} //end shutdown_server

boolean restart_server(void){
  if (!power_state){
    clear_LCD();
    Serial.println("Server Already Off");
  } //end if
  else{
    if (shutdown_server()){ //check if shutdown was successful
      clear_LCD();
      Serial.println("Server is Down");
      Serial.println("Restarting");
      if (start_server()){ //check if startup was successful
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

void disable_controller(){ //disables all commands and functions execpt enable
  clear_LCD();
  Serial.println("Controller Disabled");
  while(get_data() != '8'); //waits for enable command
  clear_LCD();
  Serial.println("Server Enabled");
} //end disable controller

void enable_controller(){ //this is command is really just a place holder
  clear_LCD();
  Serial.println("Controller Already Enabled");
} //end enable controller

boolean force_shutdown(void){
  digitalWrite(power_button_pin, HIGH); //simulate button push
  unsigned long shutdown_time_start = millis(); //get the time that shutdown starts
  unsigned long shutdown_time_curr = shutdown_time_start;
  const unsigned long time_delay_hard = 60000; //the amount of time that shutdown will be attempted (ms)
  if (shutdown_time_start >= (4294907295)){
    while (power_state && (shutdown_time_curr + 60001) < (shutdown_time_start + (time_delay_hard*2+1))) shutdown_time_curr = millis(); //wait for server shutdown
  } //end if
  else {
    while (power_state && shutdown_time_curr < (shutdown_time_start + time_delay_hard)) shutdown_time_curr = millis(); //wait for server shutdown
  } //end else
  digitalWrite(power_button_pin, LOW); //unpress simulated button
} //end force_shutdown

void Serialdump(void){
  byte temp_dump = 0;
  while(Serial.available()){
    temp_dump = Serial.read();
  } //end while
} //end Serialdump

void ether(void){
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          //client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.print("The server statues is ");
          if(power_state == 1) client.print("ON");
          else if(power_state == 0) client.print("OFF");
          else client.print("ERROR");
          client.println("<br />");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}
