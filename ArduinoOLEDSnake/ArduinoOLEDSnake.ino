#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Title.h"

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

const int D_NORTH = 1;
const int D_EAST = 2;
const int D_SOUTH = 3;
const int D_WEST = 4;

const int DISP_OFF = 17;

long score = 0;

byte snakePixelX[20];
byte snakePixelY[20];

int snakeX = 10;
int snakeY = 30;
int snakeLength = 3;
volatile int snakeDir = D_NORTH;
volatile int buttonState = LOW;

int minX = 0;
int minY = DISP_OFF;
int maxX = 128;
int maxY = 64;

int foodX = 0;
int foodY = 0;

const int BUTTON_PIN1 = 2;
const int BUTTON_PIN2 = 3;

void setupButton() {
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN1), debounceChangeDirection1, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN2), debounceChangeDirection2, FALLING);
}

long debouncing_time = 100; //Debouncing Time in Milliseconds
volatile unsigned long last_micros1;
void debounceChangeDirection1() {
  if((long)(micros() - last_micros1) >= debouncing_time * 1000) {
    changeDirection(true);
    last_micros1 = micros();
  }
}

volatile unsigned long last_micros2;
void debounceChangeDirection2(boolean up) {
  if((long)(micros() - last_micros2) >= debouncing_time * 1000) {
    changeDirection(false);
    last_micros2 = micros();
  }
}

void changeDirection(boolean up) {
  switch(snakeDir) {
    case D_NORTH:
      if (up) snakeDir = D_EAST; else snakeDir = D_WEST;
      break;
    case D_EAST:
      if (up) snakeDir = D_SOUTH; else snakeDir = D_NORTH;
      break;
    case D_SOUTH:
      if (up) snakeDir = D_WEST; else snakeDir = D_EAST;
      break;
    case D_WEST:
      if (up) snakeDir = D_NORTH; else snakeDir = D_SOUTH;
      break;
  }

  buttonState = !buttonState;
}

void setupScreen()   {
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done

  display.clearDisplay();

  minX = 0;
  minY = DISP_OFF;
  maxX = display.width()-1;
  maxY = display.height()-1;

  display.drawXBitmap(0,0, Title_bits, display.width(), display.height(), WHITE);
  display.display();
  delay(5000);
  
  display.clearDisplay();
  renderScore();
}

void setup()   {
  setupButton();
  setupScreen();

  dropFood();

  digitalWrite(BUTTON_PIN1, buttonState);
  digitalWrite(BUTTON_PIN2, buttonState);
}

bool outOfBounds() {
  return snakeX <= minX || snakeY <= minY || snakeX >= maxX || snakeY >= maxY;
}

void gameOver() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(8, 10);
  display.print("GAME OVER     :(");
  display.display();

  score = 0;
  snakeLength = 3;
  snakeX = display.width() / 2;
  snakeY = display.height() / 2 + DISP_OFF;

  snakePixelX[snakeLength-1] = snakeX;
  snakePixelY[snakeLength-1] = snakeY;
  
  snakeDir = D_SOUTH;
  
  delay(2000);
  
  display.clearDisplay();
  renderScore();
}

// Drop food on random location
void dropFood() {
  foodX = random(minX+5, maxX-5);
  foodY = random(minY+5, maxY-5);
}

bool collectFood() {
  if (snakeX == foodX && snakeY == foodY) {
    score += 10;

    tone(8, 1000, 10);
    tone(8, 1020, 10);
    tone(8, 1040, 10);
    tone(8, 1000, 10);
    tone(8, 1000, 10);
    
    renderScore();
    dropFood();

    return true;
  } else {
    return false;
  }
}

void renderScore() {
  display.fillRect(0, 0, display.width()-1, DISP_OFF, BLACK);
  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  display.print("Score: ");
  display.print(String(score, DEC));

  // TOP
  display.drawLine(0, DISP_OFF, display.width()-1, DISP_OFF, WHITE);
  // LEFT
  display.drawLine(0, DISP_OFF, 0, display.height()-1, WHITE);
  // RIGHT
  display.drawLine(display.width()-1, DISP_OFF, display.width()-1, display.height()-1, WHITE);
  // BOTTOM
  display.drawLine(0, display.height()-1, display.width()-1, display.height()-1, WHITE);
}

bool crashedIntoSelf() {
  for(byte i = 4; i < snakeLength; i++) {
      if (snakeX == snakePixelX[i] && snakeY == snakePixelY[i]) {
        return true;
      }
  }

  return false;
}

void drawScreen() {
  bool foodCollected = false;
  
    // Clear the buffer.
  display.clearDisplay();

  display.drawPixel(foodX, foodY, WHITE);
  foodCollected  = collectFood();

  // Check snake position
  if (outOfBounds() || crashedIntoSelf()) {
    gameOver();
  }

  // Render the snake
  for(int i = 0; i < snakeLength; i++) {
    display.drawPixel(snakePixelX[i], snakePixelY[i], WHITE);
  }
  
  // Move pixel values
  for(int i = snakeLength; i > 0; i--) {
    snakePixelX[i] = snakePixelX[i-1];
    snakePixelY[i] = snakePixelY[i-1];
  }

  if (foodCollected) {
    snakeLength += 1;
    snakePixelX[snakeLength-1] = snakeX;
    snakePixelY[snakeLength-1] = snakeY;
  }

  switch(snakeDir) {
    case D_NORTH:
      snakeY -= 1;
      break;
    case D_EAST:
      snakeX += 1;
      break;
    case D_SOUTH:
      snakeY += 1;
      break;
    case D_WEST:
      snakeX -= 1;
      break;
  }

  snakePixelX[0] = snakeX;
  snakePixelY[0] = snakeY;

  renderScore();
  display.display();
}

void loop() {

  drawScreen();
  
}
