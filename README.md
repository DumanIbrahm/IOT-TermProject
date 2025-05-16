# 💧 Smart Irrigation System with ESP32 and Adafruit IO

This is an IoT-based smart irrigation system that automates plant watering using a real **water pump** controlled by a relay module. The system monitors soil moisture levels in real time via an OLED display and allows remote control via the Adafruit IO dashboard.

## 🌟 Features
- Automatic watering with a real water pump when soil is dry
- OLED display for live data visualization
- Manual pump control via Adafruit IO
- Wi-Fi connection using ESP32
- Dual operation modes: Automatic and Manual
- Cloud-based monitoring with MQTT

---

## 🧰 Hardware Components

- 🧠 ESP32 DevKit V1
- 🌱 Analog Soil Moisture Sensor
- 💦 5V DC Water Pump
- ⚡ 1-Channel Relay Module (to control the pump)
- 🖥️ SSD1306 OLED Display (I2C)
- Breadboard, jumper wires, USB cable, and power supply

---

## 🔌 Circuit Connections

| Component           | ESP32 Pin  | Description                 |
|---------------------|------------|-----------------------------|
| Soil Sensor (AOUT)  | GPIO32     | Analog moisture reading     |
| Relay IN            | GPIO25     | Pump control signal         |
| OLED SDA            | GPIO21     | I2C data                    |
| OLED SCL            | GPIO22     | I2C clock                   |
| GND (All)           | GND        | Common ground               |
| VCC (OLED + Sensor) | 3.3V       | Power source                |
| Relay VCC           | 3.3V or 5V | Depends on relay module     |
| Pump + VCC          | Relay NO   | Pump powered via relay      |

⚠️ *Ensure proper power supply for the pump (e.g., external 5V if needed).*

---

## 🖼️ System Diagram

```
Soil Sensor ──> ESP32 ──> Relay ──> Water Pump
                     │
                 OLED
                     │
               Wi-Fi MQTT
                     │
             Adafruit IO Dashboard
```

---

## ☁️ Adafruit IO Integration

Feeds Used:
- `soil-moisture`: Current soil moisture value
- `pump-status`: ON/OFF pump status
- `manual-mode`: Toggle auto/manual control
- `manual-pump`: Manual ON/OFF switch (if manual mode is active)

Dashboard includes:
- Soil moisture gauge
- Pump toggle buttons
- Live status indicators

---
