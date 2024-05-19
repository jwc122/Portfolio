//Include necessary libraries
#include <CapacitiveSensor.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <MPU6050.h>
#include <SPI.h>
#include <SD.h>
#include <TMRpcm.h>
#include <Servo.h>

#define NEOPIXEL_PIN 8  // Define the pin for the NeoPixel rings
#define NUMPIXELS_TOTAL 108  // Total number of NeoPixels in all rings

// Initializing libraries objects
Adafruit_NeoPixel pixels(108, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800); 
MPU6050 mpu;
Servo myServo2;
Servo myServo; 
TMRpcm audio;

const int chipSelect = 10;  // Chip select pin for SD card
const int arraySize = 62;   // Chosen array size
const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.
 
// Defining each half segment of each ring seperately, neccessary for state 1 LED code
const int NUMPIXELS_LEFT_HALF_1 = 14;
const int NUMPIXELS_RIGHT_HALF_1 = 14;
const int NUMPIXELS_LEFT_HALF_2 = 12;
const int NUMPIXELS_RIGHT_HALF_2 = 12;
const int NUMPIXELS_LEFT_HALF_3 = 10;
const int NUMPIXELS_RIGHT_HALF_3 = 10;
const int NUMPIXELS_LEFT_HALF_4 = 8;
const int NUMPIXELS_RIGHT_HALF_4 = 8;
const int NUMPIXELS_LEFT_HALF_5 = 6;
const int NUMPIXELS_RIGHT_HALF_5 = 6;
const int NUMPIXELS_LEFT_HALF_6 = 4;
const int NUMPIXELS_RIGHT_HALF_6 = 4;

// Further LED constants for later function use
const int FADE_SPEED = 10; 
const int DIM_FACTOR = 50; 
const int FADE_OUT_FACTOR = 5;
const int NUM_RINGS = 6;
const int LED_PER_RING[NUM_RINGS] = {28, 24, 20, 16, 12, 8};
uint32_t BACKGROUND_COLOR = pixels.Color(50, 50, 55);  // Solid blue background color
int audioArray[arraySize];  

// Define variables for each segment, neccessary for snowflake state code
struct Segment {
  int startPixel;
  int endPixel;
  int currentLitPixel; // New variable to track the currently lit pixel in each segment
};

// Define segments for sixths of each ring (snowflake state)
Segment segments[NUM_RINGS][6] = {
  {{0, 5}, {5, 10}, {10, 15}, {15, 20}, {20, 25}, {25, 28}},
  {{0, 4}, {4, 8}, {8, 12}, {12, 16}, {16, 20}, {20, 24}},
  {{0, 4}, {4, 8}, {8, 12}, {12, 16}, {16, 18}, {18, 20}},
  {{0, 3}, {3, 6}, {6, 9}, {9, 12}, {12, 15}, {15, 16}},
  {{0, 2}, {2, 4}, {4, 6}, {6, 8}, {8, 10}, {10, 12}},
  {{0, 2}, {2, 4}, {4, 6}, {5, 6}, {6, 7}, {7, 8}},
};

//Defining Capacitive Sensors 
CapacitiveSensor   cs_2_3 = CapacitiveSensor(2,3);        // 10M resistor between pins 4 & 2, pin 2 is sensor pin, add a wire and or foil if desired
CapacitiveSensor   cs_2_4 = CapacitiveSensor(2,4); 
CapacitiveSensor   cs_2_5 = CapacitiveSensor(2,5);        // 10M resistor between pins 4 & 6, pin 6 is sensor pin, add a wire and or foil
CapacitiveSensor   cs_2_6 = CapacitiveSensor(2,6);   

//Defining state cases
enum State {
  NO_STATE,
  STATE_1,
  STATE_2,
  STATE_3,
  STATE_4
};


//OVERALL SYSTEM CODE FUNCTIONS, VARIABLES, DEFINITIONS, ETC.
//Initial state defined
State currentState = NO_STATE;

//allowStateChange will be our boolean to control initialisation
bool allowStateChange = true;

//returns true if pin exceeds threshold value
bool isTapeTouched(int pin) {
  return pin > 1000;
}

// returns true if any capacative sensor reading exceeds threshold value
bool isAnyTapeTouched() {
  return isTapeTouched(cs_2_3.capacitiveSensor(30)) || isTapeTouched(cs_2_4.capacitiveSensor(30)) || isTapeTouched(cs_2_5.capacitiveSensor(30)) || isTapeTouched(cs_2_6.capacitiveSensor(30));
}

// returns pixels to initial state (off)
void turnOffNeoPixels() {
  for (int i = 0; i < NUMPIXELS_TOTAL; ++i) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));  // Set all pixels to off (black color)
  }
  pixels.show();  // Show the updated NeoPixel rings
  
}
// Function to read tilt value from MPU-6050
int readTiltValue() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2, true); // request 2 registers
  // Returns X tilt value
  int16_t accelValue = Wire.read() << 8 | Wire.read();
  int tiltValue = atan2(accelValue, 16384.0) * 180.0 / PI;

  return tiltValue; //X value for tilt returns 
}

// returns Y tilt value
int readTiltValueY() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3D);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2, true);

  int16_t accelValue = Wire.read() << 8 | Wire.read();
  int tiltValueY = atan2(accelValue, 16384.0) * 180.0 / PI;

  return tiltValueY;
}


//STATE 1 FUNCTIONS

//Halves of each Neopixel Ring are accessed seperated and assigned values
void lightUpHalves(int tiltValue) {
  // maps the tilt value to the total numper of pixels, constrains this to total pixel number
  int numPixelsLitLeft = constrain(map(tiltValue, 0, -60, 0, NUMPIXELS_TOTAL), 0, NUMPIXELS_TOTAL);
  int numPixelsLitRight = constrain(map(tiltValue, 0, 60, 0, NUMPIXELS_TOTAL), 0, NUMPIXELS_TOTAL);

  // manuipulating defined functions (ie.numPixelsLitLeft / 3 ) effectively speedsup or slows down movement
  //calling updateLEDs function as assigning boolean values to determine clockwise, or anticlockwise directions and half-ring behaviour.
  updateLEDs(0, NUMPIXELS_LEFT_HALF_1, numPixelsLitLeft / 3, false, false);
  updateLEDs(NUMPIXELS_LEFT_HALF_1, NUMPIXELS_RIGHT_HALF_1, numPixelsLitRight, true, false);

  updateLEDs(NUMPIXELS_LEFT_HALF_1 + NUMPIXELS_RIGHT_HALF_1, NUMPIXELS_LEFT_HALF_2, numPixelsLitLeft, false, false);
  updateLEDs(NUMPIXELS_LEFT_HALF_1 + NUMPIXELS_RIGHT_HALF_1 + NUMPIXELS_LEFT_HALF_2, NUMPIXELS_RIGHT_HALF_2, numPixelsLitRight / 3, true, false);

  updateLEDs(NUMPIXELS_LEFT_HALF_1 + NUMPIXELS_RIGHT_HALF_1 + NUMPIXELS_LEFT_HALF_2 + NUMPIXELS_RIGHT_HALF_2, NUMPIXELS_LEFT_HALF_3, numPixelsLitLeft / 3, false, false);
  updateLEDs(NUMPIXELS_LEFT_HALF_1 + NUMPIXELS_RIGHT_HALF_1 + NUMPIXELS_LEFT_HALF_2 + NUMPIXELS_RIGHT_HALF_2 + NUMPIXELS_LEFT_HALF_3, NUMPIXELS_RIGHT_HALF_3, numPixelsLitRight, true, false);

  updateLEDs(NUMPIXELS_LEFT_HALF_1 + NUMPIXELS_RIGHT_HALF_1 + NUMPIXELS_LEFT_HALF_2 + NUMPIXELS_RIGHT_HALF_2 + NUMPIXELS_LEFT_HALF_3 + NUMPIXELS_RIGHT_HALF_3, NUMPIXELS_LEFT_HALF_4, numPixelsLitLeft, false, false);
  updateLEDs(NUMPIXELS_LEFT_HALF_1 + NUMPIXELS_RIGHT_HALF_1 + NUMPIXELS_LEFT_HALF_2 + NUMPIXELS_RIGHT_HALF_2 + NUMPIXELS_LEFT_HALF_3 + NUMPIXELS_RIGHT_HALF_3 + NUMPIXELS_LEFT_HALF_4, NUMPIXELS_RIGHT_HALF_4, numPixelsLitRight / 3, true, false);

  updateLEDs(NUMPIXELS_LEFT_HALF_1 + NUMPIXELS_RIGHT_HALF_1 + NUMPIXELS_LEFT_HALF_2 + NUMPIXELS_RIGHT_HALF_2 + NUMPIXELS_LEFT_HALF_3 + NUMPIXELS_RIGHT_HALF_3 + NUMPIXELS_LEFT_HALF_4 + NUMPIXELS_RIGHT_HALF_4, NUMPIXELS_LEFT_HALF_5, numPixelsLitLeft / 3, false, false);
  updateLEDs(NUMPIXELS_LEFT_HALF_1 + NUMPIXELS_RIGHT_HALF_1 + NUMPIXELS_LEFT_HALF_2 + NUMPIXELS_RIGHT_HALF_2 + NUMPIXELS_LEFT_HALF_3 + NUMPIXELS_RIGHT_HALF_3 + NUMPIXELS_LEFT_HALF_4 + NUMPIXELS_RIGHT_HALF_4 + NUMPIXELS_LEFT_HALF_5, NUMPIXELS_RIGHT_HALF_5, numPixelsLitRight, true, false);

  updateLEDs(NUMPIXELS_LEFT_HALF_1 + NUMPIXELS_RIGHT_HALF_1 + NUMPIXELS_LEFT_HALF_2 + NUMPIXELS_RIGHT_HALF_2 + NUMPIXELS_LEFT_HALF_3 + NUMPIXELS_RIGHT_HALF_3 + NUMPIXELS_LEFT_HALF_4 + NUMPIXELS_RIGHT_HALF_4 + NUMPIXELS_LEFT_HALF_5 + NUMPIXELS_RIGHT_HALF_5, NUMPIXELS_LEFT_HALF_6, numPixelsLitLeft, false, false);
  updateLEDs(NUMPIXELS_LEFT_HALF_1 + NUMPIXELS_RIGHT_HALF_1 + NUMPIXELS_LEFT_HALF_2 + NUMPIXELS_RIGHT_HALF_2 + NUMPIXELS_LEFT_HALF_3 + NUMPIXELS_RIGHT_HALF_3 + NUMPIXELS_LEFT_HALF_4 + NUMPIXELS_RIGHT_HALF_4 + NUMPIXELS_LEFT_HALF_5 + NUMPIXELS_RIGHT_HALF_5 + NUMPIXELS_LEFT_HALF_6, NUMPIXELS_RIGHT_HALF_6, numPixelsLitRight / 3, true, false);
}

//function is called to update each hald LED ring seperately, manipulating spped and direction that pixels are lit up to give the appearance of falling snow
//Mapping tilt values allows LED 'snow' to appear as if it is falling in the direction tilted.
// flakes 'fall' at different speeds and at different times when tilted to add to sense of falling snow.
void updateLEDs(int startPixel, int numPixelsInRing, int numPixelsLit, bool clockwise, bool specialSequence) {
  for (int i = 0; i < numPixelsInRing; ++i) {
    int pixelIndex;
  
    if (specialSequence) {
      int specialIndices[] = {1, 3, 5};
      pixelIndex = clockwise ? startPixel + specialIndices[i] : startPixel + numPixelsInRing - 1 - specialIndices[i];
    } else {
      pixelIndex = clockwise ? startPixel + i : startPixel + numPixelsInRing - 1 - i;
    }

    if (pixelIndex >= NUMPIXELS_TOTAL) {
      pixelIndex -= NUMPIXELS_TOTAL;
    }

    float fadeFactor = float(i) / float(numPixelsInRing - 1);
    uint32_t currentColor = i < numPixelsLit ? pixels.Color(255, 255, random(50, 255)) : pixels.Color(0, 0, 40);
    uint32_t blendedColor = pixels.Color(
      lerpRed(currentColor, DIM_FACTOR, fadeFactor),
      lerpGreen(currentColor, DIM_FACTOR, fadeFactor),
      lerpBlue(currentColor, DIM_FACTOR, fadeFactor)
    );
    
    pixels.setPixelColor(pixelIndex, blendedColor);
  }
}


// Linear interpolation for the red component of a color
uint8_t lerpRed(uint32_t color1, uint32_t color2, float t) {
  return lerp((color1 >> 16) & 0xFF, (color2 >> 16) & 0xFF, t);
}

// Linear interpolation for the green component of a color
uint8_t lerpGreen(uint32_t color1, uint32_t color2, float t) {
  return lerp((color1 >> 8) & 0xFF, (color2 >> 8) & 0xFF, t);
}

// Linear interpolation for the blue component of a color
uint8_t lerpBlue(uint32_t color1, uint32_t color2, float t) {
  return lerp(color1 & 0xFF, color2 & 0xFF, t);
}

// Linear interpolation function
uint8_t lerp(uint8_t start, uint8_t end, float t) {
  return start + t * (end - start);
}

// STATE 2 FUNCTIONS
void readAudioFile(const char* filename, int* array, int size) {
  File file = SD.open(filename);

  if (file) {
    for (int i = 0; i < size; i++) {
      array[i] = file.read();
    }
    file.close();
    Serial.println("Audio file read successfully!");
  } else {
    Serial.println("Error opening audio file.");
  }
}

int getAmplitude() {
  // Read amplitude from the audio array
  // For simplicity, this example assumes an 8-bit audio file
  // You may need to modify this part if your audio file has a different format

  static int index = 0;
  int amplitude = audioArray[index];

  // Increment index for the next sample
  index++;

  // Reset index to the beginning if we reach the end of the array
  if (index >= arraySize) {
    index = 0;
  }

  return amplitude;
}

// STATE 3 FUNCTIONS

//state develops gradient of colours mirroring colours of the Northern Lights
// when tilted more colours emerge in direction tilted to give sense of exploring the serene landscape
void updateGentleAuroraPattern(int brightnessX, int brightnessY) {
  for (int j = 0; j < pixels.numPixels(); j++) {
    int randomBrightnessX = random(50, brightnessX + 50);
    int randomBrightnessY = random(50, brightnessY + 50);
    int gradientColor = map(j, 0, pixels.numPixels(), 0, 255);

    // Add a gradient background
    pixels.setPixelColor(j, pixels.Color(150+gradientColor, 50+2*brightnessX,150+ 2*brightnessY));

    // Add shimmering stars with a low probability
    if (random(5) < 2) {
      int shimmerColor = pixels.Color(255-randomBrightnessY,0, 0);
      pixels.setPixelColor(j, shimmerColor);
    }
  }
}

//STATE 4 FUNCTIONS

void setSolidBackgroundColor(uint32_t color) {
  for (int i = 0; i < NUMPIXELS_TOTAL; ++i) {
    pixels.setPixelColor(i, color);
  }
}


void updateSnowflakePattern(float pulse, float acceleration) {
  for (int ring = 0; ring < NUM_RINGS; ++ring) {
    int numPixelsInRing = LED_PER_RING[ring];
    int numPixelsLit = static_cast<int>(pulse * numPixelsInRing);

    for (int segment = 0; segment < 6; ++segment) {
      int startPixel = segments[ring][segment].startPixel;
      int endPixel = segments[ring][segment].endPixel;

      if (segment == 0) {
        segments[ring][segment].currentLitPixel = endPixel - numPixelsLit;
      }

      for (int i = 0; i < numPixelsLit; ++i) {
        int index = segments[ring][segment].currentLitPixel + i + ring * NUMPIXELS_TOTAL / NUM_RINGS;

        if (index >= 0 && index < NUMPIXELS_TOTAL) {
          uint8_t shimmer = random(215, 235);
          uint8_t shimmer2 = random(0, 255);
          uint32_t currentColor = (acceleration > 11.4) ? pixels.Color(155 - shimmer2, 5, 23) : pixels.Color(0, 0, 255 - shimmer);
          pixels.setPixelColor(index, currentColor);
        }
      }

      segments[ring][segment].currentLitPixel = endPixel - numPixelsLit;
    }
  }
}




void setup() {
  
  Serial.begin(9600);
  pixels.begin(); // Initialize the NeoPixel rings
  
 
  Serial.println("Setup...");
   // Initialize SD card
  // Set 1 for 2x oversampling, Set 0 for normal, Set 2 for 4x oversampling
  Wire.begin();
  mpu.initialize();
  readAudioFile("lastxmas.wav", audioArray, arraySize);  // Read audio file into array
  audio.speakerPin = 9; // Audio output pin from Arduino

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0);    // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  
}


void loop() {
  
  // Certain values need to be read continously to avoid static reading once entering the if-for loops
  //looping before beginning capactive-sensing loop ensures values are read continuously 

  int tiltX = readTiltValue(); // Read tilt value from MPU-6050 in the x direction
  int tiltY = readTiltValueY(); // Read tilt value from MPU-6050 in the y direction
  
  // Read amplitude from audio array
  int amplitude = getAmplitude(); 

  long total1 = cs_2_3.capacitiveSensor(30);
  long total2 = cs_2_4.capacitiveSensor(30);
  long total3 = cs_2_5.capacitiveSensor(30);
  long total4 = cs_2_6.capacitiveSensor(30);
  
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);
  float accelerationX = (float)ax / 16384.0 * 9.81;
  float accelerationY = (float)ay / 16384.0 * 9.81;
  float accelerationZ = (float)az / 16384.0 * 9.81;

  // Calculate total acceleration magnitude
  float acceleration = sqrt(accelerationX * accelerationX + accelerationY * accelerationY + accelerationZ * accelerationZ);

  // Set a threshold for detecting shake
  float shakeThreshold = 11.4;
  
  
   
  // Making states clear using switch(currentState)

  switch (currentState) {
    case STATE_1:
      
  // Read tilt value from MPU-6050

  // Print the x-value to the serial monitor
      Serial.print("X-value: ");
      Serial.println(tiltX);

  // Light up the left and right halves of each ring based on tilt direction
      lightUpHalves(tiltX);
     
  // Update the NeoPixel rings
      pixels.show();

      if (!audio.isPlaying()) {
    // Start playing audio file
      audio.play("lastxmas.wav");
      break;

    case STATE_2:

     int brightness = map(amplitude, 0, 255, 0, 255);

  // Update NeoPixels with a pattern based upon audio amplitutde
      for (int j = 0; j < pixels.numPixels(); j++) {
      int randomBrightness = random(0, brightness + 1);
      int randomColor = random(250, 256);

      pixels.setPixelColor(j, pixels.Color(255 - randomBrightness, 55 - randomBrightness, 55));
      }
      pixels.show();  // Update NeoPixels for all rings

      if (!audio.isPlaying()) {
    // Start playing audio file
      audio.play("jingle.wav");
     // Read tilt value from MPU-6050 in the y direction

      break;

    case STATE_3:
      // Update NeoPixels based on tilt values for each ring
      
  // Map tilt values to NeoPixel brightness
      int brightnessX = map(tiltX, -90, 90, 0, 255);
      int brightnessY = map(tiltY, -90, 90, 0, 255);

  // Update NeoPixels with a gentler aurora pattern
      updateGentleAuroraPattern(brightnessX, brightnessY);
   
      pixels.show();

      if (!audio.isPlaying()) {
    // Start playing audio file
      audio.play("walking.wav");
      }
      
      }
      break;

    case STATE_4:
      //snowflake state function called
      if (acceleration > shakeThreshold){
          BACKGROUND_COLOR = pixels.Color(70 + random(0, 5), 70 + random(0, 5) ,55 + random(0, 5));
      } else {
            BACKGROUND_COLOR = pixels.Color(150 - random(0,10), 150- random(0,10), 55);
      } 
      setSolidBackgroundColor(BACKGROUND_COLOR);

      float pulse = sin(millis() * 0.002) * 0.5 + 0.5;
      updateSnowflakePattern(pulse, acceleration);

      pixels.show();

      if (!audio.isPlaying()) {
    // Start playing audio file
      audio.play("tree.wav");
      }      
      break;
  }



  // capacative sensing loop
  // repeating calling of certain function to reduce chance of error met if just calling them in switch code
  // allowStateChange must be true for State Initialisation to be allowed
  if (isTapeTouched(total1) && allowStateChange) {
    currentState = STATE_1;

    //allowStateChange sets to false after intial touch to ensure stays in state for period of contact
    //even if first tape touched is let go of, as long as some contact remains, will remain in first initialised state 
    
    allowStateChange = false;
    
    Serial.println(tiltX);
    lightUpHalves(tiltX);
    pixels.show();
    
    Serial.println("Initialized STATE_1");

  } else if (isTapeTouched(total2) && allowStateChange) {
    currentState = STATE_2;
    allowStateChange = false;
   int brightness = map(amplitude, 0, 255, 0, 255);

  // Update NeoPixels with a pattern based upon audio amplitutde
      for (int j = 0; j < pixels.numPixels(); j++) {
      int randomBrightness = random(0, brightness + 1);
      int randomColor = random(250, 256);

      pixels.setPixelColor(j, pixels.Color(255 - randomBrightness, 55 - randomBrightness, 55));
      }
      pixels.show();  // Update NeoPixels for all rings

      if (!audio.isPlaying()) {
    // Start playing audio file
      audio.play("jingle.wav");
     // Read tilt value from MPU-6050 in the y direction

      break;

    

    allowStateChange = false;
  } else if (isTapeTouched(total3) && allowStateChange) {
    currentState = STATE_3;
    allowStateChange = false;
   
    
  // Map tilt values to NeoPixel brightness
      int brightnessX = map(tiltX, -90, 90, 0, 255);
      int brightnessY = map(tiltY, -90, 90, 0, 255);

  // Update NeoPixels with a gentler aurora pattern
      updateGentleAuroraPattern(brightnessX, brightnessY);
   
      pixels.show();

      if (!audio.isPlaying()) {
    // Start playing audio file
      audio.play("walking.wav");
      }
      
      } 

    Serial.println("Initialized STATE_3");

  } else if (isTapeTouched(total4) && allowStateChange) {
    currentState = STATE_4;
    Serial.println("Initialized STATE_4");
    allowStateChange = false;
    // Check if audio is playing
    
    if (acceleration > shakeThreshold){
          BACKGROUND_COLOR = pixels.Color(70 + random(0, 5), 70 + random(0, 5) ,55 + random(0, 5));
      } else {
            BACKGROUND_COLOR = pixels.Color(150 - random(0,10), 150- random(0,10), 55);
      } 
      setSolidBackgroundColor(BACKGROUND_COLOR);

      float pulse = sin(millis() * 0.002) * 0.5 + 0.5;
      updateSnowflakePattern(pulse, acceleration);

      pixels.show();

      if (!audio.isPlaying()) {
    // Start playing audio file
      audio.play("tree.wav");
      }      
      break;
  }
   

  // Show the updated NeoPixel rings
  pixels.show();
  // if no tape is being touched , there is no contact, and NeoPixels turn off
  //allowStateChange set to true to allow for reinitialisation
  if (!isAnyTapeTouched()) {
    currentState = NO_STATE;
    turnOffNeoPixels();
    allowStateChange = true;  // Reset the flag when all pins are not touched
  }

  Serial.print(total1);
  Serial.print("\t");
  Serial.print(total2);
  Serial.print("\t");
  Serial.print(total3);
  Serial.println(total4);

  // Show the updated NeoPixel rings
  
  }}
