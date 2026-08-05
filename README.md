# ha_ir_esp8266

ИК-мост на ESP8266, который позволяет управлять кондиционером из Home
Assistant по MQTT (MQTT Discovery, сущность `climate`).

## Кондиционер

**NEOLINE NAG/in-09HN1** — это OEM-кондиционер на базе Midea: официальный
Wi-Fi модуль для него работает через приложение **NetHome Plus**
(`com.midea.aircondition`), а набор функций пульта (iClean, Turbo, Health,
Quiet, H/V-Sweep) типичен для Midea-прошивки. Поэтому в проекте используется
ИК-протокол `MIDEA` (см. `include/config.h`).

<p>
  <img src="docs/photos/con1.jpg" alt="Шильдик кондиционера NEOLINE NAG/in-09HN1" width="45%">
  <img src="docs/photos/pult.jpg" alt="Пульт NEOLINE" width="45%">
</p>

## Плата

Целевая плата: **TYSW_012_ZK_MAIN_V1.1** — круглый Tuya ИК-пульт на голом
модуле **TYWE3S** (ESP8266EX, 1 МБ флеша). Совпадает с шаблоном Tasmota
["YTF IR Bridge"](https://tasmota.github.io/docs/devices/YTF-IR-Bridge/),
поэтому пины в `include/config.h` заданы по нему:

- `GPIO14` — кольцо ИК-светодиодов (через транзистор `U2`), передача
- `GPIO5` — встроенный ИК-приёмник
- `GPIO4` — встроенный синий светодиод статуса (активный low, пока не используется)
- `GPIO13` — встроенная кнопка (пока не используется)

<p>
  <img src="docs/photos/plata1.jpg" alt="Плата, лицевая сторона" width="45%">
  <img src="docs/photos/plata2.jpg" alt="Плата, модуль TYWE3S" width="45%">
  <img src="docs/photos/plata3.jpg" alt="Плата, обратная сторона" width="45%">
  <img src="docs/photos/plata4.jpg" alt="Плата, разъём USB и модуль" width="45%">
</p>

На плате есть 7-пиновый разъём (`3V3 RST TXD RXD IO0 IO2 GND`) для прошивки.
Подключение USB-TTL адаптера на 3.3В:

| Разъём платы | USB-TTL адаптер |
|---|---|
| 3V3 | 3.3V |
| GND | GND |
| TXD | RX |
| RXD | TX |
| IO0 | GND (только на время прошивки — переводит ESP8266 в режим загрузчика) |

Замкните `IO0` на `GND`, затем подайте питание / нажмите `RST`, чтобы войти
в режим прошивки, залейте firmware, после — отключите `IO0` от `GND` и
перезагрузите плату для обычной работы. `board_build.flash_size` в
`platformio.ini` выставлен в 1 МБ по даташиту TYWE3S — если `pio run -t
upload` ругается на несовпадение размера флеша, поправьте это значение (или
уточните чип командой `esptool.py flash_id`).

## Настройка

1. Скопируйте `include/secrets.h.example` в `include/secrets.h` и впишите
   свои данные WiFi и MQTT-брокера. Файл `secrets.h` в `.gitignore` и в
   репозиторий не попадает.
2. В `include/config.h` при необходимости скорректируйте `kAcProtocol` (по
   умолчанию `MIDEA`, см. обоснование выше). Если команды кондиционер
   игнорирует, прошейте firmware, наведите оригинальный пульт на встроенный
   ИК-приёмник и посмотрите на реальные коды в Serial-мониторе / MQTT-топике
   `ha_ir_esp8266/<id>/learn`.
3. Соберите и залейте прошивку через PlatformIO:

   ```
   pio run -t upload
   ```

4. Устройство опубликует MQTT Discovery конфиг для сущности `climate` — она
   должна появиться в Home Assistant автоматически.

## MQTT-топики

Все топики с префиксом `ha_ir_esp8266/<chip-id>/`:

- `mode/set`, `mode/state` — off/auto/cool/heat/dry/fan_only
- `temp/set`, `temp/state` — целевая температура в °C
- `fan/set`, `fan/state` — auto/min/low/medium/high/max
- `availability` — online/offline (LWT)
- `learn` — JSON с любым пойманным ИК-кодом (для отладки/подбора протокола)

## Статус

Начальный каркас: WiFi + MQTT, Home Assistant MQTT Discovery, отправка ИК
через `IRac`, приём ИК для распознавания протокола. На реальном железе ещё
не проверялось.
