# `tools/` — host-side helpers

Stream macOS system audio to the fon ESP32 over WiFi as **HLS (HTTP Live Streaming) with AAC**.

```
macOS apps audio
    -> BlackHole (virtual audio device)
        -> ffmpeg (AAC encode + HLS muxer to /tmp/fon_stream/)
            -> python3 -m http.server  (serves the m3u8 + .ts segments)
                -> ESP32 (HLSStream + AACDecoderHelix + I2S DAC)
```

We use HLS + AAC instead of raw MP3-over-HTTP because it gave us cleaner audio with fewer dropouts in practice: AAC is more efficient at low bitrates, and HLS's segmented design tolerates jitter much better than a continuous MP3 stream.

`stream_to_esp.sh` runs **two** processes:

1. `python3 -m http.server` in the background, serving the HLS directory.
2. `ffmpeg` in the foreground, capturing audio and writing `stream.m3u8` + segment files into that directory.

The script ties their lifetimes together — Ctrl+C stops both.

## One-time setup

### 1. Install BlackHole and ffmpeg

```bash
brew install blackhole-16ch
brew install ffmpeg
```

(`blackhole-2ch` works too — set `DEVICE="BlackHole 2ch"` when running the script.)
Python 3 is already on macOS.

### 2. Route system audio through BlackHole + speakers

In **Audio MIDI Setup** (`/Applications/Utilities/Audio MIDI Setup.app`):

1. Click `+` (bottom-left) → *Create Multi-Output Device*.
2. Tick **BlackHole 16ch** *and* your normal output (built-in speakers / headphones).
3. Tick *Drift Correction* on BlackHole.
4. In **System Settings → Sound → Output**, select the new Multi-Output Device.

Now everything you play is heard locally *and* captured by BlackHole.

### 3. Tell the ESP where to find the stream

Find your Mac's LAN IP:

```bash
ipconfig getifaddr en0   # WiFi
ipconfig getifaddr en1   # Ethernet (if applicable)
```

Add this define to `include/secrets.h` (gitignored, alongside your `credentials` struct):

```c
#define AUDIO_DEFAULT_URL "http://192.168.x.x:9090/stream.m3u8"
```

Replace `192.168.x.x` with the IP from the previous step. Reflash the ESP after editing.

The fallback in `include/config.h` is `http://localhost:9090/stream.m3u8` — clearly a placeholder, never useful at runtime, but lets fresh checkouts build.

## Daily use

### Start the stream on your Mac

```bash
./tools/stream_to_esp.sh
```

The script:
- creates `/tmp/fon_stream/` and clears any old `.ts`/`.m3u8` files,
- starts `python3 -m http.server 9090 --directory /tmp/fon_stream` in the background,
- runs ffmpeg in the foreground with the HLS muxer.

It prints the URL it's serving on. Override defaults via env vars if needed:

```bash
DEVICE="BlackHole 2ch" PORT=8000 BITRATE=64k SAMPLE_RATE=44100 ./tools/stream_to_esp.sh
```

### Tell the ESP to play it

Send these OSC messages to the fon (`<fon-ip>:9000` per `LOCAL_PORT` in `config.h`):

```
/fon/audio/enable i 1
```

The fon connects to whatever URL is in `AUDIO_DEFAULT_URL` (i.e. your Mac). To override at runtime without reflashing:

```
/fon/audio/url s "http://<other-host>:9090/stream.m3u8"
/fon/audio/enable i 1
```

To stop the fon's playback:

```
/fon/audio/enable i 0
```

To stop the script: `Ctrl+C` in the terminal — both ffmpeg and the python http server are killed.

## Troubleshooting

### Find BlackHole's exact device name

avfoundation device names sometimes change between macOS versions. List what ffmpeg sees:

```bash
ffmpeg -hide_banner -f avfoundation -list_devices true -i ""
```

Look under `[AVFoundation indev …] AVFoundation audio devices:`. Match the name (e.g. `BlackHole 16ch`) and pass it via `DEVICE=...`.

### Confirm the playlist is being served

In another terminal while the script is running:

```bash
curl -s "http://localhost:9090/stream.m3u8" | head
```

You should see lines like `#EXTM3U`, `#EXT-X-VERSION:3`, then `streamN.ts` filenames. If `curl` hangs or returns 404, ffmpeg hasn't finished writing the first segment yet (initial ~2 s delay is normal with `hls_time 2`).

### "Connection refused" from the ESP

- Confirm the script printed a URL containing your real LAN IP (not `<your-mac-ip>`). If it didn't, your active interface isn't `en0` — find the right one with `ifconfig` and update the script.
- Mac firewall: System Settings → Network → Firewall — allow incoming connections for `python3` and `ffmpeg` (or temporarily disable for testing).
- Same WiFi network: the ESP and Mac must be on the same subnet.

### Crackles / dropouts

- The defaults (mono, 22050 Hz, 24 kbps AAC, 2 s segments) are deliberately conservative — robust on flaky WiFi at the cost of fidelity.
- If your WiFi is solid and you want better quality, try `BITRATE=64k SAMPLE_RATE=44100`.
- Check the ESP's serial output: `wifi_radio` lines print `bytes_5s` and `free_heap` every 5 s. A `bytes_5s` of zero means no segments are reaching the decoder.

## Known limitations

- **HTTP only.** No TLS — don't expose this beyond your LAN.
- **One ESP listener at a time** is the only practical assumption; technically the python http server is multi-client capable, but we haven't tested it.
- **Segment files accumulate briefly** — `delete_segments` keeps the live window short, but if ffmpeg is killed mid-segment, a stray `.ts` may remain in `/tmp/fon_stream/` until the next run cleans it.
