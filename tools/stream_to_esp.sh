#!/usr/bin/env bash
set -euo pipefail

DEVICE="${DEVICE:-BlackHole 16ch}"
PORT="${PORT:-9090}"
SAMPLE_RATE="${SAMPLE_RATE:-22050}"
BITRATE="${BITRATE:-24k}"

OUT_DIR="${OUT_DIR:-/tmp/fon_stream}"
PLAYLIST="${OUT_DIR}/stream.m3u8"

mkdir -p "${OUT_DIR}"
rm -f "${OUT_DIR}"/*.ts "${OUT_DIR}"/*.m3u8 2>/dev/null || true

MAC_IP="$(ipconfig getifaddr en0 2>/dev/null || echo '<your-mac-ip>')"

echo "[stream_to_esp] capture: '${DEVICE}' -> AAC ${BITRATE} mono ${SAMPLE_RATE} Hz"
echo "[stream_to_esp] HLS dir: ${OUT_DIR}"
echo "[stream_to_esp] URL:     http://${MAC_IP}:${PORT}/stream.m3u8"
echo "[stream_to_esp] press Ctrl+C to stop both ffmpeg and the python http server"
echo

python3 -m http.server "${PORT}" --directory "${OUT_DIR}" >/dev/null 2>&1 &
HTTP_PID=$!

cleanup() {
    if kill -0 "${HTTP_PID}" 2>/dev/null; then
        kill "${HTTP_PID}"
        wait "${HTTP_PID}" 2>/dev/null || true
    fi
}
trap cleanup EXIT INT TERM

exec ffmpeg -hide_banner -loglevel info \
    -f avfoundation -i ":${DEVICE}" \
    -ac 1 -ar "${SAMPLE_RATE}" \
    -c:a aac -b:a "${BITRATE}" \
    -f hls -hls_time 2 -hls_list_size 10 -hls_flags delete_segments \
    "${PLAYLIST}"
