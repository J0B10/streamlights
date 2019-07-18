#include <Adafruit_NeoPixel.h>

#define RGB_DATA_PIN 2          //Digital Pin connected to the led stripes data line
#define BUTTON_PIN 12           //Digital Pin connected to a button for control of the circuit

#define PIXEL_COUNT 25          //Number of rgb leds connected to the board

#define TICK_RATE 20            //milliseconds between each loop (determines accuracy for button press detection)
#define CLICK_MAX_TIME 500      //How long a click of the button should take at max (in milliseconds)


//Param 1 = number of pixels in strip,  neopixel stick has 8
//Param 2 = pin number (most are valid)
//Param 3 = pixel type flags, add together as needed:
//  NEO_RGB     Pixels are wired for RGB bitstream
//  NEO_GRB     Pixels are wired for GRB bitstream
//  NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//  NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)
Adafruit_NeoPixel rgb = Adafruit_NeoPixel(PIXEL_COUNT, RGB_DATA_PIN, NEO_GRB + NEO_KHZ800);

//Arrays that store the color values of all leds
int rgb_red[PIXEL_COUNT];
int rgb_green[PIXEL_COUNT];
int rgb_blue[PIXEL_COUNT];

float brightness = 1.0;         //Brightness of the stripe (0.1 to 16)
int color = 0;                  //color of the leds. Aviable: 0=black, 1=red, 2=orange, 3=yellow, 4=green, 5=cyan, 6=light_blue, 7=dark_blue, 8=purple, 9=pink, 10=white

bool isButtonEnabled = true;

unsigned int pressedTime = 0;   //How long the button has been pressed now (in milliseconds)
unsigned int releasedTime = 0;  //How long the button hasn't been pressed now (in milliseconds, doesn't increase over 10 seconds to prevent overflow)
int recent = 0;                 //How often the button has been pressed in the last second (required for double-tap functionality)

String serialInputString = "";

//Run on startup
//  Sets up all pin modes
//  sets all leds to black
//  Inits neopixel library
//  Open serial port
void setup() {
  pinMode(BUTTON_PIN, INPUT);
  for(int i = 0; i < PIXEL_COUNT; i++) {
    rgb_red[i] = 0;
    rgb_green[i] = 0;
    rgb_blue[i] = 0;
  }
  rgb.begin();
  rgb.show();
  Serial.begin(9600);
}

// Run in a loop at a fixed $TICK_RATE
void loop() {
  serialInput();
  if (isButtonEnabled) handleButtonPress();
  updateColors();
  delay(TICK_RATE);
}

//Detects press on the control button and calls according functions
void handleButtonPress() {
  if (digitalRead(BUTTON_PIN)) {
      //button is pressed
      pressedTime += TICK_RATE;
      releasedTime = 0;
      if (pressedTime >= CLICK_MAX_TIME) {
        //button is held down 
        controlBrightness();
      }
  } else {
    //button isn't pressed
    if (releasedTime < 10000) {
      //only increase releasedTime till 10s (more is not required)
      releasedTime += TICK_RATE;
    }
    if (pressedTime < CLICK_MAX_TIME && pressedTime > 0) {  
      //button was just released
      recent++;
    } else if (releasedTime >= CLICK_MAX_TIME && pressedTime == 0) {
      //button hasn't been pressed in last second
      if (recent == 1) {
        //single tap
        changeColor();
      } else if (recent == 2) {
        //double tap
        switchSpecialEffects();
      } else {
        //more than double tap
        while(--recent > 0) {
          changeColor();
        }
      }
      recent = 0;
    }
    pressedTime = 0;
  }
  
}

//TODO Handle instructions from the serial connection with the pc
void serialInput() {
  while (Serial && Serial.available() > 0) {
    char next = (char) Serial.read();
    if (next == '\n') {
      bool success = false;
      if (serialInputString[0] == '#') {
        int commandEnd = serialInputString.indexOf(':');
        if (commandEnd != -1) {
          String command = serialInputString.substring(1, commandEnd);
          command.toUpperCase();
          String data = serialInputString.substring(commandEnd + 1, serialInputString.length());
          int splitIndex = -1;
          int args_length = 0;
          do {
            args_length++;
            splitIndex = data.indexOf(',', splitIndex + 1);
          } while(splitIndex != -1);
          String args[args_length];
          for(int i = 0; i < args_length; i++) {
            splitIndex = data.indexOf(',');
            args[i] = data.substring(0, splitIndex);
            args[i].trim();
            if (splitIndex + 1 != data.length()) data = data.substring(splitIndex + 1);
            else data = "";
          }
          if (command.equals("PIXEL")) {
              success = (args_length >= 4 && PIXEL(args));
          } else if (command.equals("STRIP")) {
              success = (args_length >= 3 && STRIP(args));
          } else if (command.equals("BRIGH")) {
              success = (args_length >= 1 && BRIGH(args));
          } else if (command.equals("CTRL")) {
              success = (args_length >= 1 && CTRL(args));
          }
        }
      }
      if (!success) {
        Serial.print("ERROR Invalid statement: \"");
        Serial.print(serialInputString);
        Serial.println("\"");
      }
      serialInputString = "";
    } else {
      serialInputString += next;
    }
  }
}

//Changes the color (cycles through the following presets: 0=black, 1=red, 2=orange, 3=yellow, 4=green, 5=cyan, 6=light_blue, 7=dark_blue, 8=purple, 9=pink)
void changeColor() {
  if (++color > 10) color = 0;
  int r = 0;
  int g = 0;
  int b = 0;
  switch (color) {
    case 1:
      r = 255;
      g = 0;
      b = 0;
      break;
    case 2:
      r = 255;
      g = 127;
      b = 0;
      break;
    case 3:
      r = 255;
      g = 255;
      b = 0;
      break;
    case 4:
      r = 0;
      g = 255;
      b = 0;
      break;
    case 5:
      r = 0;
      g = 255;
      b = 190;
      break;
    case 6:
      r = 0;
      g = 200;
      b = 255;
      break;
    case 7:
      r = 0;
      g = 0;
      b = 255;
      break;
    case 8:
      r = 150;
      g = 0;
      b = 255;
      break;
    case 9:
      r = 255;
      g = 0;
      b = 255;
      break;
    case 10:
      r = 255;
      g = 255;
      b = 255;
      break;
  }
  for(int i = 0; i < PIXEL_COUNT; i++) {
    rgb_red[i] = r;
    rgb_green[i] = g;
    rgb_blue[i] = b;
  }
  
}

//Cycles through different brightness levels
void controlBrightness() {
  unsigned int INTERVALL = 3000 / 16;
  if ((pressedTime - CLICK_MAX_TIME) % INTERVALL < TICK_RATE) {
    unsigned int i = (pressedTime - CLICK_MAX_TIME) / INTERVALL + 1;
    if (i % 32 < 16) brightness = i % 32;
    else brightness = 32 - (i % 32);
  }
}

//TODO Let's you apply a special effect (flashing, rainbow, pulsating ...)
void switchSpecialEffects() {

}

//Updates the led stripes 
void updateColors() {
  if (brightness < 0) brightness = 0.2;
  for(int i = 0; i < PIXEL_COUNT; i++) {
    int r = (int) ((rgb_red[i] * brightness) / 16);
    int g = (int) ((rgb_green[i] * brightness) / 16);
    int b = (int) ((rgb_blue[i] * brightness) / 16);
    rgb.setPixelColor(i, rgb.Color(r, g, b));
  }
  rgb.show();
}

//Changes the color of a pixel
//arg 0:      the index of the pixel on the stripe (int)
//arg 1:      the red value of the pixels color (int)
//arg 2:      the green value of the pixels color (int)
//arg 3:      the blue value of the pixels color (int)
//return:     if the command was succesfull
bool PIXEL(String args[]) {
  int index = args[0].toInt();
  int red = args[1].toInt();
  int green = args[2].toInt();
  int blue = args[3].toInt();
  if ((index < 0 || index > PIXEL_COUNT)
    || (red < 0 || red > 255)
    || (green < 0 || green > 255)
    || (blue < 0 || green > 255)) return false;
  rgb_red[index] = red;
  rgb_green[index] = green;
  rgb_blue[index] = blue;
  Serial.println("Pixel color was set succesfully");
  return true;
}

//Changes the color of the entire stripe
//arg 0:        the red value of the new color (int)
//arg 1:        the green value of the new color (int)
//arg 2:        the blue value of the new color (int)
//return:       if the command was succesfull
bool STRIP(String args[]) {
  int red = args[0].toInt();
  int green = args[1].toInt();
  int blue = args[2].toInt();
  if ((red < 0 || red > 255)
    || (green < 0 || green > 255)
    || (blue < 0 || green > 255)) return false;
  for(int i = 0; i < PIXEL_COUNT; i++) {
    rgb_red[i] = red;
    rgb_green[i] = green;
    rgb_blue[i] = blue;
  }
  Serial.println("Stripe color was set succesfully");
  return true;
}

//Changes the brightness of the entire stripe
//arg 0:        the new brightness (float)
//return:       if the command was succesfull
bool BRIGH(String args[]) {
  float value = args[0].toFloat();
  if (value < 0.2 || value > 16) return false;
  brightness = value;
  Serial.println("Brightness was set succesfully");
  return true;
}

//Sets if the user may control the stripe with the button
//arg 0:        1 if control is allowed, 0 if not
//return:       if the command was succesfull
bool CTRL(String args[]) {
  int allowed = args[0].toInt();
  if (allowed < 0 || allowed > 1) return false;
  isButtonEnabled = (allowed == 1);
  Serial.println("Control mode was changed succesfully");
  return true;
}