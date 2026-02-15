#pragma once

#define PROJECT_NAME "boost_button"

#include <GyverDBFile.h>
#include <SettingsGyver.h>
#include <LittleFS.h>
#include <WiFiConnector.h>

GyverDBFile db(&LittleFS, "/data.db");
SettingsGyver sett(PROJECT_NAME, &db);

sets::Logger logger(200);

DB_KEYS(
    kk,
    wifi_ssid,
    wifi_pass,
    close_ap,

    normal_speed,
    boost_speed,
    boost_duration // minutes
);

void init_wifi(){
    WiFiConnector.onConnect([]() {
        Serial.print("Connected: ");
        Serial.println(WiFi.localIP());
    });
    WiFiConnector.onError([]() {
        Serial.print("Error. Start AP: ");
        Serial.println(WiFi.softAPIP());
    });

    WiFiConnector.setName(PROJECT_NAME);
    WiFiConnector.closeAP(db[kk::close_ap]);
    WiFiConnector.connect(db[kk::wifi_ssid], db[kk::wifi_pass]);
}

void build_wifi(sets::Builder& b){
    sets::Menu m(b, "WiFi");
    {
        b.Input(kk::wifi_ssid, "SSID");
        b.Pass(kk::wifi_pass, "Pass", "");
        if (b.Switch(kk::close_ap, "Закрывать AP")) {
            WiFiConnector.closeAP(db[kk::close_ap]);
        }
        if (b.Button("Подключить")) {
            WiFiConnector.connect(db[kk::wifi_ssid], db[kk::wifi_pass]);
        }
    }
}

// ========== build ==========
static void build(sets::Builder& b) {
    build_wifi(b);

    b.Slider(kk::normal_speed, "Скорость вентилятора:", 0, 100, 5);
    b.Slider(kk::boost_speed, "Скорость Boost:", 0, 100, 5, "", nullptr, sets::Colors::Red);
    b.Slider(kk::boost_duration, "Время Boost (минуты):", 1, 60, 1, "", nullptr, sets::Colors::Red);

    b.Log(H(logger), logger);
    
}

// ========== update ==========
static void update(sets::Updater& u) {
    u.update(H(logger), logger);
}

// ========== begin ==========
void sett_begin() {
    // fs
#ifdef ESP32
    LittleFS.begin(true);
#else
    LittleFS.begin();
#endif

    // database
    db.begin();
    db.init(kk::wifi_ssid, "");
    db.init(kk::wifi_pass, "");
    db.init(kk::close_ap, true);
    db.init(kk::normal_speed, 40);
    db.init(kk::boost_speed, 80);
    db.init(kk::boost_duration, 10);

    // wifi
    init_wifi();

    // settings
    sett.begin();
    sett.onBuild(build);
    sett.onUpdate(update);
}

// ========== loop ==========
void sett_loop() {
    WiFiConnector.tick();
    sett.tick();
}