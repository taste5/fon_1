#include "wifi_audio.h"
#include "config.h"
#include "secrets.h"

WiFiAudio::WiFiAudio()
    : urlStream(credentials.ssid, credentials.password)
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

    self->taskHandle = nullptr;
    vTaskDelete(nullptr);
}

void WiFiAudio::begin() {
    if (enabled) return;

    auto cfg = i2s.defaultConfig(TX_MODE);
    cfg.pin_bck  = I2S_BCLK_PIN;
    cfg.pin_ws   = I2S_LRC_PIN;
    cfg.pin_data = I2S_DIN_PIN;
    i2s.begin(cfg);

    decoder.begin();

    urlStream.setBufferSize(2048, 32);

    if (!urlStream.begin(currentUrl.c_str(), AUDIO_MIME_DEFAULT)) {
        Serial.printf("WiFiAudio: failed to open URL %s\n", currentUrl.c_str());
        decoder.end();
        i2s.end();
        return;
    }

    enabled = true;
    if (xTaskCreate(streamTask, "wifi_radio", 8192, this, 5, &taskHandle) != pdPASS) {
        enabled = false;
        urlStream.end();
        decoder.end();
        i2s.end();
        Serial.println("WiFiAudio: xTaskCreate failed");
        taskHandle = nullptr;
        return;
    }

    Serial.printf("WiFiAudio: streaming %s\n", currentUrl.c_str());
}

void WiFiAudio::stop() {
    if (!enabled && taskHandle == nullptr) return;

    enabled = false;

    unsigned long waited = millis();
    while (taskHandle != nullptr && millis() - waited < 2000) {
        delay(5);
    }
    if (taskHandle != nullptr) {
        vTaskDelete(taskHandle);
        taskHandle = nullptr;
    }

    urlStream.end();
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
