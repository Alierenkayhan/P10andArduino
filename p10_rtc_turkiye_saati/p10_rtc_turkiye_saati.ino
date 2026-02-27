#include <virtuabotixRTC.h>
#include <DMD2.h>
#include <fonts/Arial_Black_16.h>
#include <fonts/SystemFont5x7.h>

// ── DS1302 Pinleri ──────────────────────────────────────
int CLK = 5;
int DAT = 4;
int RST = 3;
virtuabotixRTC RTC(CLK, DAT, RST);

// ── P10 Panel: 2 panel yan yana, 1 panel yüksek ─────────
SoftDMD dmd(2, 1);  // 64x16 px toplam

// ── Ay parse ─────────────────────────────────────────────
int parseMonth(const char* s) {
  const char* months[] = {"Jan","Feb","Mar","Apr","May","Jun",
                           "Jul","Aug","Sep","Oct","Nov","Dec"};
  for (int i = 0; i < 12; i++)
    if (strncmp(s, months[i], 3) == 0) return i + 1;
  return 1;
}

// ── Zeller: haftanın günü ─────────────────────────────────
int calcDayOfWeek(int y, int m, int d) {
  if (m < 3) { m += 12; y--; }
  int k = y % 100, j = y / 100;
  int h = (d + (13*(m+1))/5 + k + k/4 + j/4 + 5*j) % 7;
  return ((h + 6) % 7) + 1;
}

// ── Sıfır dolgulu yazdırma (örn: "09") ───────────────────
void twoDigit(char* buf, int val) {
  buf[0] = '0' + val / 10;
  buf[1] = '0' + val % 10;
  buf[2] = '\0';
}

// ── Görüntüleme modu ─────────────────────────────────────
enum Mode { SHOW_TIME, SHOW_DATE };
Mode currentMode = SHOW_TIME;
unsigned long lastSwitch = 0;
const unsigned long TIME_DURATION = 5000;  // 5 sn saat göster
const unsigned long DATE_DURATION = 3000;  // 3 sn tarih göster

void setup() {
  Serial.begin(9600);

  char dateStr[] = __DATE__;
  char timeStr[] = __TIME__;
  int ay     = parseMonth(dateStr);
  int gun    = atoi(dateStr + 4);
  int yil    = atoi(dateStr + 7);
  int saat   = atoi(timeStr);
  int dakika = atoi(timeStr + 3);
  int saniye = atoi(timeStr + 6);
  int gun_no = calcDayOfWeek(yil, ay, gun);

  Serial.print("Zaman ayarlandı: ");
  Serial.print(gun); Serial.print("/"); Serial.print(ay);
  Serial.print("/"); Serial.print(yil); Serial.print("  ");
  Serial.print(saat); Serial.print(":"); Serial.print(dakika);
  Serial.print(":"); Serial.println(saniye);

  // ⚠️ İlk yüklemeden sonra bu satırı yorum yapın!
  RTC.setDS1302Time(saniye, dakika, saat, gun_no, gun, ay, yil);

  dmd.begin();
  dmd.setBrightness(150);
}

void loop() {
  RTC.updateTime();
  unsigned long now = millis();

  // Modu zamanla değiştir
  unsigned long duration = (currentMode == SHOW_TIME) ? TIME_DURATION : DATE_DURATION;
  if (now - lastSwitch >= duration) {
    currentMode = (currentMode == SHOW_TIME) ? SHOW_DATE : SHOW_TIME;
    lastSwitch = now;
    dmd.clearScreen();
  }

  dmd.clearScreen();

  if (currentMode == SHOW_TIME) {
    // ── Saat gösterimi: "HH:MM" büyük font, ortalı ───────
    char h[3], m[3];
    twoDigit(h, RTC.hours);
    twoDigit(m, RTC.minutes);

    char timeText[6];
    if (RTC.seconds % 2 == 0)
      sprintf(timeText, "%s:%s", h, m);
    else
      sprintf(timeText, "%s %s", h, m);

    dmd.selectFont(Arial_Black_16);
    int x = (64 - dmd.stringWidth(timeText)) / 2;
    if (x < 0) x = 0;
    dmd.drawString(x, 0, timeText);

  } else {
    // ── Tarih gösterimi: "DD.MM.YYYY" küçük font, ortalı ─
    char dateText[12];
    char d[3], mo[3];
    twoDigit(d, RTC.dayofmonth);
    twoDigit(mo, RTC.month);
    sprintf(dateText, "%s.%s.%d", d, mo, RTC.year);

    dmd.selectFont(SystemFont5x7);
    int x = (64 - dmd.stringWidth(dateText)) / 2;
    if (x < 0) x = 0;
    dmd.drawString(x, 4, dateText);
  }

  delay(500);
}