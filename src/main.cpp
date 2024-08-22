// ------------------------------------------------------------
// ライブラリインクルード部 Library include section.
// ------------------------------------------------------------
#include <M5StickCPlus.h>   // M5StickC 用ライブラリ Library for M5StickC.
#include <WiFi.h>           // Wifi制御用ライブラリ Wifi control library.
#include <Preferences.h>    // 不揮発静メモリ制御ライブラリ Non-volatile static memory control library.
#include <time.h>           // 時刻制御用ライブラリ Time control library.
#include "config.h"         // 設定ファイル　Setting file.

// NTP接続情報　NTP connection information.
const char* NTPSRV          = "ntp.jst.mfeed.ad.jp";  // NTPサーバーアドレス NTP server address.
const long  GMT_OFFSET      = 9 * 3600;               // GMT-TOKYO(時差９時間）9 hours time difference.
const int   DAYLIGHT_OFFSET = 0;                      // サマータイム設定なし No daylight saving time setting

// 時刻・日付の生成　Time / date generation.
RTC_TimeTypeDef RTC_TimeStruct;                    // RTC時刻　Times of Day.
RTC_DateTypeDef RTC_DateStruct;                    // RTC日付  Date 
int SMIN = 0;

// 関数プロトタイプ宣言 Function prototype declaration.
void time_sync(const char* ntpsrv, long gmt_offset, int daylight_offset);
void Clock_screen_display();

// ------------------------------------------------------------
// 時刻同期 関数　Time synchronization function.
// ------------------------------------------------------------
void time_sync(const char* ntpsrv, long gmt_offset, int daylight_offset) {

  // NTPサーバの時間とローカルの時刻を同期　Synchronize NTP server time to local time

  configTime(gmt_offset, daylight_offset, ntpsrv);// NTPの設定 Set NTP settings

  // Get local time
  struct tm timeInfo;                             // tmオブジェクトをtimeinfoとして生成 Create a tm object as timeinfo
  if (getLocalTime(&timeInfo)) {                  // timeinfoに現在時刻を格納 Get the current time in timeinfo
    // 現在時刻の格納が正常終了したら実行
    M5.Lcd.print("NTP : ");                       // LCDに表示 Show on LCD
    M5.Lcd.println(ntpsrv);                       // LCDに表示 Show on LCD

    // 時刻の取り出し
    RTC_TimeTypeDef TimeStruct;                   // 時刻格納用の構造体を生成   Create a structure for storing time
    TimeStruct.Hours   = timeInfo.tm_hour;        // 時を格納 Get hours
    TimeStruct.Minutes = timeInfo.tm_min;         // 分を格納 Get minutes
    // TimeStruct.Seconds = timeInfo.tm_sec;         // 秒を格納  Get seconds
    M5.Rtc.SetTime(&TimeStruct);                  // 時刻の書き込み Write time

    RTC_DateTypeDef DateStruct;                   // 日付格納用の構造体を生成 Create a structure for storing dates
    DateStruct.WeekDay = timeInfo.tm_wday;        // 曜日を格納 Get day of the week
    DateStruct.Month = timeInfo.tm_mon + 1;       // 月（0-11）を格納※1を足す Get month (0-11) * Add 1
    DateStruct.Date = timeInfo.tm_mday;           // 日を格納 Get day
    DateStruct.Year = timeInfo.tm_year + 1900;    // 年を格納（1900年からの経過年を取得するので1900を足す） Get elapsed years from 1900
    M5.Rtc.SetDate(&DateStruct);                  // 日付を書き込み Write date
    M5.Lcd.fillScreen(BLACK);                     // LCDを黒で塗りつぶし Fill the LCD with black
    M5.Lcd.printf("RTC %04d-%02d-%02d %02d:%02d\n",    // LCDに表示 Show on LCD
    DateStruct.Year, DateStruct.Month, DateStruct.Date,
    TimeStruct.Hours, TimeStruct.Minutes);
   

  }
  else {
    M5.Lcd.print("NTP Sync Error ");              // LCDに表示 Show on LCD
  }
}

// ------------------------------------------------------------
// 時計画面の表示用関数　Clock screen display function.
// ------------------------------------------------------------
void Clock_screen_display() {
  static const char *_wd[7] = {"Sun", "Mon", "Tue", "Wed", "Thr", "Fri", "Sat"}; // 曜日の定義　Definition of the day of the week.

  // 時刻・日付の取り出し　Extraction of time and date.
  M5.Rtc.GetTime(&RTC_TimeStruct);              // 時間情報の取り出し　Get time information.
  M5.Rtc.GetDate(&RTC_DateStruct);              // 日付情報の取り出し　Get date information.

  // 画面書き換え処理　Screen rewriting process.
  if (SMIN == RTC_TimeStruct.Minutes) {         // 分単位の変更がかかったかどうか確認 Check if there is a change in minutes.
    M5.Lcd.fillRect(180, 20, 190, 60, BLACK);   // 「秒」だけが変わった場合、秒表示エリアだけ書き換え Rewrite only the display area of seconds.
  } else {
    M5.Lcd.fillScreen(BLACK);                   // 「分」が変わったら画面全体を書き換え Rewrite the entire screen when the "minute" changes.
  }

  // ディスプレイに表示する時間の表示色を指定 Specify the display color of the time to be displayed on the display.
  M5.Lcd.setTextColor(GREEN);

  // 数字・文字表示部分 Number and character display part
  // 時刻表示   Time display
  M5.Lcd.setCursor(40, 10, 7);                 //x,y,font 7:48ピクセル7セグ風フォント 7: 48 pixel 7-segment style font
  M5.Lcd.setTextSize(1);
  M5.Lcd.printf("%02d:%02d", RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes); // 時分を表示 Hours and minutes display

  // 日付表示
  M5.Lcd.setTextSize(2);                       // 文字サイズを2に設定 Set the character size to 2
  M5.Lcd.setTextColor(WHITE);                 //日付表示文字だけ白色の文字色にする Set the text color to white only for the date display text
  M5.Lcd.setCursor(30, 70, 1);                //x,y,font 1:Adafruit 8ピクセルASCIIフォント 1: Adafruit 8 pixel ASCII font
  M5.Lcd.printf("%d.%02d.%02d %s\n", RTC_DateStruct.Year, RTC_DateStruct.Month, RTC_DateStruct.Date, _wd[RTC_DateStruct.WeekDay]); //曜日を表示

  SMIN = RTC_TimeStruct.Minutes;              //表示した「分」を保存 Save the displayed "minute"
}

//------------------------------------------------------------
// 画面表示の向き定数の設定　Holizonal screen display definication.
//------------------------------------------------------------
int HOLIZONAL = 3;

// ------------------------------------------------------------
// Setup 関数　Setup function.
// ------------------------------------------------------------
void setup() {
      // M5StickCの初期化と動作設定　Initialization and operation settings of M5StickC.
      M5.begin(); // 開始 Start
      M5.Lcd.begin(); // LCDの初期化 Initialization of LCD
      // Wi-Fi接続 We start by connecting to a WiFi network
      M5.Lcd.println(); // LCDに出力 Output to LCD
      M5.Lcd.println();
      M5.Lcd.setRotation(HOLIZONAL); // 画面表示の向き設定 Set the screen display orientation
      M5.Lcd.print("Connecting to ");
      M5.Lcd.println(ssid);
      WiFi.begin(ssid, password); // Wi-Fi接続開始 Start Wi-Fi connection
      // Wi-Fi接続の状況を監視（WiFi.statusがWL_CONNECTEDになるまで繰り返し Check the status of the Wi-Fi connection (repeat until WiFi.status is WL_CONNECTED)
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        M5.Lcd.print(".");
      }
      // Wi-Fi接続結果をLCDへ出力  Output Wi-Fi connection result to LCD
      M5.Lcd.println("");
      M5.Lcd.println("WiFi connected");
      M5.Lcd.println("IP address: ");
      M5.Lcd.println(WiFi.localIP());
      // 時刻同期関数
      time_sync(NTPSRV, GMT_OFFSET, DAYLIGHT_OFFSET);
}

void loop() {
    M5.update();                    // M5状態更新　M5 status update.
    Clock_screen_display();         // 時計表示　Clock display.
    delay(980);
      }


