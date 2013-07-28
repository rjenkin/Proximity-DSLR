/****************************************************************************************
 * File:          proximitydslr.ino
 * Version:       1.1
 * Description:   Photograph a user when they're within range of a camera
 * Author:        Ryan Jenkin
 * Date started:  2013-07-10
 ***************************************************************************************/


// Pins ---------------------------------------------------------------------------------
const unsigned int pinPing   = 7;
const unsigned int pinCamera = 9;
const unsigned int pinLED1   = 2;
const unsigned int pinLED2   = 3;
const unsigned int pinLED3   = 4;

//const unsigned int pinLatch = 8;
//const unsigned int pinData  = 11;
//const unsigned int pinClock = 12;

// Zones --------------------------------------------------------------------------------
// Note: Zones must match or overlap. Gaps will cause infinite loop.
const unsigned int zones[4][2] = {
	{ // Too close
		0   + 0,
		100 + 5
	},
	{ // Take photo
		100 - 5,
		200 + 5,
	},
	{ // Something detected, but don't take photo
		200 - 5,
		313 + 0,
	},
	{ // Out of range
		313,
		999
	}
};


// Variables ----------------------------------------------------------------------------
unsigned int zone        =    0; // Default to out of range zone.
long distance            =  400; // Initial distance set to out of range zone
unsigned int loopDelay   = 1000; // ms between loop
unsigned int cameraReady =    0; // A photo can only be taken when this is 0


/**
 * Setup()
 */
void setup() {

	Serial.begin(9600);

	pinMode(pinCamera, OUTPUT);

	// LEDs
	pinMode(pinLED1, OUTPUT);
	pinMode(pinLED2, OUTPUT);
	pinMode(pinLED3, OUTPUT);

}


/**
 * Main loop
 */
void loop() {

	// Get distance from Ping)))
	getDistance();

	// Set the zone variable
	setZone();

	// Output debug data...
	Serial.print("Distance: ");
	Serial.print(distance);
	Serial.print(", Zone: ");
	Serial.println(zone);

	switch (zone) {
		case 0:
			// Too close
			digitalWrite(pinLED1, LOW);
			digitalWrite(pinLED2, LOW);
			digitalWrite(pinLED3, HIGH);
			break;

		case 1:
			// Shoot zone
			digitalWrite(pinLED1, LOW);
			digitalWrite(pinLED2, HIGH);
			digitalWrite(pinLED3, LOW);
			activateCamera();
			break;

		case 2:
			// Detected but out of range
			digitalWrite(pinLED1, HIGH);
			digitalWrite(pinLED2, LOW);
			digitalWrite(pinLED3, LOW);
			break;

		case 3:
			// Out of range
			digitalWrite(pinLED1, LOW);
			digitalWrite(pinLED2, LOW);
			digitalWrite(pinLED3, LOW);
			break;
	}

	// Delay between photos
	if (cameraReady > 0) {
		cameraReady--;
	}

	delay(loopDelay);
}


/**
 * Use distance to determine zone.
 *
 * Zones are implemented using dynamic threshold to prevent
 * the zone from change change and forth when the object
 * is right on the edge of two zones
 */
void setZone() {

	if (distance < 0.0) {
		// Prevent negative distance
		distance = 0.0;
	}

	boolean zone_found = false;

	// Search for the correct zone
	while (false == zone_found) {

		if (distance < zones[zone][0]) {
			zone--;
		}
		else if (distance > zones[zone][1]) {
			zone++;
		}
		else {
			zone_found = true;
		}

	}

}


/**
 * Return distance to target in centimeters
 */
long getDistance() {

	long duration;

	// The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
	// Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
	pinMode(pinPing, OUTPUT);

	// Quiet
	digitalWrite(pinPing, LOW);
	delayMicroseconds(2);

	// Send ping
	digitalWrite(pinPing, HIGH);
	delayMicroseconds(5);

	// Quiet
	digitalWrite(pinPing, LOW);

	// The same pin is used to read the signal from the PING))): a HIGH
	// pulse whose duration is the time (in microseconds) from the sending
	// of the ping to the reception of its echo off of an object.
	pinMode(pinPing, INPUT);
	duration = pulseIn(pinPing, HIGH);

	// Convert ping time to distance
	distance = microsecondsToCentimeters(duration);
}


/**
 * Convert the time to distance (metric)
 *
 * @param long microseconds
 * @return long
 */
long microsecondsToCentimeters( long microseconds ) {

	// The speed of sound is 340 m/s or 29 microseconds per centimeter.
	// The ping travels out and back, so to find the distance of the
	// object we take half of the distance travelled.
	return microseconds / 29 / 2;

}


/**
 * Activate the camera
 */
void activateCamera() {

	// This prevents the camera taking too many shots.
	if (cameraReady > 0) {
		return;
	}

	// Trigger shutter..
	digitalWrite(pinCamera, HIGH);
	delay(250);
	digitalWrite(pinCamera, LOW);

	cameraReady = 2000 / loopDelay; // First number is ms to delay
}

