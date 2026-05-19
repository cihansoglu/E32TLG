#include <Wire.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Preferences.h>

#define BOT_TOKEN "BOT_TOKEN_HERE"

// ===== KİŞİ CHAT ID'LERİ =====
#define CHAT_ID_ANNEM "CHAT_ID1_HERE"
#define CHAT_ID_BABAM "CHAT_ID2_HERE"
#define CHAT_ID_ABLAM "CHAT_ID3_HERE"
#define CHAT_ID_GRUP "-GRUP_CHAT_ID_HERE"
// ==============================

#define ENC_A     18
#define ENC_B     19
#define ENC_BTN   5
#define BTN_BACK  2
#define BTN_CONFIRM 4
#define BUZZER    23

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
Preferences preferences;

const char* kisiler[] = {"Annem", "Babam", "Ablam", "Aile Grubu"};
const char* chatIDler[] = {CHAT_ID_ANNEM, CHAT_ID_BABAM, CHAT_ID_ABLAM, CHAT_ID_GRUP};
int kisiSayisi = 4;
int seciliKisi = 0;
String aktifChatID = "";
bool kisiSecildi = false;

String kayitliSSIDler[3] = {"", "", ""};
String kayitliSifreler[3] = {"", "", ""};
int kayitliAgSayisi = 0;

const char* harfler[] = {
  "a","b","c","d","e","f","g","h","i","j",
  "k","l","m","n","o","p","r","s","t","u",
  "v","y","z"," ",
  "0","1","2","3","4","5","6","7","8","9",
  ".","," ,"!","?","-","+"
};
int harfSayisi = 40;

const char* sifreHarfleri[] = {
  "a","b","c","d","e","f","g","h","i","j",
  "k","l","m","n","o","p","q","r","s","t",
  "u","v","w","x","y","z",
  "A","B","C","D","E","F","G","H","I","J",
  "K","L","M","N","O","P","Q","R","S","T",
  "U","V","W","X","Y","Z",
  "0","1","2","3","4","5","6","7","8","9",
  ".","," ,"!","?","-","+","@","_","#","*"
};
int sifreHarfSayisi = 73;

const char* hazirMesajlar[] = {
  "Evet.",
  "Tamam.",
  "Hayir.",
  "Ciktim, geliyorum.",
  "Nerdesin?",
  "Bekliyorum.",
  "Bekle, geliyorum."
};
int hazirMesajSayisi = 7;
int hazirMesajIndex = 0;

int wifiMod = 0;
int agSayisi = 0;
int seciliAg = 0;
String seciliSSID = "";
String wifiSifre = "";
bool wifiAyarTamam = false;

int mod = 0;
int harfIndex = 0;
int sifreHarfIndex = 0;
String mesaj = "";

volatile int enkoderDegisim = 0;
volatile int encA_son = HIGH;

void IRAM_ATTR enkoderISR() {
  int a = digitalRead(ENC_A);
  int b = digitalRead(ENC_B);
  if(encA_son == HIGH && a == LOW) {
    if(b == HIGH) enkoderDegisim++;
    else enkoderDegisim--;
  }
  encA_son = a;
}

unsigned long sonTelegramKontrol = 0;
#define TELEGRAM_ARALIK 3000

String mesajGecmisi[4][4] = {
  {"","","",""},
  {"","","",""},
  {"","","",""},
  {"","","",""}
};
String mesajTipleri[4][4] = {
  {"","","",""},
  {"","","",""},
  {"","","",""},
  {"","","",""}
};

bool telegramBagli = false;
unsigned long sonBasariliIstek = 0;

unsigned long sonConfirmZamani = 0;
#define CIFT_TIK_ARALIK 400
#define UZUN_BAS_SURE 2000

int kaymaOffset0 = 0;
int kaymaOffset1 = 0;
unsigned long sonKaymaZamani = 0;
#define KAYMA_ARALIK 300
#define EKRAN_KARAKTER 18

int agKaymaOffset = 0;
unsigned long sonAgKaymaZamani = 0;

bool bildirimVar = false;
unsigned long bildirimZamani = 0;
#define BILDIRIM_SURE 2000

// ===== WiFi HAFIZA =====

void kayitliAglariYukle() {
  kayitliAgSayisi = preferences.getInt("agSayisi", 0);
  for(int i = 0; i < kayitliAgSayisi; i++) {
    kayitliSSIDler[i] = preferences.getString(("ssid" + String(i)).c_str(), "");
    kayitliSifreler[i] = preferences.getString(("sifre" + String(i)).c_str(), "");
  }
}

void wifiKaydet(String ssid, String sifre) {
  for(int i = 0; i < kayitliAgSayisi; i++) {
    if(kayitliSSIDler[i] == ssid) {
      kayitliSifreler[i] = sifre;
      preferences.putString(("sifre" + String(i)).c_str(), sifre);
      return;
    }
  }
  if(kayitliAgSayisi < 3) {
    kayitliSSIDler[kayitliAgSayisi] = ssid;
    kayitliSifreler[kayitliAgSayisi] = sifre;
    kayitliAgSayisi++;
  } else {
    for(int i = 0; i < 2; i++) {
      kayitliSSIDler[i] = kayitliSSIDler[i+1];
      kayitliSifreler[i] = kayitliSifreler[i+1];
    }
    kayitliSSIDler[2] = ssid;
    kayitliSifreler[2] = sifre;
  }
  preferences.putInt("agSayisi", kayitliAgSayisi);
  for(int i = 0; i < kayitliAgSayisi; i++) {
    preferences.putString(("ssid" + String(i)).c_str(), kayitliSSIDler[i]);
    preferences.putString(("sifre" + String(i)).c_str(), kayitliSifreler[i]);
  }
}

String kayitliSifreBul(String ssid) {
  for(int i = 0; i < kayitliAgSayisi; i++) {
    if(kayitliSSIDler[i] == ssid) return kayitliSifreler[i];
  }
  return "";
}

// ===== SES =====

void gonderilenSes() {
  tone(BUZZER, 1000, 100);
  delay(150);
  tone(BUZZER, 1500, 100);
}

void gelenSes() {
  tone(BUZZER, 800, 100);
  delay(150);
  tone(BUZZER, 800, 100);
  delay(150);
  tone(BUZZER, 1200, 150);
}

// ===== YARDIMCI =====

// chat_id ve from_id ile kişi indeksini bul
int chatIDdenKisiIndex(String chatID, String fromID) {
  // Direkt eşleşme — özel mesaj
  for(int i = 0; i < kisiSayisi; i++) {
    if(chatID == chatIDler[i]) return i;
  }
  // Grup mesajı — chat_id grup ID'si, from_id gönderen kişi
  if(chatID == CHAT_ID_GRUP) return 3; // Aile Grubu index
  // from_id ile eşleşen kişi grup üyesi mi?
  for(int i = 0; i < kisiSayisi - 1; i++) {
    if(fromID == chatIDler[i]) return 3; // Aile Grubu index
  }
  return -1;
}

bool aktifKisidanMi(int kisiIndex) {
  return kisiIndex == seciliKisi;
}

String turkceLatine(String m) {
  String sonuc = "";
  int i = 0;
  while(i < (int)m.length()) {
    unsigned char c = m[i];
    if(c == 0xC4) {
      unsigned char c2 = m[i+1];
      if(c2 == 0x9F || c2 == 0x9E) sonuc += "g";
      else if(c2 == 0xB1) sonuc += "i";
      else if(c2 == 0xB0) sonuc += "I";
      else sonuc += "?";
      i += 2;
    } else if(c == 0xC5) {
      unsigned char c2 = m[i+1];
      if(c2 == 0x9F || c2 == 0x9E) sonuc += "s";
      else sonuc += "?";
      i += 2;
    } else if(c == 0xC3) {
      unsigned char c2 = m[i+1];
      if(c2 == 0xB6 || c2 == 0x96) sonuc += "o";
      else if(c2 == 0xBC || c2 == 0x9C) sonuc += "u";
      else if(c2 == 0xA7 || c2 == 0x87) sonuc += "c";
      else sonuc += "?";
      i += 2;
    } else {
      sonuc += (char)c;
      i++;
    }
  }
  return sonuc;
}

void mesajEkle(String mesajMetni, String tip, int kisiIndex) {
  if(kisiIndex < 0 || kisiIndex >= kisiSayisi) return;
  if(tip == "o") mesajMetni = turkceLatine(mesajMetni);
  for(int i = 3; i > 0; i--) {
    mesajGecmisi[kisiIndex][i] = mesajGecmisi[kisiIndex][i-1];
    mesajTipleri[kisiIndex][i] = mesajTipleri[kisiIndex][i-1];
  }
  mesajGecmisi[kisiIndex][0] = mesajMetni;
  mesajTipleri[kisiIndex][0] = tip;
  if(kisiIndex == seciliKisi) {
    kaymaOffset0 = 0;
    kaymaOffset1 = 0;
  }
}

String kaydirilmisMesaj(String m, int offset) {
  String tam = m + "   ";
  if((int)tam.length() <= EKRAN_KARAKTER) return tam;
  if(offset >= (int)tam.length()) offset = 0;
  String parcali = tam.substring(offset);
  if((int)parcali.length() < EKRAN_KARAKTER)
    parcali += tam.substring(0, EKRAN_KARAKTER - parcali.length());
  return parcali.substring(0, EKRAN_KARAKTER);
}

String kaydirilmisAg(String m, int offset) {
  String tam = m + "   ";
  if((int)tam.length() <= 18) return tam;
  if(offset >= (int)tam.length()) offset = 0;
  String parcali = tam.substring(offset);
  if((int)parcali.length() < 18)
    parcali += tam.substring(0, 18 - parcali.length());
  return parcali.substring(0, 18);
}

// ===== İKONLAR =====

void wifiIkonu(int x, bool bagli) {
  int yukseklikler[] = {3, 5, 7, 9};
  int rssi = WiFi.RSSI();
  int doluCubuk = 0;
  if(bagli) {
    if(rssi >= -55) doluCubuk = 4;
    else if(rssi >= -65) doluCubuk = 3;
    else if(rssi >= -75) doluCubuk = 2;
    else doluCubuk = 1;
  }
  for(int i = 0; i < 4; i++) {
    int cx = x + i * 4;
    int cy = 11;
    int h = yukseklikler[i];
    if(i < doluCubuk) u8g2.drawBox(cx, cy - h, 3, h);
    else u8g2.drawFrame(cx, cy - h, 3, h);
  }
  if(!bagli) u8g2.drawStr(x + 18, 10, "!");
}

void telegramIkonu(int x, bool bagli) {
  if(bagli) {
    u8g2.drawLine(x, 6, x+3, 10);
    u8g2.drawLine(x+1, 6, x+4, 10);
    u8g2.drawLine(x+3, 10, x+9, 2);
    u8g2.drawLine(x+4, 10, x+10, 2);
  } else {
    u8g2.drawLine(x, 2, x+9, 11);
    u8g2.drawLine(x+1, 2, x+10, 11);
    u8g2.drawLine(x+9, 2, x, 11);
    u8g2.drawLine(x+10, 2, x+1, 11);
  }
}

void mektupIkonu(int x) {
  u8g2.drawFrame(x, 3, 12, 8);
  u8g2.drawLine(x, 3, x+6, 8);
  u8g2.drawLine(x+12, 3, x+6, 8);
}

// ===== EKRANLAR =====

void kisiSecimEkrani() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, "E32TLG");
  u8g2.drawLine(0, 13, 128, 13);
  u8g2.drawStr(0, 25, "Kime yazacaksin?");
  u8g2.drawLine(0, 27, 128, 27);
  if(seciliKisi > 0)
    u8g2.drawStr(0, 38, kisiler[seciliKisi - 1]);
  u8g2.drawBox(0, 40, 128, 12);
  u8g2.setDrawColor(0);
  String seciliIsim = String(kisiler[seciliKisi]);
  if(seciliKisi == 3) seciliIsim = "* " + seciliIsim;
  u8g2.drawStr(2, 50, seciliIsim.c_str());
  u8g2.setDrawColor(1);
  if(seciliKisi < kisiSayisi - 1)
    u8g2.drawStr(0, 62, kisiler[seciliKisi + 1]);
  u8g2.sendBuffer();
}

void agTaraEkrani() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, "E32TLG");
  u8g2.drawLine(0, 13, 128, 13);
  u8g2.drawStr(0, 35, "WiFi taranıyor...");
  u8g2.sendBuffer();
}

void agSecimEkrani() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, "WiFi Sec:");
  u8g2.drawLine(0, 13, 128, 13);
  if(seciliAg > 0) {
    String onceki = WiFi.SSID(seciliAg-1).substring(0,18);
    if(kayitliSifreBul(WiFi.SSID(seciliAg-1)) != "") onceki += "*";
    u8g2.drawStr(0, 25, onceki.c_str());
  }
  u8g2.drawBox(0, 27, 128, 12);
  u8g2.setDrawColor(0);
  String seciliAdi = kaydirilmisAg(WiFi.SSID(seciliAg), agKaymaOffset);
  if(kayitliSifreBul(WiFi.SSID(seciliAg)) != "") seciliAdi += "*";
  u8g2.drawStr(2, 37, seciliAdi.c_str());
  u8g2.setDrawColor(1);
  if(seciliAg < agSayisi-1) {
    String sonraki = WiFi.SSID(seciliAg+1).substring(0,18);
    if(kayitliSifreBul(WiFi.SSID(seciliAg+1)) != "") sonraki += "*";
    u8g2.drawStr(0, 52, sonraki.c_str());
  }
  if(kayitliSifreBul(WiFi.SSID(seciliAg)) != "") {
    u8g2.drawStr(0, 63, "PSH:baglan *=kayitli");
  } else {
    u8g2.drawStr(0, 63, "PSH:sec BACK:tara");
  }
  u8g2.sendBuffer();
}

void sifreGirEkrani() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, seciliSSID.substring(0,18).c_str());
  u8g2.drawLine(0, 13, 128, 13);
  u8g2.drawStr(0, 25, "Sifre:");
  String sg = wifiSifre + String(sifreHarfleri[sifreHarfIndex]);
  if(sg.length() > 18) sg = sg.substring(sg.length()-18);
  u8g2.drawStr(0, 40, sg.c_str());
  u8g2.drawLine(0, 50, 128, 50);
  u8g2.drawStr(0, 62, "PSH:ekle CON:baglan");
  u8g2.sendBuffer();
}

void baglaniyorEkrani(String ssid) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, "E32TLG");
  u8g2.drawLine(0, 13, 128, 13);
  u8g2.drawStr(0, 30, "Baglanıyor...");
  u8g2.drawStr(0, 45, ssid.substring(0,20).c_str());
  u8g2.sendBuffer();
}

void ekraniGuncelle() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  String baslik = ">" + String(kisiler[seciliKisi]);
  u8g2.drawStr(0, 10, baslik.c_str());

  if(bildirimVar && millis()-bildirimZamani < BILDIRIM_SURE) {
    if((millis()-bildirimZamani) % 400 < 200) {
      mektupIkonu(114);
    }
  } else {
    bildirimVar = false;
    wifiIkonu(90, WiFi.status() == WL_CONNECTED);
    if(WiFi.status() == WL_CONNECTED) telegramIkonu(114, telegramBagli);
  }

  u8g2.drawLine(0, 13, 128, 13);

  if(mod == 0) {
    if(mesajGecmisi[seciliKisi][1] != "") {
      String s1 = (mesajTipleri[seciliKisi][1]=="ben") ? ">>" : "<<";
      u8g2.drawStr(0, 27, (s1+kaydirilmisMesaj(mesajGecmisi[seciliKisi][1],kaymaOffset1)).c_str());
    }
    if(mesajGecmisi[seciliKisi][0] != "") {
      String s0 = (mesajTipleri[seciliKisi][0]=="ben") ? ">>" : "<<";
      u8g2.drawStr(0, 40, (s0+kaydirilmisMesaj(mesajGecmisi[seciliKisi][0],kaymaOffset0)).c_str());
    }
    u8g2.drawLine(0, 50, 128, 50);
    u8g2.drawStr(0, 62, (">"+mesaj+String(harfler[harfIndex])).c_str());
  } else {
    u8g2.drawStr(0, 25, "Hazir mesaj:");
    u8g2.drawLine(0, 27, 128, 27);
    if(hazirMesajIndex > 0)
      u8g2.drawStr(0, 38, String(hazirMesajlar[hazirMesajIndex-1]).substring(0,22).c_str());
    u8g2.drawBox(0, 40, 128, 12);
    u8g2.setDrawColor(0);
    u8g2.drawStr(2, 50, String(hazirMesajlar[hazirMesajIndex]).substring(0,22).c_str());
    u8g2.setDrawColor(1);
    if(hazirMesajIndex < hazirMesajSayisi-1)
      u8g2.drawStr(0, 62, String(hazirMesajlar[hazirMesajIndex+1]).substring(0,22).c_str());
  }
  u8g2.sendBuffer();
}

void mesajGonder(String m) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 20, ("-> " + String(kisiler[seciliKisi])).c_str());
  u8g2.drawStr(0, 40, "Gonderiliyor...");
  u8g2.sendBuffer();
  if(bot.sendMessage(aktifChatID, m, "")) {
    mesajEkle(m, "ben", seciliKisi);
    telegramBagli = true;
    sonBasariliIstek = millis();
    gonderilenSes();
  }
}

bool wifiBaglan(String ssid, String sifre) {
  baglaniyorEkrani(ssid);
  WiFi.begin(ssid.c_str(), sifre.c_str());
  int d = 0;
  while(WiFi.status() != WL_CONNECTED && d < 20) { delay(500); d++; }
  return WiFi.status() == WL_CONNECTED;
}

void wifiSecimineGit() {
  wifiAyarTamam = false;
  kisiSecildi = false;
  WiFi.disconnect();
  agTaraEkrani();
  agSayisi = WiFi.scanNetworks();
  seciliAg = 0;
  agKaymaOffset = 0;
  wifiMod = 0;
  enkoderDegisim = 0;
  agSecimEkrani();
}

// ===== SETUP =====

void setup() {
  Wire.begin(21, 22);
  u8g2.begin();
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  pinMode(ENC_BTN, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);
  pinMode(BTN_CONFIRM, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(ENC_A), enkoderISR, CHANGE);

  preferences.begin("e32tlg", false);
  kayitliAglariYukle();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, "E32TLG");
  u8g2.drawLine(0, 13, 128, 13);

  if(kayitliAgSayisi > 0) {
    String kSSID = kayitliSSIDler[kayitliAgSayisi-1];
    String kSifre = kayitliSifreler[kayitliAgSayisi-1];
    u8g2.drawStr(0, 30, "Kayitli WiFi:");
    u8g2.drawStr(0, 45, kSSID.substring(0,20).c_str());
    u8g2.sendBuffer();
    delay(1000);
    if(wifiBaglan(kSSID, kSifre)) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawStr(0, 10, "E32TLG");
      u8g2.drawLine(0, 13, 128, 13);
      u8g2.drawStr(0, 35, "WiFi TAMAM!");
      u8g2.sendBuffer();
      delay(1000);
      client.setInsecure();
      telegramBagli = true;
      sonBasariliIstek = millis();
      wifiAyarTamam = true;
      seciliKisi = 0;
      kisiSecimEkrani();
      return;
    } else {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawStr(0, 30, "Baglanamadı!");
      u8g2.drawStr(0, 45, "Ag taranıyor...");
      u8g2.sendBuffer();
      delay(1500);
    }
  }

  agTaraEkrani();
  agSayisi = WiFi.scanNetworks();
  seciliAg = 0;
  wifiMod = 0;
  agSecimEkrani();
}

// ===== LOOP =====

void loop() {

  // ===== WiFi SEÇİM MODU =====
  if(!wifiAyarTamam) {
    if(millis()-sonAgKaymaZamani > 300) {
      sonAgKaymaZamani = millis();
      if(wifiMod==0 && (int)WiFi.SSID(seciliAg).length()>18) {
        agKaymaOffset++;
        if(agKaymaOffset >= (int)(WiFi.SSID(seciliAg).length()+3)) agKaymaOffset=0;
        agSecimEkrani();
      }
    }

    if(enkoderDegisim != 0) {
      if(wifiMod == 0) {
        if(enkoderDegisim > 0) seciliAg = (seciliAg+1) % agSayisi;
        else seciliAg = (seciliAg-1+agSayisi) % agSayisi;
        agKaymaOffset = 0;
        enkoderDegisim = 0;
        agSecimEkrani();
      } else {
        if(enkoderDegisim > 0) {
          sifreHarfIndex = (sifreHarfIndex+1) % sifreHarfSayisi;
        } else {
          sifreHarfIndex--;
          if(sifreHarfIndex < 0) sifreHarfIndex = sifreHarfSayisi-1;
        }
        enkoderDegisim = 0;
        sifreGirEkrani();
      }
    }

    if(digitalRead(ENC_BTN)==LOW) {
      delay(200);
      if(wifiMod==0) {
        seciliSSID = WiFi.SSID(seciliAg);
        String kayitliSifre = kayitliSifreBul(seciliSSID);
        if(kayitliSifre != "") {
          if(wifiBaglan(seciliSSID, kayitliSifre)) {
            u8g2.clearBuffer();
            u8g2.setFont(u8g2_font_ncenB08_tr);
            u8g2.drawStr(0, 10, "E32TLG");
            u8g2.drawLine(0, 13, 128, 13);
            u8g2.drawStr(0, 35, "WiFi TAMAM!");
            u8g2.sendBuffer();
            delay(1000);
            client.setInsecure();
            telegramBagli=true;
            sonBasariliIstek=millis();
            wifiAyarTamam=true;
            seciliKisi=0;
            kisiSecimEkrani();
          } else {
            wifiSifre=""; sifreHarfIndex=0;
            wifiMod=1; sifreGirEkrani();
          }
        } else {
          wifiSifre=""; sifreHarfIndex=0;
          wifiMod=1; sifreGirEkrani();
        }
      } else {
        wifiSifre += String(sifreHarfleri[sifreHarfIndex]);
        sifreGirEkrani();
      }
    }

    if(digitalRead(BTN_BACK)==LOW) {
      delay(200);
      if(wifiMod==1) {
        if(wifiSifre.length()>0) {
          wifiSifre = wifiSifre.substring(0, wifiSifre.length()-1);
          sifreGirEkrani();
        } else {
          wifiMod=0; agKaymaOffset=0; agSecimEkrani();
        }
      } else {
        agTaraEkrani();
        agSayisi=WiFi.scanNetworks();
        seciliAg=0; agKaymaOffset=0; agSecimEkrani();
      }
    }

    if(digitalRead(BTN_CONFIRM)==LOW) {
      delay(200);
      if(wifiMod==1) {
        wifiSifre += String(sifreHarfleri[sifreHarfIndex]);
        if(wifiBaglan(seciliSSID, wifiSifre)) {
          wifiKaydet(seciliSSID, wifiSifre);
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_ncenB08_tr);
          u8g2.drawStr(0, 10, "E32TLG");
          u8g2.drawLine(0, 13, 128, 13);
          u8g2.drawStr(0, 35, "WiFi TAMAM!");
          u8g2.sendBuffer();
          delay(1000);
          client.setInsecure();
          telegramBagli=true;
          sonBasariliIstek=millis();
          wifiAyarTamam=true;
          seciliKisi=0;
          kisiSecimEkrani();
        } else {
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_ncenB08_tr);
          u8g2.drawStr(0, 30, "Baglanamadı!");
          u8g2.drawStr(0, 45, "Sifre yanlis?");
          u8g2.sendBuffer();
          delay(2000);
          wifiSifre = "";
          sifreHarfIndex = 0;
          sifreGirEkrani();
        }
      }
    }
    return;
  }

  // ===== KİŞİ SEÇİM MODU =====
  if(!kisiSecildi) {
    if(enkoderDegisim != 0) {
      if(enkoderDegisim>0) seciliKisi=(seciliKisi+1)%kisiSayisi;
      else seciliKisi=(seciliKisi-1+kisiSayisi)%kisiSayisi;
      enkoderDegisim=0;
      kisiSecimEkrani();
    }
    if(digitalRead(ENC_BTN)==LOW || digitalRead(BTN_CONFIRM)==LOW) {
      delay(200);
      aktifChatID = chatIDler[seciliKisi];
      kisiSecildi = true;
      kaymaOffset0 = 0;
      kaymaOffset1 = 0;
      enkoderDegisim = 0;
      ekraniGuncelle();
    }
    return;
  }

  // ===== NORMAL ÇALIŞMA MODU =====

  if(enkoderDegisim != 0) {
    if(mod==0) {
      if(enkoderDegisim>0) harfIndex=(harfIndex+1)%harfSayisi;
      else harfIndex=(harfIndex-1+harfSayisi)%harfSayisi;
    } else {
      if(enkoderDegisim>0) hazirMesajIndex=(hazirMesajIndex+1)%hazirMesajSayisi;
      else hazirMesajIndex=(hazirMesajIndex-1+hazirMesajSayisi)%hazirMesajSayisi;
    }
    enkoderDegisim=0;
    ekraniGuncelle();
  }

  if(bildirimVar && millis()-bildirimZamani >= BILDIRIM_SURE) {
    bildirimVar = false;
    ekraniGuncelle();
  }

  if(bildirimVar && millis()-bildirimZamani < BILDIRIM_SURE) {
    ekraniGuncelle();
    delay(50);
  }

  if(millis()-sonKaymaZamani > KAYMA_ARALIK) {
    sonKaymaZamani=millis(); bool g=false;
    if(mesajGecmisi[seciliKisi][0].length()>EKRAN_KARAKTER) {
      kaymaOffset0++;
      if(kaymaOffset0>=(int)(mesajGecmisi[seciliKisi][0].length()+3)) kaymaOffset0=0; g=true;
    }
    if(mesajGecmisi[seciliKisi][1].length()>EKRAN_KARAKTER) {
      kaymaOffset1++;
      if(kaymaOffset1>=(int)(mesajGecmisi[seciliKisi][1].length()+3)) kaymaOffset1=0; g=true;
    }
    if(g) ekraniGuncelle();
  }

  if(digitalRead(ENC_BTN)==LOW) {
    if(mod==0) { mesaj+=String(harfler[harfIndex]); ekraniGuncelle(); }
    delay(300);
  }

  if(digitalRead(BTN_BACK)==LOW) {
    unsigned long bz=millis();
    while(digitalRead(BTN_BACK)==LOW) {
      if(millis()-bz > 3000) {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(0, 30, "WiFi degistiriliyor...");
        u8g2.sendBuffer();
        delay(1000);
        wifiSecimineGit();
        return;
      }
    }
    if(millis()-bz < 3000) {
      if(mod==1) { mod=0; ekraniGuncelle(); }
      else if(mesaj.length()>0) {
        mesaj=mesaj.substring(0,mesaj.length()-1); ekraniGuncelle();
      }
    }
    delay(300);
  }

  if(digitalRead(BTN_CONFIRM)==LOW) {
    unsigned long cz = millis();
    while(digitalRead(BTN_CONFIRM)==LOW) {
      if(millis()-cz > UZUN_BAS_SURE) {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(0, 30, "Kisi secimi...");
        u8g2.sendBuffer();
        delay(500);
        kisiSecildi = false;
        mesaj = "";
        enkoderDegisim = 0;
        kisiSecimEkrani();
        return;
      }
    }
    unsigned long simdi = millis();
    if(millis()-cz < UZUN_BAS_SURE) {
      if(mod==1) {
        mesajGonder(String(hazirMesajlar[hazirMesajIndex]));
        mod=0; hazirMesajIndex=0; delay(500); ekraniGuncelle();
      } else {
        if(sonConfirmZamani != 0 && (simdi-sonConfirmZamani<CIFT_TIK_ARALIK)) {
          sonConfirmZamani=0; mod=1; hazirMesajIndex=0; ekraniGuncelle();
        } else {
          sonConfirmZamani=simdi;
        }
      }
    }
    delay(200);
  }

  if(sonConfirmZamani != 0 && (millis()-sonConfirmZamani>CIFT_TIK_ARALIK)) {
    if(mod==0 && mesaj.length()>0) {
      mesajGonder(mesaj); mesaj=""; harfIndex=0; delay(500); ekraniGuncelle();
    }
    sonConfirmZamani=0;
  }

  // ===== TELEGRAM KONTROL =====
  if(millis()-sonTelegramKontrol > TELEGRAM_ARALIK) {
    if(WiFi.status()==WL_CONNECTED) {
      int ms=bot.getUpdates(bot.last_message_received+1);
      if(ms>0) {
        for(int i=0;i<ms;i++) {
          String chatID = bot.messages[i].chat_id;
          String fromID = bot.messages[i].from_id;
          int kisiIndex = chatIDdenKisiIndex(chatID, fromID);
          if(kisiIndex >= 0) {
            mesajEkle(bot.messages[i].text, "o", kisiIndex);
            if(!aktifKisidanMi(kisiIndex)) {
              bildirimVar = true;
              bildirimZamani = millis();
            }
          }
        }
        telegramBagli=true; sonBasariliIstek=millis(); gelenSes(); ekraniGuncelle();
      }
      if(millis()-sonBasariliIstek>15000) telegramBagli=false;
    }
    sonTelegramKontrol=millis();
  }
}
