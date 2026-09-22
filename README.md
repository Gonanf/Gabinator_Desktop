<p align="center">
  <img src="assets/banner.png" alt="Gabinator_Desktop" width="100%">
</p>

<h1 align="center">Gabinator Desktop</h1>

<p align="center"><b>Screen sharing from Windows PC to Android phone via USB.</b></p>

<p align="center">
  <img alt="estado" src="https://img.shields.io/badge/estado-prototipo-FF9900">
  <img alt="lenguaje" src="https://img.shields.io/badge/C++-20-blue">
  <img alt="licencia" src="https://img.shields.io/badge/licencia-privado-lightgrey">
  <img alt="ultima actividad" src="https://img.shields.io/badge/ultima_actividad-2024--06-lightgrey">
</p>

---

## What it is

Gabinator Desktop captures the Windows desktop screen, compresses it to JPEG, and sends the image data to an Android device over USB using the Android Open Accessory (AOA) protocol. The Android side is handled by a separate project ([Gabinator_Android](https://github.com/Gonanf/Gabinator_Android)).

**In one sentence:** A Windows screen-to-phone relay over USB, built as a companion to Gabinator Android.

## State

| | |
|---|---|
| **State** | Prototype |
| **Last activity** | 2024-06 |
| **Can it be used today** | No — requires a specific Android phone configured for AOA, hardcoded paths to OpenCV and libusb on Windows, and the companion Android app |
| **What's missing** | TCP transport (stub only), cross-platform support, configuration UI, error recovery, tests, `.gitignore` |
| **Risks / known debt** | Hardcoded Windows paths in CMakeLists.txt (`C:/opencv`, `C:/libusb-1.0.27`), no error handling on most USB failures, no unit tests |

## Why it exists

Part of a course project ( ACT series of commits) that explores sending desktop screen data to a phone over USB, using Android's Open Accessory mode as the transport layer.

## Installation and usage

**Requirements:**
- Windows (uses Win32 API, GDI, `Ws2_32.lib`)
- OpenCV (installed at `C:/opencv/build`)
- libusb 1.0.27 (installed at `C:/libusb-1.0.27`)
- CMake 3.28+
- An Android phone supporting AOA, with [Gabinator_Android](https://github.com/Gonanf/Gabinator_Android) installed

```bash
cmake -B build
cmake --build build
```

```bash
# Run — connects to AOA-compatible phone and starts streaming screenshots
./build/Debug/Gabinator_Desktop.exe
```

**Settings:** The app reads `settings.txt` for transport method (`Metodo:USB` or `Metodo:TCP`) and JPEG compression level (`Compresion:7`, range 1-10). If the file is missing, defaults are created.

## Stack

- **Language:** C++ (CMake 3.28)
- **Dependencies:** OpenCV (image capture + JPEG encoding), libusb 1.0.27 (USB/AOA communication)
- **Platform:** Windows only (Win32 API, GDI screen capture, `Ws2_32.lib`)

## Architecture

```
CaptureScreen()  →  JPEG encode (OpenCV)  →  USB bulk transfer (libusb AOA)  →  Android device
       ↑
  Win32 GDI
  (desktop + cursor)
```

- **`main.cpp`** — Entry point. Polls for USB connection, sends captures in a loop.
- **`gabinator.hpp`** — All logic in one header:
  - `Settings` — reads/writes `settings.txt` for method and compression
  - `CaptureScreen()` — GDI-based desktop screenshot with cursor overlay, returns JPEG buffer
  - `USB` — AOA protocol handshake, device discovery, bulk transfer
  - `TCP` — Empty stub (`//TODO: Terminar esto`)

## Repo structure

```
src/
  main.cpp          # entry point
  gabinator.hpp     # all implementation (USB, capture, settings)
CMakeLists.txt      # build config (hardcoded Windows paths)
docs/
  overview.md       # auto-generated overview
```

## Roadmap

- [ ] Implement TCP transport (currently a stub)
- [ ] Make CMakeLists.txt portable (detect OpenCV/libusb via `find_package` or env vars)
- [ ] Add `.gitignore` (build artifacts are committed)
- [ ] Error recovery for USB disconnections
- [ ] Cross-platform support or at least document Windows-only limitation
- [x] USB/AOA screen capture and transfer
- [x] Settings file support
- [x] JPEG compression control

## Notes

- The Android companion app lives at [Gonanf/Gabinator_Android](https://github.com/Gonanf/Gabinator_Android).
- AOA vendor/product IDs are hardcoded for Google's AOA spec (`0x18D1:0x2D00/0x2D01`).
- The `build/` directory is committed to the repo (should be gitignored).

## License

Private — no license file. Not open source.
