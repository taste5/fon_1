#ifndef __WIFI_AUDIO_H__
#define __WIFI_AUDIO_H__

#include "AudioTools.h"
#include "AudioTools/AudioCodecs/CodecAACHelix.h"
#include "AudioTools/Communication/HLSStream.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class WiFiAudio {
public:
    WiFiAudio();
    void begin();
    void stop();
    bool isEnabled() const { return enabled; }
    void setUrl(const char* url);
    const char* getUrl() const { return currentUrl.c_str(); }

private:
    static void streamTask(void* arg);

    HLSStream            hlsStream;
    I2SStream            i2s;
    AACDecoderHelix      aac;
    EncodedAudioStream   decoder{ &i2s, &aac };
    StreamCopy           copier{ decoder, hlsStream };

    String               currentUrl;
    TaskHandle_t         taskHandle = nullptr;
    volatile bool        enabled    = false;
};

#endif
