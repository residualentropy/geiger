//---------------------//
// Geiger Counter Code //
//---------------------//

#include <LiquidCrystal.h>

const int PIEZO = 8;
const int HVTRIG = 7;
const int RESP = 2;

const int TIME = 700;// 219; // 219 @ 3V, 145 @ 5V

const double FREQ_KHZ = 1.43;
const double FREQ_KHZ_DIGITS = 4;

const int ITERS_BETWEEN_REPORTING = 100;

volatile int counts = 0;
int countsBeforeLastBeep = 0;

const int LCD_RS = 12, LCD_EN = 11, LCD_D4 = 3, LCD_D5 = 4, LCD_D6 = 5, LCD_D7 = 6;

const int CPM_INTERVAL_SECS = 15;
const int CPM_INTERVAL_SECS_DIGITS = 2;
const double CPM_INTERVAL_MINS = ((double)CPM_INTERVAL_SECS) / 60;

const byte LCDBG_ONE [7] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111
};

const byte LCDBG_TWO [7] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111
};

const byte LCDBG_THREE [7] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111
};

const byte LCDBG_FOUR [7] = {
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111,
  B11111
};

const byte LCDBG_GTFOUR [7] = {
  B00100,
  B01010,
  B10001,
  B00000,
  B11111,
  B11111,
  B11111
};

LiquidCrystal lcd (LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

int cpsCountsBefore = 0;
long cpsTimeBefore = 0;

int rollingCps [CPM_INTERVAL_SECS];

void setup() {
  // put your setup code here, to run once:
  lcd.begin(16, 2);
  pinMode(HVTRIG, OUTPUT);
  pinMode(RESP, INPUT);
  attachInterrupt(digitalPinToInterrupt(RESP), isr_resp, RISING);
  lcd.createChar(0, LCDBG_ONE);
  lcd.createChar(1, LCDBG_TWO);
  lcd.createChar(2, LCDBG_THREE);
  lcd.createChar(3, LCDBG_FOUR);
  lcd.createChar(4, LCDBG_GTFOUR);
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i = 0; i < ITERS_BETWEEN_REPORTING; i++) {
    digitalWrite(HVTRIG, LOW);
    delayMicroseconds(TIME / 2);
    digitalWrite(HVTRIG, HIGH);
    delayMicroseconds(TIME / 2);
  }
  int countsNow = counts;
  if (countsNow > countsBeforeLastBeep) {
    tone(PIEZO, 740, 10);
    countsBeforeLastBeep = countsNow;
  }
  if ((millis() - cpsTimeBefore) >= 1000) {
    int cpsNow = countsNow - cpsCountsBefore;
    for (int i = CPM_INTERVAL_SECS - 1; i > 0; i--) {
      rollingCps[i] = rollingCps[i - 1];
    }
    rollingCps[0] = cpsNow;
    cpsCountsBefore = countsNow;
    cpsTimeBefore = millis();
  }
  int clicksInCpmInterval = 0;
  for (int i = 0; i < CPM_INTERVAL_SECS; i++) {
    clicksInCpmInterval += rollingCps[i];
  }
  double cpmNow = clicksInCpmInterval / CPM_INTERVAL_MINS;
  lcd.clear();
  lcd.setCursor(0, 0);
  for (int i = 0; i < CPM_INTERVAL_SECS; i++) {
    int x = rollingCps[i];
    if (x == 0) {
      lcd.print(" ");
    } else if (x > 4) {
      lcd.write(byte(4));
    } else {
      lcd.write(byte(x - 1));
    }
  }
  //lcd.print("Total: ");
  //lcd.print(countsNow);
  lcd.setCursor(0, 1);
  lcd.print((int)cpmNow);
  lcd.print(" CPM");
  if ((millis() / 1000) <= CPM_INTERVAL_SECS) {
    lcd.print(" (WAIT!)");
  } else {
    switch ((millis() / 1000) % 6) {
      case 0:
        lcd.setCursor(16 - 6, 1);
        lcd.print("SBM-20");
        break;
      case 1:
        lcd.setCursor(16 - 4, 1);
        lcd.print("400V");
        break;
      case 2:
        lcd.setCursor(16 - 5, 1);
        lcd.print("4.7M");
        lcd.print((char)244);
        break;
      case 3:
        lcd.setCursor(16 - FREQ_KHZ_DIGITS - 3, 1);
        lcd.print(FREQ_KHZ);
        lcd.print("KHz");
        break;
      case 4:
        lcd.setCursor(16 - 5, 1);
        lcd.print("@10mH");
        break;
      case 5:
        lcd.setCursor(16 - CPM_INTERVAL_SECS_DIGITS - 1, 1);
        lcd.print(CPM_INTERVAL_SECS);
        lcd.print("s");
        break;
    }
  }
}

void isr_resp() {
  counts++;
}
