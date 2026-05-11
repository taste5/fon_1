# Applies arduino-audio-tools URLStream.h fix for Arduino-ESP32 2.x (no
# WiFiClient::setConnectionTimeout) while preserving it on ESP32 core 3+.
Import("env")

from pathlib import Path

_SECURE_OLD = """#ifdef ESP32
        client_secure->setConnectionTimeout(client_timeout);
        client_secure->setHandshakeTimeout(handshake_timeout);
#endif"""
_SECURE_NEW = """#ifdef ESP32
#if ESP_ARDUINO_VERSION_MAJOR >= 3
        client_secure->setConnectionTimeout(client_timeout);
#endif
        client_secure->setHandshakeTimeout(handshake_timeout);
#endif"""
_INSECURE_OLD = """#ifdef ESP32
      client_insecure->setConnectionTimeout(client_timeout);
#endif"""
_INSECURE_NEW = """#ifdef ESP32
#if ESP_ARDUINO_VERSION_MAJOR >= 3
      client_insecure->setConnectionTimeout(client_timeout);
#endif
#endif"""

_PATCH_MARK = "#if ESP_ARDUINO_VERSION_MAJOR >= 3\n        client_secure->setConnectionTimeout"


def _main():
    proj = Path(env["PROJECT_DIR"])
    for urlstream in proj.glob(
        ".pio/libdeps/*/audio-tools/src/AudioTools/Communication/HTTP/URLStream.h"
    ):
        text = urlstream.read_text(encoding="utf-8")
        if _PATCH_MARK in text:
            continue
        new_text = text.replace(_SECURE_OLD, _SECURE_NEW).replace(_INSECURE_OLD, _INSECURE_NEW)
        if new_text != text:
            urlstream.write_text(new_text, encoding="utf-8")
            print("patched:", urlstream)


_main()
