# 📡 ESP8266 IR Remote

![Platform](https://img.shields.io/badge/platform-ESP8266-blue)
![Framework](https://img.shields.io/badge/framework-Arduino-00979D)
![Build](https://img.shields.io/badge/build-PlatformIO-orange)

Biến một con **ESP8266** giá rẻ thành **remote hồng ngoại đa năng**: học lệnh từ remote bất kỳ và điều khiển **TV, điều hòa, quạt, đầu thu...** ngay trên điện thoại — **không cần cài app, không cần Internet**.

Xây dựng trên thư viện [IRremoteESP8266](https://github.com/crankyoldgit/IRremoteESP8266).

## Mục lục

- [Tính năng](#-tính-năng)
- [Bắt đầu nhanh](#-bắt-đầu-nhanh)
- [Phần cứng](#-phần-cứng)
- [Hướng dẫn sử dụng](#-hướng-dẫn-sử-dụng)
- [Tự dò hãng điều hòa hoạt động thế nào?](#-tự-dò-hãng-điều-hòa-hoạt-động-thế-nào)
- [HTTP API](#-http-api)
- [Cấu trúc project](#-cấu-trúc-project)
- [Xử lý sự cố](#-xử-lý-sự-cố)
- [Chuyển sang ESP32](#-chuyển-sang-esp32)
- [Giấy phép](#-giấy-phép)

---

## ✨ Tính năng

| | |
|---|---|
| 🎓 **Học lệnh** | Học từ remote IR bất kỳ, tự nhận diện giao thức (NEC, Sony, Samsung, LG, RC5/RC6...). Remote lạ → lưu và phát lại **tín hiệu thô (raw)**. |
| ❄️ **Điều hòa** | Điều khiển đầy đủ: nguồn, nhiệt độ 16–32°C, chế độ, quạt, đảo gió, Turbo, Êm, Tiết kiệm, Đèn. Hỗ trợ **70+ hãng** (Daikin, Panasonic, LG, Samsung, Toshiba, Mitsubishi, Gree, Midea, Sharp, Fujitsu, Hitachi, Carrier, Haier, TCL, Coolix...). |
| 🔍 **Tự dò hãng** | Bấm 1 nút trên remote điều hòa là biết hãng. |
| 📶 **Captive portal** | Phát WiFi mở `IR-Remote`, điện thoại kết nối là **tự bật trang điều khiển**. |
| 📱 **Giao diện như app** | Vừa khít 1 màn hình, tab dưới đáy, nút lớn, rung phản hồi, nhấn giữ để sửa/xóa, tự đổi sáng/tối. |
| 🏠 **WiFi nhà (tùy chọn)** | Kết nối thêm WiFi nhà, truy cập qua `http://irremote.local`. |
| 💾 **Lưu bền vững** | Dữ liệu lưu trên **LittleFS**, không mất khi mất điện. |
| 🔌 **HTTP API** | Dễ tích hợp Home Assistant, Node-RED, script. |

---

## 🚀 Bắt đầu nhanh

**Yêu cầu:** [VS Code](https://code.visualstudio.com/) + extension [PlatformIO](https://platformio.org/).
Thư viện (`IRremoteESP8266`, `ArduinoJson` v7) được PlatformIO tự cài theo `platformio.ini`.

```bash
git clone git@github.com:HuYingTran/ESP_IR_REMOTE.git
cd ESP_IR_REMOTE

pio run -t upload        # biên dịch & nạp firmware
pio device monitor       # xem log Serial (115200 baud)
```

Sau khi nạp: trên điện thoại kết nối WiFi **`IR-Remote`** → trang điều khiển tự mở (hoặc vào **http://192.168.4.1**).

---

## 🔧 Phần cứng

| Linh kiện | Ghi chú |
|---|---|
| ESP8266 (ESP-12E / NodeMCU / Wemos D1 mini) | Flash 4MB |
| Mắt thu IR **VS1838B** hoặc **TSOP38238** | 38kHz |
| LED phát IR **940nm** (5mm) | Dùng 2 LED nếu cần phát xa |
| Transistor NPN **S8050 / 2N2222** | Khuếch đại dòng cho LED |
| Điện trở 1kΩ và 33–100Ω | |

### Sơ đồ nối dây

```
Mắt thu IR (VS1838B)            LED phát IR
┌─────────┐
│ OUT ────┼──► GPIO14 (D5)       5V/3V3 ── R 33–100Ω ──►|── LED IR ──┐
│ GND ────┼──► GND                                                  │ C
│ VCC ────┼──► 3V3               GPIO4 (D2) ── R 1kΩ ──────────── B  NPN (S8050)
└─────────┘                                                         │ E
                                                                   GND
```

| Chức năng | GPIO | NodeMCU |
|---|---|---|
| Mắt thu IR (OUT) | 14 | D5 |
| LED phát IR (qua transistor) | 4 | D2 |

> [!WARNING]
> Không nối LED IR trực tiếp vào chân ESP — dòng quá nhỏ, chỉ phát được vài chục cm. Dùng transistor để phát xa 5–8m.
> Tránh GPIO0, GPIO2, GPIO15 (chân khởi động).

Đổi chân trong [src/main.cpp](src/main.cpp):

```cpp
constexpr uint16_t IR_RECV_PIN = 14;   // D5
constexpr uint16_t IR_SEND_PIN = 4;    // D2
```

---

## 📱 Hướng dẫn sử dụng

### 1. Kết nối

| WiFi | Mật khẩu | Địa chỉ |
|---|---|---|
| `IR-Remote` | *(không có)* | http://192.168.4.1 |

> [!TIP]
> - **Android** hỏi *"Mạng không có Internet, vẫn kết nối?"* → chọn **Có / Giữ kết nối**.
> - **iPhone**: popup captive portal sẽ ngắt WiFi khi đóng. Muốn dùng lâu, bấm **Hủy → Sử dụng không có Internet**, rồi mở Safari vào `192.168.4.1`.
> - **Thêm vào màn hình chính** để mở như một app.

### 2. Các tab trong giao diện

**🎮 Remote** — điều khiển các thiết bị đã học
- Chọn thiết bị ở thanh trên cùng, bấm nút để phát lệnh.
- Nhiều nút → **vuốt ngang** để sang trang.
- **Nhấn giữ** nút để đổi tên, chuyển thiết bị hoặc xóa. Nút **＋** để học thêm.

**➕ Học lệnh**
1. Bấm **Bắt đầu học** (chờ tối đa 30 giây).
2. Hướng remote vào mắt thu (5–15cm), bấm nút cần học.
3. Bấm **Phát thử** để kiểm tra.
4. Nhập **Thiết bị** (VD: `TV Phòng khách`) và **Tên nút** (VD: `Nguồn`).
5. **💾 Lưu & học nút tiếp** để học liên tục cùng thiết bị, hoặc **Lưu** để kết thúc.

**❄️ Điều hòa**
1. Bấm tên hãng → **🔍 Tự dò hãng** → bấm 1 nút bất kỳ trên remote điều hòa.
2. Chỉnh nhiệt độ, chế độ, quạt, đảo gió... — lệnh gửi ngay.
3. Có thể chọn hãng thủ công. Đúng hãng mà không ăn lệnh → thử đổi **Model** (1, 2, 3...).

**⚙️ Cài đặt**
- **📶 Tìm WiFi** → chọn mạng → nhập mật khẩu → **Kết nối**. Sau đó truy cập qua **http://irremote.local** hoặc IP hiển thị.
- Xem số thiết bị đang kết nối, RAM trống, dung lượng bộ nhớ; quên WiFi nhà.

> [!NOTE]
> WiFi `IR-Remote` luôn bật song song với WiFi nhà. Nếu WiFi nhà không kết nối được sau 15 giây, thiết bị ngừng thử để giữ `IR-Remote` ổn định.

---

## 🧠 Tự dò hãng điều hòa hoạt động thế nào?

Dữ liệu các hãng **nằm sẵn trong firmware** (thư viện IRremoteESP8266, do cộng đồng giải mã ngược) — không cần Internet.

Khác với remote TV (chỉ gửi mã nút vừa bấm), **remote điều hòa mỗi lần bấm gửi toàn bộ trạng thái** (nguồn, nhiệt độ, chế độ, quạt, swing...) kèm checksum. Khi bắt được tín hiệu, `irrecv.decode()` thử bộ giải mã của từng hãng và so khớp:

1. Độ dài xung mở đầu (header)
2. Độ dài từng bit (sai số ±25%)
3. Tổng số bit (VD: Daikin 280 bit, Gree 64 bit)
4. Checksum

Hãng khớp → `IRAcUtils::decodeToState()` chuyển thành thông số chung. Khi điều khiển, `IRac::sendAc()` dựng lại gói tin đúng giao thức hãng đó.

> Nhiều hãng bán tại Việt Nam (Casper, Nagakawa, Funiki...) dùng lại giao thức của Gree, Midea, TCL, Kelon nên sẽ được nhận diện dưới tên các hãng này.

---

## 🌐 HTTP API

Request `POST` dùng `application/x-www-form-urlencoded`, phản hồi JSON.

| Method | Endpoint | Tham số | Mô tả |
|---|---|---|---|
| GET | `/api/info` | | Thông tin thiết bị |
| GET | `/api/buttons` | | Danh sách nút `[{id, n, d, p}]` |
| POST | `/api/send` | `id` | Phát lệnh của nút |
| POST | `/api/rename` | `id`, `n`, `d` | Đổi tên nút / thiết bị |
| POST | `/api/delete` | `id` | Xóa nút |
| POST | `/api/learn/start` | `mode` = `btn` \| `ac` | Bắt đầu học (30s) |
| GET | `/api/learn/status` | | `idle` / `waiting` / `captured` / `timeout` |
| POST | `/api/learn/test` | | Phát thử lệnh vừa học |
| POST | `/api/learn/save` | `n`, `d` | Lưu lệnh vừa học |
| POST | `/api/learn/cancel` | | Hủy học |
| GET | `/api/ac` | | Trạng thái điều hòa + danh sách giao thức |
| POST | `/api/ac` | `proto`, `model`, `power`, `mode`, `temp`, `fan`, `swing`, `turbo`, `quiet`, `econo`, `light`, `send` | Đặt & phát lệnh điều hòa (`send=0` chỉ lưu) |
| GET | `/api/wifi/scan` | | Quét WiFi (gọi lặp đến khi hết `scanning`) |
| POST | `/api/wifi/save` | `ssid`, `pass` | Lưu & kết nối WiFi nhà |
| POST | `/api/wifi/forget` | | Quên WiFi nhà |

**Giá trị enum điều hòa**

| Tham số | Giá trị |
|---|---|
| `mode` | `0` Tự động · `1` Lạnh · `2` Sưởi · `3` Khô · `4` Quạt |
| `fan` | `0` Tự động · `1` Min · `2` Thấp · `3` Vừa · `4` Cao · `5` Max |
| `swing` | `-1` Tắt · `0` Tự động · `1` Cao nhất … `5` Thấp nhất |

**Ví dụ**

```bash
# Bật điều hòa 25°C chế độ lạnh
curl -X POST http://irremote.local/api/ac -d "power=1&mode=1&temp=25"

# Phát nút có id = 3
curl -X POST http://irremote.local/api/send -d "id=3"
```

---

## 📁 Cấu trúc project

```
ESP_IR_REMOTE/
├── platformio.ini     # Board, thư viện, LittleFS
├── include/
│   └── web_ui.h       # Giao diện web (HTML/CSS/JS nhúng trong flash)
└── src/
    └── main.cpp       # Firmware: học/phát IR, điều hòa, WiFi, web API
```

Dữ liệu trên LittleFS:

```
/b/<id>.json   # Mỗi nút đã học: tên, thiết bị, giao thức, mã, raw
/ac.json       # Trạng thái điều hòa gần nhất
/wifi.json     # WiFi nhà (nếu có)
```

---

## 🛠 Xử lý sự cố

| Hiện tượng | Cách khắc phục |
|---|---|
| Học lệnh báo hết thời gian | Kiểm tra dây mắt thu, pin remote; đưa remote gần hơn (5–15cm) |
| Phát thử không phản hồi | Nhìn LED IR qua camera điện thoại (thấy nháy tím); dùng transistor; hướng LED về thiết bị |
| Chỉ phát được ở gần | Giảm điện trở LED (tối thiểu 33Ω), cấp 5V cho LED, thêm LED thứ 2 |
| Không nhận diện được hãng điều hòa | Hãng chưa hỗ trợ → học từng nút như remote TV |
| Đúng hãng nhưng không ăn lệnh | Đổi **Model** trong tab Điều hòa |
| Kết nối `IR-Remote` nhưng trang không tự bật | Mở `http://192.168.4.1`; tắt dữ liệu di động |
| iPhone ngắt WiFi khi đóng popup | Chọn **Hủy → Sử dụng không có Internet**, rồi dùng Safari |
| Không vào được `irremote.local` | Dùng IP hiển thị trong tab ⚙️ Cài đặt |
| Đổi WiFi nhà | Tab ⚙️ → **Quên WiFi này** → **Tìm WiFi** |

---

## 🔄 Chuyển sang ESP32

IRremoteESP8266 cũng hỗ trợ ESP32. Những chỗ cần sửa:

| ESP8266 | ESP32 |
|---|---|
| `platform = espressif8266`, `board = esp12e` | `platform = espressif32`, `board = esp32dev` |
| `ESP8266WiFi.h`, `ESP8266WebServer.h`, `ESP8266mDNS.h` | `WiFi.h`, `WebServer.h`, `ESPmDNS.h` |
| `LittleFS.openDir()` / `Dir` | `LittleFS.open(dir)` + `openNextFile()` |
| `FSInfo` / `LittleFS.info()` | `LittleFS.usedBytes()`, `LittleFS.totalBytes()` |
| `MDNS.update()` | Bỏ |

> Với Arduino core 3.x, dùng bản IRremoteESP8266 mới nhất từ GitHub:
> `lib_deps = https://github.com/crankyoldgit/IRremoteESP8266.git`

---

## 📜 Giấy phép

Project sử dụng các thư viện:

- [IRremoteESP8266](https://github.com/crankyoldgit/IRremoteESP8266) — LGPL-2.1
- [ArduinoJson](https://arduinojson.org/) — MIT
