const int FSR_PIN = A0;
const int FSR_CLICK_THRESHOLD = 300;
const int FSR_COUNT_THRESHOLD = 1000;
int fsr_counter = 0;
int fsr_val = 0;

/* 
 * 0 = waiting to connect to node
 * 1 = waiting for drink order 1
 * 2 = waiting for drink order 2
 * 3 = sending drink order
 * 4 = waiting for drink started
 * 5 = waiting for drink finished
 * 6 = sending drink delivered
 * 7 = waiting to restart
 */
const int WAIT_CONNECT = 0;
const int WAIT_DRINK_1 = 1;
const int WAIT_DRINK_2 = 2;
const int SEND_DRINK_O = 3;
const int WAIT_DRINK_S = 4;
const int WAIT_DRINK_F = 5;
const int SEND_DRINK_D = 6;
const int WAIT_RESTART = 7;
int state = 0;

// 96 is '^', chosen because beginning of line in regex
const char OPEN  = 96;
// 36 is '$', chosen because end of line in regex
const char CLOSE = 36;

/* 
 * values 0-2 represent different drinks on the chart
 * -1 means not selected yet
 * Spirit
 *   0 => Vodka => Green
 *   1 => Gin   => Blue
 *   2 => Rum   => Red
 *
 * Mixer: 
 *   0 => Lemon Lime Soda => Green
 *   1 => Orange          => Blue
 *   2 => Cranberry       => Red
 */
const int RED   = 0;
const int BLUE  = 1;
const int GREEN = 2;
//int spirit = -1;
//int mixer  = -1;
// Store as an array, spirit in 0, mixer in 1
int spirit_mixer[] = {-1, -1};
// Keep track of the current drink waiting to be selected
int current_drink_slot = 0;

boolean sending = false;

// For how many loops has the pressure sensor been reading above a certain threshold?
int pressed_counter = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  switch (state) {
    case WAIT_CONNECT:
      connect_to_server();
      break;
    case WAIT_DRINK_1:
    case WAIT_DRINK_2:
//    Serial.println("drink");
//    Serial.println(fsr_counter);
      process_drink_selection();
      break;
    case SEND_DRINK_O:
      // state++;
      break;
    case WAIT_DRINK_S:
      // state++;
      break;
    case WAIT_DRINK_F:
      // state++;
      break;
    case SEND_DRINK_D:
      // state++;
      break;
    case WAIT_RESTART:
      // state = WAIT_DRINK_1;
      break;
  }
}

// Handles state WAIT_CONNECT
// Send a connect message until the server responds
void connect_to_server() {
  if (message_was_received()) {
    state = WAIT_DRINK_1;
  }
  else {
    // Throw out a request to connect until the server responds with a '$'
    Serial.println("connect"); 
  }
}

// Handles state WAIT_DRINK_1 & WAIT_DRINK_2
void process_drink_selection() {
   if (was_clicked()) {
     // TODO: hardcoded RGB result for POT testing
     spirit_mixer[current_drink_slot] = RED;
     Serial.println(spirit_mixer[current_drink_slot]);
     current_drink_slot++;
     
     if (current_drink_slot > 1) {
       state = SEND_DRINK_O; 
     }
   } 
}

boolean was_clicked() {
  fsr_val = analogRead(FSR_PIN);
//  Serial.println(fsr_val);

  // Light up this many leds
  // const int LOOPS_PER_LED = FSR_CLICK_THRESHOLD / NEOPIXEL_COUNT;
  // int pins_to_light = (int)(fsr_counter / LOOPS_PER_LED)
  if (fsr_val > FSR_CLICK_THRESHOLD) {
    fsr_counter++;
  }
  else {
    fsr_counter = 0;
  }
  
  if (fsr_counter > FSR_COUNT_THRESHOLD) {
    fsr_counter = 0;
    return true;
  }
  else {
    return false;
  }
}

// Returns true if we get the close char ("$"), false otherwise
boolean message_was_received() {
  char in_char = Serial.read();
  if (in_char == CLOSE) {
    return true;
  }
  else {
    return false;
  }
}
    
// read for incoming messages. c = send, x = don't send:
//  char inChar = Serial.read();
//  switch (inChar) {
//  case 'c':    // connection open
//    sending = true;
//    break;
//  case 'x':    // connection closed
//    sending = false;
//    break;
//  }
//
//  if (sending) {
//    // read sensors:
//    int x = random();
//    delay(1);
//    int y = random();
//    delay(1);
//    int z = random();
//
//    // form a JSON-formatted string:
//    String jsonString = "{\"x\":\"";
//    jsonString += x;
//    jsonString +="\",\"y\":\"";
//    jsonString += y;
//    jsonString +="\",\"z\":\"";
//    jsonString += z;
//    jsonString +="\"}";
//
//    // print it:
//    Serial.println(jsonString);
//  }

