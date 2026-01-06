/*--------------------------------------------------------------------------------------
  Includes
--------------------------------------------------------------------------------------*/
#include <SPI.h>
#include <DMD.h>
#include <TimerOne.h>
#include "SystemFont5x7.h"
#include "Arial_black_16.h"

/*======================================================================================
  AYARLAR - BURADAN DEĞİŞTİRİN
======================================================================================*/

// Panel Ayarları
#define DISPLAYS_ACROSS 2         // Yatayda kaç panel var
#define DISPLAYS_DOWN 1           // Dikeyde kaç panel var

// Font Seçimi (birini seçin, diğerini yorum yapın)
#define KULLANILAN_FONT System5x7       // Küçük font (7 piksel)
//#define KULLANILAN_FONT Arial_black_16  // Büyük font (16 piksel)

// Font Yüksekliği (seçtiğiniz fonta göre ayarlayın)
#define FONT_YUKSEKLIGI 7         // System5x7 için 7, Arial_black_16 için 16

// Karakter Genişliği (font başına)
#define KARAKTER_GENISLIGI 6      // System5x7 için 6, Arial_black_16 için ~12-14

// Gösterilecek Metinler
const char* METIN_1 = "VEX V5";
const char* METIN_2 = "VEX IQ";
const char* METIN_KAYAN = "VEX V5 | VEX IQ";

// Zamanlama Ayarları (milisaniye)
#define SABIT_YAZI_SURESI 1500    // Sabit yazıların ekranda kalma süresi
#define KAYAN_YAZI_HIZI 30        // Kayan yazı hızı (düşük = hızlı)
#define KAYAN_YAZI_BEKLEME 500    // Kayan yazı sonrası bekleme

// Dikey Pozisyon Ayarı (manuel ayar için)
#define Y_OFFSET 0                // Yazıyı yukarı/aşağı kaydırmak için (-2, -1, 0, 1, 2)

/*======================================================================================
  AYARLAR SONU
======================================================================================*/

DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);

// Sabitler
const int PANEL_GENISLIK = 32 * DISPLAYS_ACROSS;
const int PANEL_YUKSEKLIK = 16 * DISPLAYS_DOWN;

/*--------------------------------------------------------------------------------------
  Interrupt handler
--------------------------------------------------------------------------------------*/
void ScanDMD() {
  dmd.scanDisplayBySPI();
}

/*--------------------------------------------------------------------------------------
  setup
--------------------------------------------------------------------------------------*/
void setup(void) {
  Timer1.initialize(5000);
  Timer1.attachInterrupt(ScanDMD);
  dmd.clearScreen(true);
}

/*--------------------------------------------------------------------------------------
  Yardımcı fonksiyon: Metin genişliğini hesapla
--------------------------------------------------------------------------------------*/
int getTextWidth(const char* text) {
  int len = strlen(text);
  if (len == 0) return 0;
  return (len * KARAKTER_GENISLIGI) - 1;
}

/*--------------------------------------------------------------------------------------
  Yardımcı fonksiyon: Metni ortala ve göster
--------------------------------------------------------------------------------------*/
void showCenteredText(const char* text) {
  dmd.clearScreen(true);
  dmd.selectFont(KULLANILAN_FONT);
  
  int textWidth = getTextWidth(text);
  int x = (PANEL_GENISLIK - textWidth) / 2;
  int y = ((PANEL_YUKSEKLIK - FONT_YUKSEKLIGI) / 2) + Y_OFFSET;
  
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  
  dmd.drawString(x, y, text, strlen(text), GRAPHICS_NORMAL);
}

/*--------------------------------------------------------------------------------------
  Yardımcı fonksiyon: Kayan yazı göster
--------------------------------------------------------------------------------------*/
void showScrollingText(const char* text) {
  dmd.clearScreen(true);
  dmd.selectFont(KULLANILAN_FONT);
  
  int y = ((PANEL_YUKSEKLIK - FONT_YUKSEKLIGI) / 2) + Y_OFFSET;
  if (y < 0) y = 0;
  
  dmd.drawMarquee((char*)text, strlen(text), PANEL_GENISLIK - 1, y);
  
  long timer = millis();
  bool done = false;
  while (!done) {
    if (millis() - timer > KAYAN_YAZI_HIZI) {
      done = dmd.stepMarquee(-1, 0);
      timer = millis();
    }
  }
}

/*--------------------------------------------------------------------------------------
  loop
--------------------------------------------------------------------------------------*/
void loop(void) {
  // 1) İlk metin
  showCenteredText(METIN_1);
  delay(SABIT_YAZI_SURESI);

  // 2) İkinci metin
  showCenteredText(METIN_2);
  delay(SABIT_YAZI_SURESI);

  // 3) Kayan yazı
  showScrollingText(METIN_KAYAN);
  delay(KAYAN_YAZI_BEKLEME);
}