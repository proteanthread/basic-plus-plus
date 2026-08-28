# `iotkit` Internet of Things & Sensor Integration Library

## 1. Library Overview & Usage

The `iotkit` library provides high-level APIs for reading temperature, humidity, light, and motion sensors, controlling GPIO pins, and publishing telemetry data over MQTT and HTTP.

### Features:
- **GPIO Pin Control**: Digital read/write and PWM duty cycle output.
- **Sensor Drivers**: Pre-configured handlers for DHT11/22, BME280, and I2C sensors.

---

## 2. BASIC Example

```basic
10 OPEN "GPIO:18" FOR OUTPUT AS #1
20 PRINT #1, "1" : REM Turn LED on
30 SLEEP 1
40 PRINT #1, "0" : REM Turn LED off
50 CLOSE #1
```
