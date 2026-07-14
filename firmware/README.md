# Nixie Clock Firmware

Firmware for the ESP32-based nixie clock.

## How it works

- On boot the firmware forces the outputs into a safe state.
- It connects to WiFi, configures NTP, and waits for valid time.
- Once time is valid, high voltage is enabled and the display starts multiplexing `HH:MM`.
- WiFi is shut down after sync unless the OTA maintenance window is active.
- During OTA the display is blanked and high voltage stays off.

## Main pieces

- `ClockTime` handles WiFi, NTP, timezone, and time validity.
- `NixieDisplay` owns the display buffer and multiplexing.
- `NixieSafetyController` keeps display and high voltage changes safe.
- `ClockDisplay` updates the shown time once per minute.
- `StatusLed` shows boot, waiting, sync, and OTA states.
- `OtaMaintenance` opens a short OTA window after sync.
- `CathodeCleaning` runs the anti-poisoning cycle at midnight and on long press.

## Controls

- Button 1 long press starts cathode cleaning.

## Configuration

- WiFi credentials currently live in `src/config.h`.
- Debug logging is enabled with the `DEBUG_LOG` build flag.
