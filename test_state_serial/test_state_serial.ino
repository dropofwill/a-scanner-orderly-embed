
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

// '^' chosen because beginning of line in regex
const char OPEN  = 96;
// '$' chosen because end of line in regex
const char CLOSE = 36;

/* 
 * values 0-2 represent different drinks on the chart
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
int spirit = -1;
int mixer  = -1;

boolean sending = false;

// For how many loops has the pressure sensor been reading above a certain threshold?
int pressed_counter = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  switch (state) {
    case WAIT_CONNECT:
      if (message_was_received()) {
        state++;
      }
      else {
        // Throw out a request to connect until the server responds with a '$'
        Serial.println("connect"); 
      }
      break;
    case WAIT_DRINK_1:
    case WAIT_DRINK_2:
      Serial.println("drink");
      // state++;
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

