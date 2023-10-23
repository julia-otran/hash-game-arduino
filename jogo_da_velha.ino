#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>

#include <Adafruit_SH110X.h>

// Display Setitings
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Display
// OLED_SCL 13
// OLED_SDA (MOSI) 11
#define OLED_RST   10
#define OLED_DC    9
#define OLED_CS    -1

// Create the OLED display
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, OLED_DC, OLED_RST, OLED_CS, 8000000);

#define INIT_DSP display.begin(0, true)
#define DSP_COLOR_WHITE SH110X_WHITE
#define DSP_COLOR_BLACK SH110X_BLACK

// Default state: 0v
// Pressed state: 5v
// Use pull down resistors
#define KEY_SWITCH PIND2
#define KEY_SELECT PIND3

#define SYMBOL_O 0
#define SYMBOL_X 1
#define NO_PLAYER 255

// Every line is a uint8_t int.
// Every column is a 2 bit part of line integer
//
// Every 2 bit group means:
// The MSB means if plaze is in use
// The LSB means the player of the plaze, only if plaze is in use
uint8_t positions[3];
uint8_t personPlayerIs;

// ---------------------------
// Table Manipulation Helpers
// ---------------------------
uint8_t isPlaceFree(uint8_t x, uint8_t y) {
  uint8_t xStack = positions[x];
  uint8_t plaze = xStack >> (y * 2);
  return ((plaze >> 1) & 0x1) == 0;
}

uint8_t isRowFull(uint8_t x) {
  return isPlaceFree(x, 0) == 0 && isPlaceFree(x, 1) == 0 && isPlaceFree(x, 2) == 0;
}

uint8_t isColumnFull(uint8_t y) {
  return isPlaceFree(0, y) == 0 && isPlaceFree(1, y) == 0 && isPlaceFree(2, y) == 0;
}

uint8_t isFirstDiagonalFull() {
  return isPlaceFree(0, 0) == 0 && isPlaceFree(1, 1) == 0 && isPlaceFree(2, 2) == 0;
}

uint8_t isSecondDiagonalFull() {
  return isPlaceFree(0, 2) == 0 && isPlaceFree(1, 1) == 0 && isPlaceFree(2, 0) == 0;
}

uint8_t getPlayerOfPlace(uint8_t x, uint8_t y) {
  if (isPlaceFree(x, y)) {
    return NO_PLAYER;
  }

  uint8_t xStack = positions[x];
  uint8_t plaze = xStack >> (y * 2);
  return plaze & 0x1;
}

void setPlacePlayerX(uint8_t x, uint8_t y) {
  uint8_t xStack = positions[x];
  uint8_t plaze = SYMBOL_X << (y * 2);
  plaze = plaze | (1 << (y * 2) + 1);

  positions[x] = xStack | plaze;
}

void setPlacePlayerO(uint8_t x, uint8_t y) {
  uint8_t xStack = positions[x];
  uint8_t plaze = SYMBOL_O << (y * 2);
  plaze = plaze | (1 << (y * 2) + 1);

  positions[x] = xStack | plaze;
}

uint8_t getWinPlayer() {
  if (
    isRowFull(0) && getPlayerOfPlace(0, 0) == getPlayerOfPlace(0, 1) && getPlayerOfPlace(0, 1) == getPlayerOfPlace(0, 2)
  ) {
    return getPlayerOfPlace(0, 2);
  }

  if (
    isRowFull(1) && getPlayerOfPlace(1, 0) == getPlayerOfPlace(1, 1) && getPlayerOfPlace(1, 1) == getPlayerOfPlace(1, 2)
  ) {
    return getPlayerOfPlace(1, 2);
  }

  if (
    isRowFull(2) && getPlayerOfPlace(2, 0) == getPlayerOfPlace(2, 1) && getPlayerOfPlace(2, 1) == getPlayerOfPlace(2, 2)
  ) {
    return getPlayerOfPlace(2, 2);
  }

  if (
    isColumnFull(0) && getPlayerOfPlace(0, 0) == getPlayerOfPlace(1, 0) && getPlayerOfPlace(1, 0) == getPlayerOfPlace(2, 0)
  ) {
    return getPlayerOfPlace(2, 0);
  }

  if (
    isColumnFull(1) && getPlayerOfPlace(0, 1) == getPlayerOfPlace(1, 1) && getPlayerOfPlace(1, 1) == getPlayerOfPlace(2, 1)
  ) {
    return getPlayerOfPlace(2, 1);
  }

  if (
    isColumnFull(2) && getPlayerOfPlace(0, 2) == getPlayerOfPlace(1, 2) && getPlayerOfPlace(1, 2) == getPlayerOfPlace(2, 2)
  ) {
    return getPlayerOfPlace(2, 2);
  }

  if (
    isFirstDiagonalFull() && getPlayerOfPlace(0, 0) == getPlayerOfPlace(1, 1) && getPlayerOfPlace(1, 1) == getPlayerOfPlace(2, 2)
  ) {
    return getPlayerOfPlace(2, 2);
  }

  if (
    isSecondDiagonalFull() && getPlayerOfPlace(0, 2) == getPlayerOfPlace(1, 1) && getPlayerOfPlace(1, 1) == getPlayerOfPlace(2, 0)
  ) {
    return getPlayerOfPlace(2, 0);
  }

  return NO_PLAYER;
}

uint8_t endOfGame() {
  if (getWinPlayer() != NO_PLAYER) {
    return 1;
  }

  return isRowFull(0) && isRowFull(1) && isRowFull(2);
}

uint8_t isRowWon(uint8_t x) {
  return isRowFull(x) && getPlayerOfPlace(x, 0) == getPlayerOfPlace(x, 1) && getPlayerOfPlace(x, 1) == getPlayerOfPlace(x, 2);
}

uint8_t isColumnWon(uint8_t y) {
  return isColumnFull(y) && getPlayerOfPlace(0, y) == getPlayerOfPlace(1, y) && getPlayerOfPlace(1, y) == getPlayerOfPlace(2, y);
}

uint8_t isFirstDiagonalWon() {
  return isFirstDiagonalFull() && getPlayerOfPlace(0, 0) == getPlayerOfPlace(1, 1) && getPlayerOfPlace(1, 1) == getPlayerOfPlace(2, 2);
}

uint8_t isSecondDiagonalWon() {
  return isSecondDiagonalFull() && getPlayerOfPlace(0, 2) == getPlayerOfPlace(1, 1) && getPlayerOfPlace(1, 1) == getPlayerOfPlace(2, 0);
}

// Printing functions

#define SYMBOL_X_PAD 19
#define SYMBOL_Y_PAD 7

uint16_t getCharColorForPosition(uint8_t x, uint8_t y) {
  if (isRowWon(x) || isColumnWon(y)) {
    return DSP_COLOR_BLACK;
  }

  if (isFirstDiagonalWon() && x == y) {
    return DSP_COLOR_BLACK;
  } 

  if (isSecondDiagonalWon() && x == (2 - y)) {
    return DSP_COLOR_BLACK;
  }

  return DSP_COLOR_WHITE;
}

void printTable() {
  display.drawLine(0, 21, SCREEN_WIDTH, 21, DSP_COLOR_WHITE);
  display.drawLine(0, 42, SCREEN_WIDTH, 42, DSP_COLOR_WHITE);

  display.drawLine(42, 0, 42, SCREEN_HEIGHT, DSP_COLOR_WHITE);
  display.drawLine(84, 0, 84, SCREEN_HEIGHT, DSP_COLOR_WHITE);

  for (uint8_t i = 0; i < 3; i++) {
    if (isRowWon(i)) {
      display.fillRect(0, (i * 21), SCREEN_WIDTH, 22, DSP_COLOR_WHITE);
    }
    if (isColumnWon(i)) {
      display.fillRect((i * 42), 0, 43, SCREEN_HEIGHT, DSP_COLOR_WHITE);
    }
  }

  if (isFirstDiagonalWon()) {
    display.fillRect(0, 0, 42, 21, DSP_COLOR_WHITE);
    display.fillRect(42, 21, 42, 21, DSP_COLOR_WHITE);
    display.fillRect(84, 42, 42, 21, DSP_COLOR_WHITE);
  }

  if (isSecondDiagonalWon()) {
    display.fillRect(84, 0, 42, 21, DSP_COLOR_WHITE);
    display.fillRect(42, 21, 42, 21, DSP_COLOR_WHITE);
    display.fillRect(0, 42, 42, 21, DSP_COLOR_WHITE);
  }

  display.setTextSize(1);

  for (uint8_t x = 0; x < 3; x++) {
    for (uint8_t y = 0; y < 3; y++) {
      if (isPlaceFree(x, y) == 0) {
        display.setCursor((y * 42) + SYMBOL_X_PAD, (x * 21) + SYMBOL_Y_PAD);
        display.setTextColor(getCharColorForPosition(x, y));

        if (getPlayerOfPlace(x, y) == SYMBOL_X) {
          display.print("X");
        } else {
          display.print("O");
        }
      }
    }
  }
}

void printSelectedPosition(uint8_t sx, uint8_t sy, uint8_t symbol) {
  display.setTextSize(1);

  for (uint8_t x = 0; x < 3; x++) {
    for (uint8_t y = 0; y < 3; y++) {
      if (sx == x && sy == y) {
        display.fillRect((42*y) + 3, (21*x) + 3, 36, 15, DSP_COLOR_WHITE);
        display.setCursor((42*y) + SYMBOL_X_PAD, (21*x) + SYMBOL_Y_PAD);
        display.setTextColor(DSP_COLOR_BLACK);

        if (symbol == SYMBOL_X) {
          display.print("X");
        } else {
          display.print("O");
        }
      }
    }
  }
}

// Show Player Turn
void showPlayerTurn(uint8_t symbol) {
  display.clearDisplay();
  display.setCursor(10, 0);
  display.setTextSize(1);
  display.setTextColor(DSP_COLOR_WHITE);
  display.print("Agora e a vez de:");

  display.setCursor(55, 20);
  display.setTextSize(3);

  if (symbol == SYMBOL_X) {
    display.print("X");
  } else {
    display.print("O");
  }

  display.setTextSize(1);

  if (symbol == personPlayerIs) {
    display.setCursor(45, 56);
    display.print("(Voce)");
  } else {
    display.setCursor(28, 56);
    display.print("(Computador)");
  }

  display.display();

  delay(3000);
}

// ----------------------
// Person Player Functions
// ----------------------

void personPlay() {
  showPlayerTurn(personPlayerIs);

  uint8_t selected = 0;

  while (1) {
    display.clearDisplay();

    for (uint8_t check = 0; check < 9; check++) {
      if (isPlaceFree(selected / 3, selected % 3) == 0) {
        selected++;
      }

      if (selected > 8) {
        selected = 0;
      }
    }

    printTable();
    printSelectedPosition(selected / 3, selected % 3, personPlayerIs);

    display.display();

    if (digitalRead(KEY_SWITCH) == HIGH) {
      selected++;
      delay(200);
    }

    if (digitalRead(KEY_SELECT) == HIGH) {
      uint8_t x = selected / 3;
      uint8_t y = selected % 3;

      if (isPlaceFree(x, y)) {
        if (personPlayerIs == SYMBOL_X) {
          setPlacePlayerX(x, y);
        } else {
          setPlacePlayerO(x, y);
        }

        break;
      }
    }
  }

  display.clearDisplay();
  printTable();
  display.display();
  
  delay(3000);
}

// Computer play functions

#define IMPOSSIBLE_PLAY 255

uint8_t getWinnerPoint() {
  uint8_t computerSymbol;

  if (personPlayerIs == SYMBOL_X) {
    computerSymbol = SYMBOL_O;
  } else {
    computerSymbol = SYMBOL_X;
  }

  for (uint8_t x = 0; x < 3; x++) {
    if (isRowFull(x) == 0) {
      if (getPlayerOfPlace(x, 0) == computerSymbol && getPlayerOfPlace(x, 1) == computerSymbol) {
        return (x << 4) | (2);
      }
      if (getPlayerOfPlace(x, 1) == computerSymbol && getPlayerOfPlace(x, 2) == computerSymbol) {
        return (x << 4) | (0);
      }
      if (getPlayerOfPlace(x, 0) == computerSymbol && getPlayerOfPlace(x, 2) == computerSymbol) {
        return (x << 4) | (1);
      }
    }
  }

  for (uint8_t y = 0; y < 3; y++) {
    if (isColumnFull(y) == 0) {
      if (getPlayerOfPlace(0, y) == computerSymbol && getPlayerOfPlace(1, y) == computerSymbol) {
        return (2 << 4) | (y);
      }
      if (getPlayerOfPlace(1, y) == computerSymbol && getPlayerOfPlace(2, y) == computerSymbol) {
        return (0 << 4) | (y);
      }
      if (getPlayerOfPlace(0, y) == computerSymbol && getPlayerOfPlace(2, y) == computerSymbol) {
        return (1 << 4) | (y);
      }
    }
  }

  if (isFirstDiagonalFull() == 0) {
    if (getPlayerOfPlace(0, 0) == computerSymbol && getPlayerOfPlace(1, 1) == computerSymbol) {
      return (2 << 4) | (2);
    }
    if (getPlayerOfPlace(1, 1) == computerSymbol && getPlayerOfPlace(2, 2) == computerSymbol) {
      return (0 << 4) | (0);
    }
    if (getPlayerOfPlace(0, 0) == computerSymbol && getPlayerOfPlace(2, 2) == computerSymbol) {
      return (1 << 4) | (1);
    }
  }

  if (isSecondDiagonalFull() == 0) {
    if (getPlayerOfPlace(0, 2) == computerSymbol && getPlayerOfPlace(1, 1) == computerSymbol) {
      return (2 << 4) | (0);
    }
    if (getPlayerOfPlace(1, 1) == computerSymbol && getPlayerOfPlace(2, 0) == computerSymbol) {
      return (0 << 4) | (2);
    }
    if (getPlayerOfPlace(0, 2) == computerSymbol && getPlayerOfPlace(2, 0) == computerSymbol) {
      return (1 << 4) | (1);
    }
  }

  return IMPOSSIBLE_PLAY;
}

uint8_t getPreventLossPoint() {
  for (uint8_t x = 0; x < 3; x++) {
    if (isRowFull(x) == 0) {
      if (getPlayerOfPlace(x, 0) == personPlayerIs && getPlayerOfPlace(x, 1) == personPlayerIs) {
        return (x << 4) | (2);
      }
      if (getPlayerOfPlace(x, 1) == personPlayerIs && getPlayerOfPlace(x, 2) == personPlayerIs) {
        return (x << 4) | (0);
      }
      if (getPlayerOfPlace(x, 0) == personPlayerIs && getPlayerOfPlace(x, 2) == personPlayerIs) {
        return (x << 4) | (1);
      }
    }
  }

  for (uint8_t y = 0; y < 3; y++) {
    if (isColumnFull(y) == 0) {
      if (getPlayerOfPlace(0, y) == personPlayerIs && getPlayerOfPlace(1, y) == personPlayerIs) {
        return (2 << 4) | (y);
      }
      if (getPlayerOfPlace(1, y) == personPlayerIs && getPlayerOfPlace(2, y) == personPlayerIs) {
        return (0 << 4) | (y);
      }
      if (getPlayerOfPlace(0, y) == personPlayerIs && getPlayerOfPlace(2, y) == personPlayerIs) {
        return (1 << 4) | (y);
      }
    }
  }

  if (isFirstDiagonalFull() == 0) {
    if (getPlayerOfPlace(0, 0) == personPlayerIs && getPlayerOfPlace(1, 1) == personPlayerIs) {
      return (2 << 4) | (2);
    }
    if (getPlayerOfPlace(1, 1) == personPlayerIs && getPlayerOfPlace(2, 2) == personPlayerIs) {
      return (0 << 4) | (0);
    }
    if (getPlayerOfPlace(0, 0) == personPlayerIs && getPlayerOfPlace(2, 2) == personPlayerIs) {
      return (1 << 4) | (1);
    }
  }

  if (isSecondDiagonalFull() == 0) {
    if (getPlayerOfPlace(0, 2) == personPlayerIs && getPlayerOfPlace(1, 1) == personPlayerIs) {
      return (2 << 4) | (0);
    }
    if (getPlayerOfPlace(1, 1) == personPlayerIs && getPlayerOfPlace(2, 0) == personPlayerIs) {
      return (0 << 4) | (2);
    }
    if (getPlayerOfPlace(0, 2) == personPlayerIs && getPlayerOfPlace(2, 0) == personPlayerIs) {
      return (1 << 4) | (1);
    }
  }

  return IMPOSSIBLE_PLAY;
}

uint8_t getRecommendedPlayPoint() {
  uint8_t countPreferentialPoints = isPlaceFree(0, 0) + isPlaceFree(0, 2) + isPlaceFree(2, 0) + isPlaceFree(2, 2);
  uint8_t sorted;

  if (countPreferentialPoints == 0) {
    while (1) {
      sorted = random(0, 9);

      if (isPlaceFree(sorted / 3, sorted % 3)) {
        return ((sorted / 3) << 4) | (sorted % 3);
      }
    }
  }

  sorted = random(0, countPreferentialPoints);
  uint8_t countFree = 0;

  if (isPlaceFree(0, 0)) {
    if (countFree == sorted) {
      return (0 << 4) | 0;
    } else {
      countFree++;
    }
  }

  if (isPlaceFree(0, 2)) {
    if (countFree == sorted) {
      return (0 << 4) | 2;
    } else {
      countFree++;
    }
  }

  if (isPlaceFree(2, 0)) {
    if (countFree == sorted) {
      return (2 << 4) | 0;
    } else {
      countFree++;
    }
  }

  return (2 << 4) | 2;
}

uint8_t getComputerPlayPoint() {
  uint8_t point = getWinnerPoint();

  if (point != IMPOSSIBLE_PLAY) {
    return point;
  }

  point = getPreventLossPoint();

  if (point != IMPOSSIBLE_PLAY) {
    return point;
  }

  return getRecommendedPlayPoint();
}

void computerPlay() {
  showPlayerTurn(!personPlayerIs);

  uint8_t point = getComputerPlayPoint();

  display.clearDisplay();
  printTable();
  printSelectedPosition(point >> 4, point & 0xF, !personPlayerIs);
  display.display();
  delay(2000);

  if (personPlayerIs == SYMBOL_X) {
    setPlacePlayerO(point >> 4, point & 0xF);
  } else {
    setPlacePlayerX(point >> 4, point & 0xF);
  }

  display.clearDisplay();
  printTable();
  display.display();
  delay(3000);
}

// Winner announce

void announceFinish() {
  display.clearDisplay();
  display.setTextColor(DSP_COLOR_WHITE);
  display.setTextSize(2);

  if (getWinPlayer() == NO_PLAYER) {  
    display.setCursor(20, 24);
    display.print("Empatou!");
  } else if (getWinPlayer() == personPlayerIs) {
    display.setCursor(44, 10);
    display.print("Voce");
    display.setCursor(30, 34);
    display.print("Ganhou!");
  } else {
    display.setCursor(5, 10);
    display.print("Computador");
    display.setCursor(30, 34);
    display.print("Ganhou!");
  }

  display.display();

  while (1) {
    if (digitalRead(KEY_SELECT) == HIGH) {
      delay(200);
      break;
    }
  }
}

// ---------------------- 
// Main routine functions
// ----------------------

void setup() {
  if (INIT_DSP == false) {
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(DSP_COLOR_WHITE);
  display.print("Joguinho da\n");
  display.setCursor(20, 25);
  display.setTextSize(2);
  display.print("Velha #");
  display.display();

  delay(4000);

  personPlayerIs = SYMBOL_X;
}

void chooseSymbol() {
  uint8_t symbolXcolor, symbolOcolor, symbolSelected;

  symbolSelected = 0;

  while (symbolSelected == 0) {
    display.clearDisplay();
    display.setTextColor(DSP_COLOR_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Voce quer ser?");

    if (personPlayerIs == SYMBOL_X) {
      display.fillRect(0, 15, SCREEN_WIDTH / 2, SCREEN_HEIGHT - 15, DSP_COLOR_WHITE);
      symbolXcolor = DSP_COLOR_BLACK;
      symbolOcolor = DSP_COLOR_WHITE;
    } else {
      display.fillRect(SCREEN_WIDTH / 2, 15, SCREEN_WIDTH, SCREEN_HEIGHT - 15, DSP_COLOR_WHITE);
      symbolOcolor = DSP_COLOR_BLACK;
      symbolXcolor = DSP_COLOR_WHITE;
    }

    display.setTextSize(3);

    display.setCursor(25, 25);
    display.setTextColor(symbolXcolor);
    display.print("X");

    display.setCursor((SCREEN_WIDTH / 2) + 25, 25);
    display.setTextColor(symbolOcolor);
    display.print("O");

    display.display();

    if (digitalRead(KEY_SWITCH) == HIGH) {
      personPlayerIs = !personPlayerIs;
      delay(100);
    }

    while (digitalRead(KEY_SELECT) == HIGH) {
      symbolSelected = 1;
    }
  }
}


void loop() {
  // Clear table
  positions[0] = 0;
  positions[1] = 0;
  positions[2] = 0;

  chooseSymbol();

  while (1) {
    personPlay();

    if (endOfGame()) {
      break;
    }

    computerPlay();

    if (endOfGame()) {
      break;
    }
  }

  announceFinish();
}
