// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Christmas Tree LEDs
// Alex Suter (asuter@gmail.com)
// April 14, 2024
//
// Run a randomized Christmas tree pattern with pulsing lights. Every ten minutes switch to another
// random pattern, then return to the default Christmas tree. If a button press is detected, switch
// to the next pattern. Uses the FastLED library to control a WS2812 LED strip arranged in the shape
// of a Christmas tree starting at the bottom left with row one, right, left, row two, three, four,
// five, and six in that order.
//
//                              +
//                             /_\               Row six
//                            /___\              Row five
//                Left Side  /_____\ Right Side  Row four
//                          /_______\            Row three
//                         /_________\           Row two
//                        /___________\          Row one
//
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include <FastLED.h>
#include <set.h>

#define LED_PIN 6
#define NUM_LEDS 190
#define NUM_TWINKLE_LIGHTS 80
#define NOT_SET_TWINKLE_LIGHT -1
#define SCALE_MAX 200
#define SCALE_MIN 0

#define END_OUTER_LOOP 121

CRGB leds[NUM_LEDS];
int twinkleLights[NUM_TWINKLE_LIGHTS];
int twinkleLightValues[NUM_TWINKLE_LIGHTS];
int twinkleLightRising[NUM_TWINKLE_LIGHTS];
CRGB twinkleLightColors[NUM_TWINKLE_LIGHTS];

int _doOnePixelArray[NUM_LEDS];
int _doOnePixelMaxDelay = 100;  // in ms
int _doOnePixelIndex = 0;

CRGB _fullWhite = CRGB(255, 255, 255);
CRGB _fullRed = CRGB(255, 0, 0);
CRGB _fullGreen = CRGB(0, 255, 0);
CRGB _fullBlue = CRGB(0, 0, 255);
CRGB _lightWhite = CRGB(60, 60, 60);
CRGB _lightRed = CRGB(60, 0, 0);
CRGB _lightGreen = CRGB(0, 60, 0);
CRGB _lightBlue = CRGB(0, 0, 60);
CRGB _lightYellow = CRGB(60, 60, 0);
CRGB _lightAqua = CRGB(0, 60, 60);
CRGB _lightPurple = CRGB(60, 0, 60);
CRGB _lightPink = CRGB(100, 30, 30);
CRGB _lightTeal = CRGB(30, 100, 30);
CRGB _black = CRGB(0, 0, 0);

CRGB _christmasGreen = CRGB(20, 51, 6);

int _slideOffset = 0;

int _doChase = 1;
int _chaseLED = -1;
CRGB _colorBefore = _black;

bool _doRGB;
bool _doRandomize;
bool _doSlide;
bool _doRandomSide;
bool _doOnePixel;
bool _doChristmasTree;

// For the pushbutton mode
const int _buttonPin = 2;
int _lastButtonPress = LOW;
unsigned long _lastButtonPressTime;

unsigned long _lastSwitch;
unsigned long _rgbSwitch;
int _rgbColor;  // 0=r, 1=g, 2=b

unsigned long _lastSlideUpdateTime;
unsigned long _lastRandomizeUpdateTime;

void setAllColor(int r, int g, int b) {
  for (int ii = 0; ii < NUM_LEDS; ++ii) {
    leds[ii] = CRGB(r, g, b);
  }
  FastLED.show();
}

void setNextRGBColor() {
  if (_rgbColor == 0) {
    _rgbColor = 1;
    setAllColor(0, 155, 0);
  } else if (_rgbColor == 1) {
    _rgbColor = 2;
    setAllColor(0, 0, 155);
  } else if (_rgbColor == 2) {
    _rgbColor = 0;
    setAllColor(155, 0, 0);
  }
}

void rgb() {
  unsigned long currentTime = millis();
  if (currentTime - _rgbSwitch >= 500) {
    _rgbSwitch = currentTime;
    setNextRGBColor();
  }

  delay(5);
}

void setRandomColor(int led) {
  leds[led] = getRandomColor();
}

CRGB getRandomColor() {
  return CRGB(random(0, 150), random(0, 150), random(0, 150));
}

void randomize() {
  unsigned long currentTime = millis();

  if (currentTime - _lastRandomizeUpdateTime >= 500)
  {
    for (int ii = 0; ii < NUM_LEDS; ++ii) {
      leds[ii] = getRandomColor();
    }
    FastLED.show();
    _lastRandomizeUpdateTime = currentTime;
  }
  delay(5);
}

CRGB oneOfFour() {
  int val = random(0, 4);
  {
    if (val == 0)
      return _fullWhite;
    if (val == 1)
      return _fullRed;
    if (val == 2)
      return _fullGreen;
    if (val == 3)
      return _fullBlue;

    return _black;
  }
}

CRGB oneOfFive(int ii) {
  int val = ii % 5;
  if (val == 0)
    return _fullWhite;
  if (val == 1)
    return _fullRed;
  if (val == 2)
    return _fullGreen;
  if (val == 3)
    return _fullBlue;
  if (val == 4)
    return _black;

  return _black;
}

void allOff() {
  for (int ii = 0; ii < NUM_LEDS; ++ii) {
    leds[ii] = CRGB(0, 0, 0);
  }
  FastLED.show();
}

void slide() {
  unsigned long currentTime = millis();
  if (currentTime - _lastSlideUpdateTime > 250)
  {
    for (int ii = 0; ii < NUM_LEDS; ++ii) {
      leds[ii] = oneOfFive(ii + _slideOffset);
    }

    ++_slideOffset;
    if (_slideOffset == 4)
      _slideOffset = 0;

    FastLED.show();
    _lastSlideUpdateTime = currentTime;
  }
  //delay(250);
  delay(5);
}

void christmasTree() {
  for (int ii = 0; ii < NUM_LEDS; ++ii) {
    leds[ii] = _christmasGreen;
  }
  FastLED.show();
}

// Return true if the light index is already set to be one of the twinkle
// lights to avoid hitting the same one twice.
bool alreadyTwinkleLight(int lightIndex) {
  for (int ii = 0; ii < NUM_TWINKLE_LIGHTS; ++ii) {
    if (twinkleLights[ii] == NOT_SET_TWINKLE_LIGHT) {
      return false;
    }

    if (lightIndex == twinkleLights[ii]) {
      return true;
    }
  }

  return false;
}

void setupTwinkleLights() {
  // initialize the twinkle light globals
  for (int ii = 0; ii < NUM_TWINKLE_LIGHTS; ++ii) {
    twinkleLights[ii] = NOT_SET_TWINKLE_LIGHT;
    twinkleLightValues[ii] = random(SCALE_MIN, SCALE_MAX);
    twinkleLightRising[ii] = random(0, 2);
    twinkleLightColors[ii] = oneOfFour();
  }

  // pick the twinkle light LED indices
  int lightIndex = 0;
  for (int ii = 0; ii < NUM_TWINKLE_LIGHTS; ++ii) {
    lightIndex = random(0, NUM_LEDS);
    while (alreadyTwinkleLight(lightIndex)) {
      lightIndex = random(0, NUM_LEDS);
    }

    twinkleLights[ii] = lightIndex;
  }
}

void showTwinkleLights() {
  for (int ii = 0; ii < NUM_TWINKLE_LIGHTS; ++ii) {
    CRGB color = twinkleLightColors[ii];
    leds[twinkleLights[ii]] = color.scale8(twinkleLightValues[ii]);
  }

  delay(25);
  FastLED.show();
}

void updateTwinkleLights() {
  for (int ii = 0; ii < NUM_TWINKLE_LIGHTS; ++ii) {
    int offset = random(1, 10);

    if (twinkleLightRising[ii]) {
      twinkleLightValues[ii] = twinkleLightValues[ii] + offset;
      if (twinkleLightValues[ii] >= SCALE_MAX) {
        twinkleLightValues[ii] = SCALE_MAX;
        twinkleLightRising[ii] = 0;
      }
    } else {
      twinkleLightValues[ii] = twinkleLightValues[ii] - offset;
      if (twinkleLightValues[ii] <= SCALE_MIN) {
        twinkleLightValues[ii] = SCALE_MIN;
        twinkleLightRising[ii] = 1;
      }
    }
  }
}

void chase() {
  if (_chaseLED < 0)
    return;

  if (_chaseLED > 0)
    leds[_chaseLED - 1] = _colorBefore;

  _colorBefore = leds[_chaseLED];
  leds[_chaseLED] = _fullWhite;

  _chaseLED += 1;
  if (_chaseLED >= END_OUTER_LOOP) {
    FastLED.show();
    delay(10);
    leds[_chaseLED - 1] = _colorBefore;
    _chaseLED = -1;
  }

  FastLED.show();
}

bool startChase() {
  if (_chaseLED > 0)
    return false;

  if (random(1, 10001) > 9990) {
    _chaseLED = 0;
  }
}

void rowOne(CRGB color) {
  for (int i = 0; i < 30; ++i) {
    leds[i] = color;
  }
}

void rightSide(CRGB color) {
  for (int i = 30; i < 76; ++i) {
    leds[i] = color;
  }
}

void leftSide(CRGB color) {
  for (int i = 76; i < 121; ++i) {
    leds[i] = color;
  }
}

void rowTwo(CRGB color) {
  for (int i = 121; i < 145; ++i) {
    leds[i] = color;
  }
}

void rowThree(CRGB color) {
  for (int i = 145; i < 164; ++i) {
    leds[i] = color;
  }
}

void rowFour(CRGB color) {
  for (int i = 164; i < 178; ++i) {
    leds[i] = color;
  }
}

void rowFive(CRGB color) {
  for (int i = 178; i < 186; ++i) {
    leds[i] = color;
  }
}

void rowSix(CRGB color) {
  for (int i = 186; i < 190; ++i) {
    leds[i] = color;
  }
}

void randomSide(CRGB color) {
  int side = random(0, 8);
  if (side == 0)
    rowOne(color);
  else if (side == 1)
    rowTwo(color);
  else if (side == 2)
    rowThree(color);
  else if (side == 3)
    rowFour(color);
  else if (side == 4)
    rowFive(color);
  else if (side == 5)
    rowSix(color);
  else if (side == 6)
    leftSide(color);
  else if (side == 7)
    rightSide(color);

  delay(100);
}

// Set-up the array of pixel
void initDoOnePixel() {
  allOff();

  Set allPixels;
  for (int ii = 0; ii < NUM_LEDS; ++ii) {
    allPixels.add(ii);
  }

  int index = 0;
  int allPixelsCount = allPixels.count();
  while (index < NUM_LEDS) {
    int value = allPixels.getNth(random(0, allPixels.count()));
    _doOnePixelArray[index] = value;
    allPixels.sub(value);

    ++index;
  }
}

// Turn on one pixel at a time until the whole tree is lit, speed up
// as each pixel is activated, then pause at the end and restart.
void doOnePixel() {
  if (_doOnePixelIndex == 0) {
    initDoOnePixel();
  } else if (_doOnePixelIndex == NUM_LEDS) {
    delay(2000);
    _doOnePixelIndex = 0;
    return;
  }

  leds[_doOnePixelArray[_doOnePixelIndex]] = _christmasGreen;

  int onePixelDelay = int(_doOnePixelMaxDelay - _doOnePixelMaxDelay * ((float)_doOnePixelIndex / (float)NUM_LEDS));
  if (onePixelDelay < 0)
    onePixelDelay = 0;

  delay(onePixelDelay);

  ++_doOnePixelIndex;
}

void clearAll() {
  _doRGB = false;
  _doRandomize = false;
  _doSlide = false;
  _doRandomSide = false;
  _doOnePixel = false;
  _doChristmasTree = false;
}

void runChristmasTreePattern() {
  clearAll();
  _doChristmasTree = true;
}

void runRandomPattern() {
  clearAll();
  int pick = random(6);

  if (pick == 0) {
    _doRGB = true;
    _rgbColor = 2;
    _rgbSwitch = millis();
  } else if (pick == 1) {
    _doRandomize = true;
    _lastRandomizeUpdateTime = millis();
  } else if (pick == 2) {
    _doSlide - true;
    _lastSlideUpdateTime = millis();
  } else if (pick == 3) {
    _doRandomSide = true;
  } else if (pick == 4) {
    _doOnePixel = true;
    _doOnePixelIndex = 0;
  } else {
    _doChristmasTree = true;
    christmasTree();
  }
}

void runNextPattern() {
  if (_doRGB) {
    _doRGB = false;
    _doRandomize = true;
    _lastRandomizeUpdateTime = millis();
  } else if (_doRandomize) {
    _doRandomize = false;
    _doSlide = true;
    _lastSlideUpdateTime = millis();
  } else if (_doSlide) {
    _doSlide = false;
    _doRandomSide = true;
  } else if (_doRandomSide) {
    _doRandomSide = false;
    _doOnePixel = true;
    _doOnePixelIndex = 0;
  } else if (_doOnePixel) {
    _doOnePixel = false;
    _doChristmasTree = true;
    christmasTree();
  } else if (_doChristmasTree) {
    _doChristmasTree = false;
    _doRGB = true;
    _rgbColor = 2;
    _rgbSwitch = millis();
  }
}

bool notRunningChristmasTree() {
  return !_doChristmasTree;
}

// Primarily run christmas tree, but every ten minutes switch to one of the other ones
// for two minutes.
void checkProgramSwitch() {
  // check if the pushbutton is pressed
  unsigned long currentTime = millis();
  if (_lastButtonPress == LOW && currentTime - _lastButtonPressTime > 500 && digitalRead(_buttonPin) == HIGH) {
    runNextPattern();
    _lastSwitch = millis();
    _lastButtonPress = HIGH;
    _lastButtonPressTime = currentTime;
    return;
  }
  else
  {
    _lastButtonPress = LOW;
  }

  unsigned long elapsedTime = millis() - _lastSwitch;

  if (notRunningChristmasTree()) {
    if (elapsedTime > 120000)  // two minutes
    {
      runChristmasTreePattern();
      christmasTree();
      _lastSwitch = millis();
    }
  } else {
    if (elapsedTime > 600000)  // ten minutes
    {
      runRandomPattern();
      _lastSwitch = millis();
    }
  }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Setup
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

  randomSeed(analogRead(0));

  // initialize the pushbutton pin as an input:
  pinMode(_buttonPin, INPUT);

  _lastSwitch = millis();
  _lastButtonPressTime = _lastSwitch;

  clearAll();
  allOff();
  christmasTree();
  setupTwinkleLights();

  runChristmasTreePattern();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// The Main Loop
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
void loop() {
  // Primarily run the christmasTree, every ten minutes run one of the others for two minutes.
  // If a button press was detected it switches to the next pattern.
  checkProgramSwitch();

  if (_doRGB) {
    rgb();
  } else if (_doRandomize) {
    randomize();
  } else if (_doSlide) {
    slide();
  } else if (_doRandomSide) {
    randomSide(getRandomColor());
  } else if (_doOnePixel) {
    doOnePixel();
  } else if (_doChristmasTree) {
    showTwinkleLights();
    updateTwinkleLights();

    if (_doChase) {
      startChase();
      chase();
      delay(10);
    }
  }

  FastLED.show();
}