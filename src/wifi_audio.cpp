#include "wifi_audio.h"
#include "config.h"
#include "secrets.h"

WiFiAudio::WiFiAudio()
    : hlsStream(credentials.ssid, credentials.password)
    , currentUrl(AUDIO_DEFAULT_URL) {}

void WiFiAudio::streamTask(void* arg) {
    auto* self = static_cast<WiFiAudio*>(arg);
    uint32_t lastReport = millis();
    uint32_t totalBytes = 0;

    while (self->enabled) {
        size_t copied = self->copier.copy();
        totalBytes += copied;

        uint32_t now = millis();
        if (now - lastReport >= 5000) {
            Serial.printf(
                "{\"dbg\":\"wifi_radio\",\"bytes_5s\":%u,\"free_heap\":%u}\n",
                (unsigned)totalBytes, (unsigned)ESP.getFreeHeap());
            totalBytes = 0;
            lastReport = now;
        }

        if (copied == 0) {
            vTaskDelay(pdMS_TO_TICKS(2));
        }
    }
    vTaskDelete(NULL);
}

void WiFiAudio::begin() {
    if (enabled) return;

    auto cfg = i2s.defaultConfig(TX_MODE);
    cfg.pin_bck  = I2S_BCLK_PIN;
    cfg.pin_ws   = I2S_LRC_PIN;
    cfg.pin_data = I2S_DIN_PIN;
    i2s.begin(cfg);

    decoder.begin();

    if (!hlsStream.begin(currentUrl.c_str())) {
        Serial.printf("WiFiAudio: failed to open HLS URL %s\n", currentUrl.c_str());
        decoder.end();
        i2s.end();
        return;
    }

    enabled = true;
    xTaskCreate(streamTask, "wifi_radio", 8192, this, 5, &taskHandle);
    Serial.printf("WiFiAudio: streaming %s\n", currentUrl.c_str());
}

void WiFiAudio::stop() {
    if (!enabled) return;
    enabled = false;

    if (taskHandle) {
        vTaskDelete(taskHandle);
        taskHandle = nullptr;
    }

    hlsStream.end();
    decoder.end();
    i2s.end();
    Serial.println("WiFiAudio: stopped");
}

void WiFiAudio::setUrl(const char* url) {
    if (!url || !*url) return;
    currentUrl = url;
    if (enabled) {
        stop();
        begin();
    }
}
