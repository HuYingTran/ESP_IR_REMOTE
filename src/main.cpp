/*
 * ESP8266 IR Remote
 * - Học (learn) lệnh từ remote hồng ngoại bất kỳ, lưu vào LittleFS
 * - Phát lại lệnh đã học để điều khiển TV, quạt, đầu thu...
 * - Điều khiển điều hòa đầy đủ (nhiệt độ, chế độ, quạt, swing) qua IRac
 *   cho hơn 70 hãng (Daikin, Panasonic, LG, Samsung, Toshiba, Gree, Midea...)
 * - Phát WiFi mở "IR-Remote" (không mật khẩu) + captive portal:
 *   điện thoại kết nối vào là tự bật trang điều khiển (http://192.168.4.1)
 * - Tùy chọn kết nối thêm WiFi nhà (cài trong tab Cài đặt) -> http://irremote.local
 *
 * Sơ đồ nối dây (ESP-12E / NodeMCU):
 *   Mắt thu IR (VS1838B/TSOP38238): OUT -> GPIO14 (D5), VCC -> 3V3, GND -> GND
 *   LED phát IR 940nm: GPIO4 (D2) -> R 1k -> cực B transistor NPN (S8050/2N2222)
 *                      E -> GND, C -> catot LED, anot LED -> R 33-100Ω -> 5V/3V3
 */
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <LittleFS.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRac.h>
#include <IRutils.h>
#include <vector>
#include "web_ui.h"

// ---------------- Cấu hình ----------------
constexpr uint16_t IR_RECV_PIN = 14;   // D5
constexpr uint16_t IR_SEND_PIN = 4;    // D2
constexpr uint16_t CAPTURE_BUFFER = 1024;
constexpr uint8_t  CAPTURE_TIMEOUT_MS = 50;   // 50ms đủ cho tín hiệu điều hòa dài
constexpr uint16_t MIN_UNKNOWN_SIZE = 12;
constexpr uint32_t LEARN_TIMEOUT_MS = 30000;
constexpr uint16_t RAW_FREQ_KHZ = 38;
const char *HOSTNAME = "irremote";
const char *AP_NAME = "IR-Remote";          // WiFi mở, không mật khẩu
const IPAddress AP_IP(192, 168, 4, 1);
constexpr uint32_t STA_CONNECT_TIMEOUT_MS = 15000;
const char *WIFI_FILE = "/wifi.json";
const char *BTN_DIR = "/b";
const char *AC_FILE = "/ac.json";

IRrecv irrecv(IR_RECV_PIN, CAPTURE_BUFFER, CAPTURE_TIMEOUT_MS, true);
IRsend irsend(IR_SEND_PIN);
IRac ac(IR_SEND_PIN);
ESP8266WebServer server(80);
DNSServer dns;

// ---------------- Mã IR ----------------
struct IrCode {
  decode_type_t proto = decode_type_t::UNKNOWN;
  uint16_t bits = 0;
  uint64_t value = 0;
  std::vector<uint8_t> state;   // cho giao thức điều hòa (nhiều byte)
  std::vector<uint16_t> raw;    // dạng thô, dùng khi không giải mã được / dự phòng
};

static String bytesToHex(const std::vector<uint8_t> &v) {
  static const char hex[] = "0123456789ABCDEF";
  String s;
  s.reserve(v.size() * 2);
  for (uint8_t b : v) { s += hex[b >> 4]; s += hex[b & 0xF]; }
  return s;
}

static std::vector<uint8_t> hexToBytes(const char *s) {
  std::vector<uint8_t> v;
  if (!s) return v;
  auto nib = [](char c) -> uint8_t {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0;
  };
  for (size_t i = 0; s[i] && s[i + 1]; i += 2) v.push_back((nib(s[i]) << 4) | nib(s[i + 1]));
  return v;
}

static void codeFromResults(const decode_results &r, IrCode &c) {
  c.proto = r.decode_type;
  c.bits = r.bits;
  c.value = r.value;
  c.state.clear();
  c.raw.clear();
  if (hasACState(r.decode_type)) {
    c.state.assign(r.state, r.state + r.bits / 8);
  }
  uint16_t len = getCorrectedRawLength(&r);
  uint16_t *raw = resultToRawArray(&r);
  if (raw) {
    c.raw.assign(raw, raw + len);
    delete[] raw;
  }
}

static void codeToJson(const IrCode &c, JsonObject o) {
  o["p"] = typeToString(c.proto);
  o["b"] = c.bits;
  if (!c.state.empty()) o["s"] = bytesToHex(c.state);
  else if (c.proto != decode_type_t::UNKNOWN) o["v"] = uint64ToString(c.value, 16);
  JsonArray raw = o["r"].to<JsonArray>();
  for (uint16_t t : c.raw) raw.add(t);
}

static void codeFromJson(JsonObjectConst o, IrCode &c) {
  c.proto = strToDecodeType(o["p"] | "UNKNOWN");
  c.bits = o["b"] | 0;
  c.value = strtoull(o["v"] | "0", nullptr, 16);
  c.state = hexToBytes(o["s"] | "");
  c.raw.clear();
  for (JsonVariantConst t : o["r"].as<JsonArrayConst>()) c.raw.push_back(t.as<uint16_t>());
}

static bool sendCode(const IrCode &c) {
  bool ok = false;
  if (c.proto != decode_type_t::UNKNOWN) {
    if (!c.state.empty()) ok = irsend.send(c.proto, c.state.data(), c.state.size());
    else ok = irsend.send(c.proto, c.value, c.bits);
  }
  if (!ok && !c.raw.empty()) {
    irsend.sendRaw(c.raw.data(), c.raw.size(), RAW_FREQ_KHZ);
    ok = true;
  }
  return ok;
}

// ---------------- Trạng thái học lệnh ----------------
enum class LearnState { Idle, Waiting, Captured, Timeout };
enum class LearnMode { Button, AcDetect };
LearnState learnState = LearnState::Idle;
LearnMode learnMode = LearnMode::Button;
uint32_t learnStart = 0;
IrCode pending;
String pendingDesc;
decode_results results;

// ---------------- Trạng thái điều hòa ----------------
stdAc::state_t acState;

static void acDefaults() {
  acState = stdAc::state_t();
  acState.protocol = decode_type_t::DAIKIN;
  acState.model = -1;
  acState.power = false;
  acState.mode = stdAc::opmode_t::kCool;
  acState.celsius = true;
  acState.degrees = 26;
  acState.fanspeed = stdAc::fanspeed_t::kAuto;
  acState.swingv = stdAc::swingv_t::kAuto;
  acState.swingh = stdAc::swingh_t::kOff;
  acState.light = true;
  acState.beep = true;
  acState.sleep = -1;
  acState.clock = -1;
}

static void acToJson(JsonObject o) {
  o["proto"] = typeToString(acState.protocol);
  o["model"] = acState.model;
  o["power"] = acState.power;
  o["mode"] = (int)acState.mode;
  o["temp"] = acState.degrees;
  o["fan"] = (int)acState.fanspeed;
  o["swing"] = (int)acState.swingv;
  o["turbo"] = acState.turbo;
  o["quiet"] = acState.quiet;
  o["econo"] = acState.econo;
  o["light"] = acState.light;
}

static void acSave() {
  JsonDocument doc;
  acToJson(doc.to<JsonObject>());
  File f = LittleFS.open(AC_FILE, "w");
  if (f) { serializeJson(doc, f); f.close(); }
}

static void acLoad() {
  acDefaults();
  File f = LittleFS.open(AC_FILE, "r");
  if (!f) return;
  JsonDocument doc;
  if (deserializeJson(doc, f) == DeserializationError::Ok) {
    decode_type_t p = strToDecodeType(doc["proto"] | "DAIKIN");
    if (IRac::isProtocolSupported(p)) acState.protocol = p;
    acState.model = doc["model"] | -1;
    acState.power = doc["power"] | false;
    acState.mode = (stdAc::opmode_t)(doc["mode"] | 1);
    acState.degrees = doc["temp"] | 26;
    acState.fanspeed = (stdAc::fanspeed_t)(doc["fan"] | 0);
    acState.swingv = (stdAc::swingv_t)(doc["swing"] | 0);
    acState.turbo = doc["turbo"] | false;
    acState.quiet = doc["quiet"] | false;
    acState.econo = doc["econo"] | false;
    acState.light = doc["light"] | true;
  }
  f.close();
}

// ---------------- Lưu trữ nút ----------------
uint32_t nextId = 1;

static String btnPath(uint32_t id) { return String(BTN_DIR) + "/" + id + ".json"; }

static void scanNextId() {
  Dir dir = LittleFS.openDir(BTN_DIR);
  while (dir.next()) {
    uint32_t id = dir.fileName().toInt();
    if (id >= nextId) nextId = id + 1;
  }
}

static bool loadButton(uint32_t id, JsonDocument &doc) {
  File f = LittleFS.open(btnPath(id), "r");
  if (!f) return false;
  bool ok = deserializeJson(doc, f) == DeserializationError::Ok;
  f.close();
  return ok;
}

// ---------------- HTTP helpers ----------------
static void sendJson(const JsonDocument &doc, int code = 200) {
  String out;
  serializeJson(doc, out);
  server.send(code, "application/json", out);
}

static void sendOk() { server.send(200, "application/json", "{\"ok\":true}"); }

static void sendErr(const String &msg, int code = 400) {
  JsonDocument doc;
  doc["ok"] = false;
  doc["error"] = msg;
  sendJson(doc, code);
}

static void learnStop() {
  irrecv.disableIRIn();
}

// ---------------- WiFi nhà (tùy chọn) ----------------
enum class StaState { Off, Connecting, Connected, Failed };
StaState staState = StaState::Off;
String staSsid, staPass;
uint32_t staStart = 0;

static void staConnect() {
  if (!staSsid.length()) return;
  WiFi.mode(WIFI_AP_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(staSsid.c_str(), staPass.c_str());
  staState = StaState::Connecting;
  staStart = millis();
  Serial.printf("Đang kết nối WiFi nhà: %s\n", staSsid.c_str());
}

static void staLoad() {
  File f = LittleFS.open(WIFI_FILE, "r");
  if (!f) return;
  JsonDocument doc;
  if (deserializeJson(doc, f) == DeserializationError::Ok) {
    staSsid = doc["ssid"] | "";
    staPass = doc["pass"] | "";
  }
  f.close();
}

static void wifiLoop() {
  if (staState == StaState::Connecting) {
    if (WiFi.status() == WL_CONNECTED) {
      staState = StaState::Connected;
      Serial.printf("WiFi nhà OK, IP: %s\n", WiFi.localIP().toString().c_str());
    } else if (millis() - staStart > STA_CONNECT_TIMEOUT_MS) {
      // Ngừng thử để AP không bị nhảy kênh liên tục làm rớt điện thoại
      WiFi.setAutoReconnect(false);
      WiFi.disconnect();
      staState = StaState::Failed;
      Serial.println(F("Không kết nối được WiFi nhà, chỉ dùng AP"));
    }
  } else if (staState == StaState::Connected && WiFi.status() != WL_CONNECTED) {
    staState = StaState::Connecting;   // mất kết nối, SDK tự kết nối lại
    staStart = millis();
  }
}

// ---------------- Captive portal ----------------
static bool isIp(const String &s) {
  for (char c : s) if (c != '.' && c != ':' && !isDigit(c)) return false;
  return s.length() > 0;
}

// Điện thoại kiểm tra Internet qua connectivitycheck.gstatic.com, captive.apple.com...
// -> chuyển hướng về trang điều khiển để điện thoại tự bật popup
static bool captiveRedirect() {
  String host = server.hostHeader();
  if (isIp(host) || host.startsWith(HOSTNAME)) return false;
  server.sendHeader("Location", String("http://") + AP_IP.toString() + "/", true);
  server.send(302, "text/plain", "");
  return true;
}

// ---------------- HTTP handlers ----------------
static void handleRoot() {
  if (captiveRedirect()) return;
  server.sendHeader("Cache-Control", "no-cache");
  server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

static void handleInfo() {
  JsonDocument doc;
  FSInfo fs;
  LittleFS.info(fs);
  static const char *staNames[] = {"off", "connecting", "connected", "failed"};
  doc["apSsid"] = AP_NAME;
  doc["apIp"] = WiFi.softAPIP().toString();
  doc["apClients"] = WiFi.softAPgetStationNum();
  doc["sta"] = staNames[(int)staState];
  doc["staSsid"] = staSsid;
  if (staState == StaState::Connected) {
    doc["staIp"] = WiFi.localIP().toString();
    doc["rssi"] = WiFi.RSSI();
  }
  doc["host"] = String(HOSTNAME) + ".local";
  doc["heap"] = ESP.getFreeHeap();
  doc["fsUsed"] = fs.usedBytes;
  doc["fsTotal"] = fs.totalBytes;
  doc["uptime"] = millis() / 1000;
  doc["version"] = _IRREMOTEESP8266_VERSION_STR;
  sendJson(doc);
}

static void handleButtons() {
  JsonDocument filter;
  filter["n"] = true;
  filter["d"] = true;
  filter["p"] = true;
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();
  Dir dir = LittleFS.openDir(BTN_DIR);
  while (dir.next()) {
    File f = dir.openFile("r");
    JsonDocument item;
    if (deserializeJson(item, f, DeserializationOption::Filter(filter)) == DeserializationError::Ok) {
      JsonObject o = arr.add<JsonObject>();
      o["id"] = dir.fileName().toInt();
      o["n"] = item["n"];
      o["d"] = item["d"];
      o["p"] = item["p"];
    }
    f.close();
  }
  sendJson(doc);
}

static void handleSend() {
  uint32_t id = server.arg("id").toInt();
  JsonDocument doc;
  if (!loadButton(id, doc)) return sendErr("Không tìm thấy nút", 404);
  IrCode c;
  codeFromJson(doc.as<JsonObjectConst>(), c);
  doc.clear();
  if (!sendCode(c)) return sendErr("Phát lệnh thất bại", 500);
  sendOk();
}

static void handleDelete() {
  uint32_t id = server.arg("id").toInt();
  if (!LittleFS.remove(btnPath(id))) return sendErr("Không tìm thấy nút", 404);
  sendOk();
}

static void handleRename() {
  uint32_t id = server.arg("id").toInt();
  JsonDocument doc;
  if (!loadButton(id, doc)) return sendErr("Không tìm thấy nút", 404);
  if (server.hasArg("n") && server.arg("n").length()) doc["n"] = server.arg("n");
  if (server.hasArg("d") && server.arg("d").length()) doc["d"] = server.arg("d");
  File f = LittleFS.open(btnPath(id), "w");
  if (!f) return sendErr("Lỗi ghi file", 500);
  serializeJson(doc, f);
  f.close();
  sendOk();
}

static void handleLearnStart() {
  learnMode = server.arg("mode") == "ac" ? LearnMode::AcDetect : LearnMode::Button;
  learnState = LearnState::Waiting;
  learnStart = millis();
  pending = IrCode();
  pendingDesc = "";
  irrecv.enableIRIn();
  irrecv.resume();
  sendOk();
}

static void handleLearnCancel() {
  learnStop();
  learnState = LearnState::Idle;
  sendOk();
}

static void handleLearnStatus() {
  JsonDocument doc;
  const char *st = "idle";
  switch (learnState) {
    case LearnState::Waiting:  st = "waiting"; break;
    case LearnState::Captured: st = "captured"; break;
    case LearnState::Timeout:  st = "timeout"; break;
    default: break;
  }
  doc["state"] = st;
  doc["mode"] = learnMode == LearnMode::AcDetect ? "ac" : "btn";
  if (learnState == LearnState::Waiting) {
    doc["remain"] = (LEARN_TIMEOUT_MS - (millis() - learnStart)) / 1000;
  }
  if (learnState == LearnState::Captured) {
    doc["proto"] = typeToString(pending.proto);
    doc["bits"] = pending.bits;
    doc["raw"] = pending.raw.size();
    doc["desc"] = pendingDesc;
    doc["acSupported"] = IRac::isProtocolSupported(pending.proto);
    if (learnMode == LearnMode::AcDetect) acToJson(doc["ac"].to<JsonObject>());
  }
  sendJson(doc);
}

static void handleLearnTest() {
  if (learnState != LearnState::Captured) return sendErr("Chưa có lệnh nào được học");
  sendCode(pending);
  sendOk();
}

static void handleLearnSave() {
  if (learnState != LearnState::Captured) return sendErr("Chưa có lệnh nào được học");
  String name = server.arg("n");
  String dev = server.arg("d");
  name.trim();
  dev.trim();
  if (!name.length()) return sendErr("Thiếu tên nút");
  if (!dev.length()) dev = "Khác";
  JsonDocument doc;
  doc["n"] = name;
  doc["d"] = dev;
  codeToJson(pending, doc.as<JsonObject>());
  uint32_t id = nextId++;
  File f = LittleFS.open(btnPath(id), "w");
  if (!f) return sendErr("Lỗi ghi file (bộ nhớ đầy?)", 500);
  serializeJson(doc, f);
  f.close();
  learnState = LearnState::Idle;
  pending = IrCode();
  JsonDocument res;
  res["ok"] = true;
  res["id"] = id;
  sendJson(res);
}

static void handleAcGet() {
  JsonDocument doc;
  acToJson(doc["state"].to<JsonObject>());
  JsonArray list = doc["protocols"].to<JsonArray>();
  for (int i = 1; i <= kLastDecodeType; i++) {
    decode_type_t p = (decode_type_t)i;
    if (IRac::isProtocolSupported(p)) list.add(typeToString(p));
  }
  sendJson(doc);
}

static void handleAcSet() {
  if (server.hasArg("proto")) {
    decode_type_t p = strToDecodeType(server.arg("proto").c_str());
    if (!IRac::isProtocolSupported(p)) return sendErr("Giao thức điều hòa không hỗ trợ");
    acState.protocol = p;
  }
  if (server.hasArg("model")) acState.model = server.arg("model").toInt();
  if (server.hasArg("power")) acState.power = server.arg("power") == "1";
  if (server.hasArg("mode")) acState.mode = (stdAc::opmode_t)server.arg("mode").toInt();
  if (server.hasArg("temp")) acState.degrees = constrain(server.arg("temp").toFloat(), 16, 32);
  if (server.hasArg("fan")) acState.fanspeed = (stdAc::fanspeed_t)server.arg("fan").toInt();
  if (server.hasArg("swing")) acState.swingv = (stdAc::swingv_t)server.arg("swing").toInt();
  if (server.hasArg("turbo")) acState.turbo = server.arg("turbo") == "1";
  if (server.hasArg("quiet")) acState.quiet = server.arg("quiet") == "1";
  if (server.hasArg("econo")) acState.econo = server.arg("econo") == "1";
  if (server.hasArg("light")) acState.light = server.arg("light") == "1";
  acSave();
  bool sent = true;
  if (server.arg("send") != "0") {
    acState.celsius = true;
    sent = ac.sendAc(acState, nullptr);
  }
  JsonDocument doc;
  doc["ok"] = sent;
  if (!sent) doc["error"] = "Không phát được lệnh điều hòa";
  acToJson(doc["state"].to<JsonObject>());
  sendJson(doc);
}

static void handleWifiScan() {
  int n = WiFi.scanComplete();
  JsonDocument doc;
  if (n == WIFI_SCAN_FAILED) {
    WiFi.scanNetworks(true);
    n = WIFI_SCAN_RUNNING;
  }
  if (n == WIFI_SCAN_RUNNING) {
    doc["scanning"] = true;
    return sendJson(doc);
  }
  JsonArray list = doc["list"].to<JsonArray>();
  for (int i = 0; i < n; i++) {
    String ssid = WiFi.SSID(i);
    if (!ssid.length()) continue;
    bool dup = false;   // gộp các AP cùng tên (mesh)
    for (JsonObject o : list) if (ssid == (const char *)o["ssid"]) { dup = true; break; }
    if (dup) continue;
    JsonObject o = list.add<JsonObject>();
    o["ssid"] = ssid;
    o["rssi"] = WiFi.RSSI(i);
    o["lock"] = WiFi.encryptionType(i) != ENC_TYPE_NONE;
  }
  WiFi.scanDelete();
  sendJson(doc);
}

static void handleWifiSave() {
  staSsid = server.arg("ssid");
  staPass = server.arg("pass");
  staSsid.trim();
  if (!staSsid.length()) return sendErr("Thiếu tên WiFi");
  JsonDocument doc;
  doc["ssid"] = staSsid;
  doc["pass"] = staPass;
  File f = LittleFS.open(WIFI_FILE, "w");
  if (!f) return sendErr("Lỗi ghi file", 500);
  serializeJson(doc, f);
  f.close();
  sendOk();
  staConnect();
}

static void handleWifiForget() {
  LittleFS.remove(WIFI_FILE);
  staSsid = staPass = "";
  WiFi.setAutoReconnect(false);
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  staState = StaState::Off;
  sendOk();
}

// ---------------- Vòng lặp học lệnh ----------------
static void learnLoop() {
  if (learnState != LearnState::Waiting) return;
  if (millis() - learnStart > LEARN_TIMEOUT_MS) {
    learnStop();
    learnState = LearnState::Timeout;
    return;
  }
  if (!irrecv.decode(&results)) return;
  // Bỏ qua mã lặp (giữ nút) và tín hiệu bị tràn bộ đệm
  if (results.repeat || results.overflow) {
    irrecv.resume();
    return;
  }
  codeFromResults(results, pending);
  pendingDesc = resultToHumanReadableBasic(&results);
  Serial.print(pendingDesc);

  if (learnMode == LearnMode::AcDetect && IRac::isProtocolSupported(results.decode_type)) {
    stdAc::state_t detected;
    if (IRAcUtils::decodeToState(&results, &detected)) {
      acState.protocol = detected.protocol;
      acState.model = detected.model;
      acState.power = detected.power;
      acState.mode = detected.mode;
      if (detected.degrees >= 16 && detected.degrees <= 32) acState.degrees = detected.degrees;
      acState.fanspeed = detected.fanspeed;
      acState.swingv = detected.swingv;
    } else {
      acState.protocol = results.decode_type;
    }
    acSave();
  }
  learnStop();
  learnState = LearnState::Captured;
}

// ---------------- WiFi ----------------
static void setupWifi() {
  WiFi.persistent(false);   // cấu hình lưu trong LittleFS, tránh SDK ghi flash
  WiFi.hostname(HOSTNAME);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_IP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_NAME);     // không mật khẩu
  Serial.printf("WiFi: \"%s\" (không mật khẩu) -> http://%s\n", AP_NAME, AP_IP.toString().c_str());

  // DNS trả mọi tên miền về ESP -> điện thoại tự bật trang điều khiển
  dns.setErrorReplyCode(DNSReplyCode::NoError);
  dns.start(53, "*", AP_IP);

  staLoad();
  staConnect();

  if (MDNS.begin(HOSTNAME)) {
    MDNS.addService("http", "tcp", 80);
    Serial.printf("Truy cập: http://%s.local\n", HOSTNAME);
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println(F("\n=== ESP8266 IR Remote ==="));

  irsend.begin();
  irrecv.setUnknownThreshold(MIN_UNKNOWN_SIZE);
  irrecv.setTolerance(kTolerance);

  if (!LittleFS.begin()) {
    Serial.println(F("LittleFS lỗi, đang format..."));
    LittleFS.format();
    LittleFS.begin();
  }
  if (!LittleFS.exists(BTN_DIR)) LittleFS.mkdir(BTN_DIR);
  scanNextId();
  acLoad();

  setupWifi();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/info", HTTP_GET, handleInfo);
  server.on("/api/buttons", HTTP_GET, handleButtons);
  server.on("/api/send", HTTP_POST, handleSend);
  server.on("/api/delete", HTTP_POST, handleDelete);
  server.on("/api/rename", HTTP_POST, handleRename);
  server.on("/api/learn/start", HTTP_POST, handleLearnStart);
  server.on("/api/learn/cancel", HTTP_POST, handleLearnCancel);
  server.on("/api/learn/status", HTTP_GET, handleLearnStatus);
  server.on("/api/learn/test", HTTP_POST, handleLearnTest);
  server.on("/api/learn/save", HTTP_POST, handleLearnSave);
  server.on("/api/ac", HTTP_GET, handleAcGet);
  server.on("/api/ac", HTTP_POST, handleAcSet);
  server.on("/api/wifi/scan", HTTP_GET, handleWifiScan);
  server.on("/api/wifi/save", HTTP_POST, handleWifiSave);
  server.on("/api/wifi/forget", HTTP_POST, handleWifiForget);
  server.onNotFound([]() {
    if (!captiveRedirect()) server.send(404, "text/plain", "Not found");
  });
  server.begin();
  Serial.println(F("Web server đã chạy"));
}

void loop() {
  dns.processNextRequest();
  server.handleClient();
  MDNS.update();
  wifiLoop();
  learnLoop();
}
