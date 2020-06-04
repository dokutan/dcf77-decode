/*
 * This code can be used to connect a DCF77 receiver module over serial to a PC.
 * 
 * Connect the data output of the receiver to data_pin (2).
 * Depending on the module you might need to connect an enable pin on the module to ground.
 * 
 * The following lines are sent over serial:
 * 1 → received bit 1
 * 0 → received bit 0
 * empty line → missing pulse to mark a new minute
 * 
 * © 2020 dokutan, Licensed under the GNU GPL v3 or later
 */

// pin connected to the data output of the receiver module
int data_pin = 2;

void setup() {
  pinMode(data_pin, INPUT_PULLUP);
  Serial.begin(9600);
}

// data pin state
int state_old = digitalRead(data_pin);
int state_new = digitalRead(data_pin);

// counter for time since last pulse
// used to detect missing pulse at the beginning of a minute
int count = 0;

void loop() {

  state_old = state_new;
  state_new = digitalRead(data_pin);

  // rising edge
  if( state_old == 0 && state_new == 1 ){

    // check and reset counter → beginning of new minute?
    if( count > 100 ){
      Serial.print("\n");
    }
    count = 0;
    
    // wait 150ms → read → 0 or 1
    delay(150);
    Serial.print( digitalRead(data_pin) );
    Serial.print("\n");
    
  }

  // wait 10ms → increment counter
  delay(10);
  count++;
  
}
