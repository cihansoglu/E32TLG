# E32TLG — ESP32 Telegram Chat Device
📄 **Documentation:** [English](index.html) | [Türkçe](E32TLG_v310_TR.html)
> ESP32-powered Telegram chat device with OLED + rotary encoder. Send & receive messages without a smartphone. Per-contact history, WiFi memory, buzzer alerts, and family group support. Just a knob and two buttons.

![Version](https://img.shields.io/badge/version-3.1.0-00e5ff?style=flat-square)
![Platform](https://img.shields.io/badge/platform-ESP32-00ff9d?style=flat-square)
![License](https://img.shields.io/badge/license-MIT-ff6b35?style=flat-square)

---

## What is E32TLG?

E32TLG is a standalone, pocket-sized two-way chat device built with an **ESP32** microcontroller, a **1.3" OLED + EC11 rotary encoder + 2-button** module, and the **Telegram Bot API**. It lets you send and receive Telegram messages using only a rotary knob and two buttons — no smartphone, no keyboard, no touchscreen.

You select letters by rotating the encoder, confirm each letter with a button press, and send the message with a long press. Incoming messages appear on the OLED screen in real time.

---

## Features

| Feature | Description |
|---|---|
| 📡 WiFi Network Selection | Scan nearby networks on startup, select and connect via encoder |
| 💾 WiFi Memory | Last 3 networks saved — reconnects without re-entering password |
| 💬 Two-Way Chat | Send messages to Telegram, receive and display replies on OLED |
| ⌨️ Encoder Keyboard | Interrupt-based encoder for smooth, lag-free letter selection |
| 👤 Contact Selection | Choose who to message on each boot (or anytime with long press) |
| 📜 Per-Contact History | Separate message history for each contact |
| 👨‍👩‍👧 Family Group | Send to a Telegram group — everyone sees it |
| ✉️ Smart Notification | Blinking envelope icon when a message arrives from another contact |
| 🔊 Audio Alerts | 2-tone beep on send, 3-tone beep on receive |
| 📶 Signal Indicator | 4-bar WiFi signal strength in status bar |
| ✓/✗ Telegram Status | Connection indicator (✓ connected, ✗ disconnected) |
| 📝 Quick Messages | Pre-defined messages selectable via double-tap |
| 🔄 Scrolling Text | Long messages scroll horizontally on the same line |

---

## Hardware

| Component | Specs |
|---|---|
| **ESP32 DevKit** | WiFi + BT SoC, ISM2.4G 802.11 b/g/n |
| **OLED Encoder Module** | 1.3" SH1106 128×64, EC11 rotary + BACK + CONFIRM buttons |
| **Passive Buzzer** | Audio feedback for sent/received messages |

> The OLED encoder module used in this project is an all-in-one board combining a 1.3" SH1106 OLED display, an EC11 rotary encoder, and two push buttons (BACK and CONFIRM) on a single PCB.

---

## Pin Connections

| Module Pin | ESP32 Pin | Description |
|---|---|---|
| VCC | 3.3V | ⚠️ Not 5V! |
| GND | GND | Ground |
| SDA | D21 | I2C Data |
| SCL | D22 | I2C Clock |
| TRA (Encoder A) | D18 | Interrupt |
| TRB (Encoder B) | D19 | Direction detection |
| PSH (Encoder Press) | D5 | Confirm letter |
| BAK | D2 | Back / Delete |
| CON | D4 | Send / Menu |
| Buzzer (+) | D23 | Passive buzzer |
| Buzzer (-) | GND | Ground |

---

## OLED Screen Layout

```
┌────────────────────────────────┐
│ >Mom            ▌▌▌▌  ✓       │  ← Status bar (contact, WiFi, Telegram)
│────────────────────────────────│
│ >>hello there                  │  ← Sent message
│ <<on my way!                   │  ← Received message
│────────────────────────────────│
│ >good_                         │  ← Input area (current letter cursor)
└────────────────────────────────┘
```

- `>>` = message you sent
- `<<` = message received
- `>` = current input
- `▌▌▌▌` = WiFi signal bars (filled = strong)
- `✓` = Telegram connected / `✗` = disconnected
- Envelope icon blinks when a new message arrives from another contact

---

## Controls

| Action | Function |
|---|---|
| **Rotate Encoder** | Select letter / character (a-z, 0-9, punctuation) |
| **PSH (short press)** | Add selected letter to message |
| **BACK (short press)** | Delete last letter / exit menu |
| **BACK (hold 3s)** | Go to WiFi selection screen |
| **CONFIRM (single)** | Send message |
| **CONFIRM (double tap)** | Open quick message menu |
| **CONFIRM (hold 2s)** | Go to contact selection screen |
| **Encoder in menu** | Navigate quick messages |
| **CONFIRM in menu** | Send selected quick message |

---

## Contact List

On each boot (or by holding CONFIRM for 2 seconds), you choose who to message:

1. **Mom** → sends to Mom's Telegram
2. **Dad** → sends to Dad's Telegram
3. **Sister** → sends to Sister's Telegram
4. **Family Group** → sends to the shared Telegram group (everyone sees it)

> Customize the contact names and Chat IDs in the code to fit your own setup.

---

## Quick Messages

Double-tap CONFIRM to open the quick message menu. Scroll with encoder, send with CONFIRM:

1. Yes.
2. OK.
3. No.
4. On my way.
5. Where are you?
6. Waiting.
7. Wait, I'm coming.

> You can customize these in the code under `const char* quickMessages[]`.

---

## Required Libraries

Install all of these via **Arduino IDE → Sketch → Include Library → Manage Libraries**:

| Library | Author | How to install |
|---|---|---|
| U8g2 | oliver | Library Manager → "U8g2" |
| UniversalTelegramBot | Brian Lough | Library Manager → "UniversalTelegramBot" |
| ArduinoJson | Benoit Blanchon | Library Manager → "ArduinoJson" |
| Preferences | ESP32 Built-in | Included automatically |
| ESP32 Board | Espressif | Board Manager → "esp32 by Espressif" |

---

## Telegram Setup

### 1. Create a Bot

1. Open Telegram and find **@BotFather**
2. Send `/newbot`
3. Enter a name and username (username must end with `bot`)
4. BotFather gives you a **Token** like:
   ```
   1234567890:AAGgIaQsSz2a29SUOVgeZFA-otTS4cvhaHo
   ```
5. Add it to the code:
   ```cpp
   #define BOT_TOKEN "your_token_here"
   ```

### 2. Get Personal Chat IDs

For each person:
1. Have them find your bot on Telegram (search `@yourbotname`)
2. They send `/start`
3. They open **@userinfobot** and send `/start`
4. Their **Chat ID** appears next to "Your ID"

**Alternative method** — upload this snippet to ESP32, have each person message your bot, and read their Chat ID from Serial Monitor (115200 baud):

```cpp
void loop() {
  int ms = bot.getUpdates(bot.last_message_received + 1);
  for(int i = 0; i < ms; i++) {
    Serial.print("Name: ");
    Serial.println(bot.messages[i].from_name);
    Serial.print("Chat ID: ");
    Serial.println(bot.messages[i].chat_id);
    Serial.println("---");
  }
  delay(2000);
}
```

### 3. Create a Family Group

1. Create a new Telegram group and add family members
2. Add your bot to the group (Add member → search bot name)
3. **Important:** Go to @BotFather → `/mybots` → Bot Settings → **Group Privacy → Turn OFF**
4. Send a message in the group, run the snippet above — the group Chat ID will appear
5. Group IDs start with a **minus sign**: `-1001234567890`

### 4. Add to Code

```cpp
#define BOT_TOKEN      "your_bot_token"
#define CHAT_ID_MOM    "moms_chat_id"
#define CHAT_ID_DAD    "dads_chat_id"
#define CHAT_ID_SISTER "sisters_chat_id"
#define CHAT_ID_GROUP  "-group_chat_id"   // minus sign included!
```

---

## How to Build & Flash

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Add ESP32 board support:
   - Go to **File → Preferences**
   - Add this URL to "Additional Board Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to **Tools → Board → Board Manager** → search "esp32" → install **esp32 by Espressif**
3. Install all required libraries (see table above)
4. Open the `.ino` file
5. Fill in your Bot Token and Chat IDs
6. Select **Tools → Board → ESP32 Dev Module**
7. Select the correct port
8. Click **Upload**

---

## First Boot

1. Device powers on and shows **"Scanning WiFi..."**
2. Rotate encoder to select your WiFi network (saved networks marked with `*`)
3. Press PSH to select → enter password letter by letter → press CONFIRM to connect
4. On success, **contact selection screen** appears
5. Choose who to message → start chatting

> On subsequent boots, the device auto-connects to the last saved network and goes straight to contact selection.

---

## Customization

### Change Contact Names & IDs

```cpp
// In the code, update these arrays:
const char* contacts[] = {"Mom", "Dad", "Sister", "Family Group"};
const char* chatIDs[]  = {CHAT_ID_MOM, CHAT_ID_DAD, CHAT_ID_SISTER, CHAT_ID_GROUP};
```

### Change Quick Messages

```cpp
const char* quickMessages[] = {
  "Yes.", "OK.", "No.",
  "On my way.", "Where are you?",
  "Waiting.", "Wait, I'm coming."
};
```

### Change Telegram Poll Interval

```cpp
#define TELEGRAM_INTERVAL 3000  // milliseconds (default: 3 seconds)
```

### Change Notification Duration

```cpp
#define NOTIFICATION_DURATION 2000  // milliseconds (default: 2 seconds)
```

---

## Turkish Character Handling

Incoming Telegram messages with Turkish characters are automatically converted for OLED display:

| Turkish | Displayed as |
|---|---|
| ş, Ş | s |
| ğ, Ğ | g |
| ü, Ü | u |
| ö, Ö | o |
| ç, Ç | c |
| ı | i |

---

## Troubleshooting

| Problem | Solution |
|---|---|
| OLED shows static / noise | Check SDA/SCL wiring, verify I2C address (0x3C) |
| WiFi won't connect | Check SSID/password, ensure 2.4GHz (not 5GHz) |
| Telegram messages not sending | Verify Bot Token and Chat ID are correct |
| Group messages not received | Enable Group Privacy OFF in BotFather settings |
| Encoder skips / double-counts | Check TRA/TRB pin connections |
| No sound from buzzer | Verify buzzer (+) on D23, (-) on GND |
| Serial Monitor freezes IDE | Install CP2102 or CH340 USB driver for your OS |

---

## Project Structure

```
E32TLG/
├── E32TLG.ino          ← Main Arduino sketch
├── README.md           ← This file
└── docs/
    └── E32TLG_v310_EN.html  ← Full project documentation
```

---

## License

MIT License — free to use, modify, and share. If you build one, we'd love to hear about it!

---

## Version History

| Version | Changes |
|---|---|
| v3.1.0 | Per-contact message history, smart notifications, Family Group support |
| v3.0.0 | Contact selection, WiFi memory (last 3 networks), CONFIRM long-press to change contact |
| v2.0.0 | Quick messages, scrolling text, buzzer alerts, WiFi signal bars |
| v1.0.0 | Initial release — basic send/receive over Telegram |
