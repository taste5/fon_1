#ifndef __WIFI_AUDIO_H__
#define __WIFI_AUDIO_H__

#include "AudioTools.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include "AudioTools/Communication/AudioHttp.h"
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

    URLStreamBuffered urlStream;
    I2SStream         i2s;
    MP3DecoderHelix   mp3;
    EncodedAudioStream decoder{ &i2s, &mp3 };
    StreamCopy         copier{ decoder, urlStream };

    String              currentUrl;
    TaskHandle_t        taskHandle = nullptr;
    volatile bool       enabled    = false;
};

#endif
