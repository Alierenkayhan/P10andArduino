#include <virtuabotixRTC.h>
#include <DMD2.h>
#include <fonts/SystemFont5x7.h>
#include <EEPROM.h>

#define CLK_PIN 5
#define DAT_PIN 4
#define RST_PIN 3
virtuabotixRTC RTC(CLK_PIN, DAT_PIN, RST_PIN);

SoftDMD dmd(1, 2);

#define EEPROM_FLAG_ADDR 0
#define EEPROM_FLAG_VAL  0xAB

int parseMonth(const char* s) {
  const char* months[] = {"Jan","Feb","Mar","Apr","May","Jun",
                           "Jul","Aug","Sep","Oct","Nov","Dec"};
  for (int i = 0; i < 12; i++)
    if (strncmp(s, months[i], 3) == 0) return i + 1;
  return 1;
}

int calcDayOfWeek(int y, int m, int d) {
  if (m < 3) { m += 12; y--; }
  int k = y % 100, j = y / 100;
  int h = (d + (13*(m+1))/5 + k + k/4 + j/4 + 5*j) % 7;
  return ((h + 6) % 7) + 1;
}

void twoDigit(char* buf, int val) {
  buf[0] = '0' + (val / 10) % 10;
  buf[1] = '0' + val % 10;
  buf[2] = '\0';
}

const char* ayIsmi(int ay) {
const char* aylar[] = {"Ocak","Subat","Mart","Nisan","Mayis","Haziran",
                        "Temmuz","Agustos","Eylul","Ekim","Kasim","Aralik"};
  if (ay < 1 || ay > 12) return "???";
  return aylar[ay - 1];
}

void clearRegion(int y1, int y2) {
  dmd.drawFilledBox(0, y1, 31, y2, GRAPHICS_OFF);
}

void drawCentered(const char* text, int y) {
  dmd.selectFont(SystemFont5x7);
  int w = dmd.stringWidth(text);
  int x = (32 - w) / 2;
  if (x < 0) x = 0;
  dmd.drawString(x, y, text);
}

char prevTime[8]  = "";
char prevDate[12] = "";

void setup() {
  Serial.begin(9600);

  if (EEPROM.read(EEPROM_FLAG_ADDR) != EEPROM_FLAG_VAL) {
    char dateStr[] = __DATE__;
    char timeStr[] = __TIME__;
    int ay     = parseMonth(dateStr);
    int gun    = atoi(dateStr + 4);
    int yil    = atoi(dateStr + 7);
    int saat   = atoi(timeStr);
    int dakika = atoi(timeStr + 3);
    int saniye = atoi(timeStr + 6);
    int gun_no = calcDayOfWeek(yil, ay, gun);
    RTC.setDS1302Time(saniye, dakika, saat, gun_no, gun, ay, yil);
    EEPROM.write(EEPROM_FLAG_ADDR, EEPROM_FLAG_VAL);
    Serial.println("RTC ilk kez ayarlandı.");
  } else {
    Serial.println("RTC zaten ayarlı.");
  }

  dmd.begin();
  dmd.setBrightness(150);
  dmd.clearScreen();

  // ── Başlangıç testi: 2 sn her iki panel görünsün ────
  dmd.selectFont(SystemFont5x7);
  dmd.drawString(2, 2,  "SAAT");  // alt panel
  dmd.drawString(2, 18, "TRH");   // üst panel
  delay(2000);
  dmd.clearScreen();
}

void loop() {
  RTC.updateTime();

  // ── Saat metni ───────────────────────────────────────
  char h[3], m[3];
  twoDigit(h, RTC.hours);
  twoDigit(m, RTC.minutes);
  char sep = (RTC.seconds % 2 == 0) ? ':' : ' ';
  char timeText[8];
  snprintf(timeText, sizeof(timeText), "%s%c%s", h, sep, m);

  // ── Tarih metni ──────────────────────────────────────
  char d[3];
  twoDigit(d, RTC.dayofmonth);
  const char* ayStr = ayIsmi(RTC.month);
  char dateText[12];
  snprintf(dateText, sizeof(dateText), "%s-%s", d, ayStr);

  // ── Saat değiştiyse üst paneli güncelle (y:16-31) ────
  if (strcmp(timeText, prevTime) != 0) {
    clearRegion(16, 31);
    drawCentered(timeText, 18);
    strcpy(prevTime, timeText);
    Serial.print("Saat: "); Serial.println(timeText);
  }

  // ── Tarih değiştiyse alt paneli güncelle (y:0-15) ────
  if (strcmp(dateText, prevDate) != 0) {
    clearRegion(0, 15);
    drawCentered(d,     1);   // üst satır: "12"
    drawCentered(ayStr, 9);   // alt satır: "Mar"
    strcpy(prevDate, dateText);
    Serial.print("Tarih: "); Serial.print(d);
    Serial.print(" "); Serial.println(ayStr);
  }

  delay(500);
}
