# yt-dlp-gui
A simple, cross-platform GUI extension for YouTube downloader yt-dlp, built with C++ and Qt 6.

## Features
* Download single videos or entire playlists.
* Choose between downloading Audio only (MP3, M4A, FLAC, etc.) or Video (MP4, MKV, WebM, etc.).
* Real-time detection of required binaries (`yt-dlp`, `ffmpeg`, and JS Runtime).
* Built-in rate-limiting to avoid IP bans.
* Automatic dark/light mode switching based on your OS settings.
* English and Czech language support

---

## Prerequisites
For this app to work, you **must** have these three tools installed on your system:
1. **yt-dlp** (The core download tool)
2. **ffmpeg** (Used for merging video/audio streams and converting media formats)
3. **JavaScript Runtime** (Node.js or Deno, required by yt-dlp to solve YouTube JS challenges)
The app will search for these binaries automatically in your system's `PATH`.
For more info, check [yt-dlp](https://github.com/yt-dlp/yt-dlp) and [ffmpeg](https://ffmpeg.org).

---

### Windows
#### WinGet
The easiest way to install the dependencies on Windows is using [WinGet](https://learn.microsoft.com/en-us/windows/package-manager/winget/)
```powershell
winget install yt-dlp.yt-dlp Gyan.FFmpeg OpenJS.NodeJS
```
#### Direct download
1. Download `yt-dlp.exe` from the [yt-dlp releases page](https://github.com/yt-dlp/yt-dlp/releases).
2. Download `ffmpeg.exe` from the [official FFmpeg site](https://ffmpeg.org/download.html).
3. Download `Node.js` from the [official Node.js site](https://nodejs.org/).
4. Place all `.exe` files into a folder that is in your system's `PATH`, or put them in the same directory as the `yt-dlp-gui` executable.

---

### macOS
#### Homebrew
The easiest way to install the dependencies on macOS is using [Homebrew](https://brew.sh/):
```bash
brew install yt-dlp ffmpeg node
```

---

### Linux
You can install the dependencies using your package manager
#### Ubuntu/Debian/Mint
```bash
sudo apt update
sudo apt install yt-dlp ffmpeg nodejs
```
#### Arch/Manjaro
```bash
sudo pacman -Syu ffmpeg yt-dlp nodejs
```
#### Fedora/Red Hat
```bash
sudo dnf install ffmpeg yt-dlp nodejs
```

---

## Installation
Go to the **[Releases](../../releases)** page of this repository and download the package for your operating system:
* Windows: Download the `yt-dlp-gui-Windows-x64.zip` file, extract it and run `yt-dlp-gui.exe`.
* macOS: Download the `yt-dlp-gui-macOS.dmg` file, open it and drag the app to your Applications folder.
    > **Important Note for macOS Users:**  
    > Because this is an unsigned open-source application, macOS Gatekeeper might show an error saying "yt-dlp-gui.app is damaged and can't be opened." To fix this, move the app to your Applications folder from the `.dmg` file, open Terminal, and run the following command to clear the quarantine flag:
    >```bash
    >xattr -cr /Applications/yt-dlp-gui.app
    >```
* Linux: Download the `yt-dlp-gui-Linux-x64.AppImage` file, make it executable, and run it:
```
chmod +x yt-dlp-gui-Linux-x64.AppImage
./yt-dlp-gui-Linux-x64.AppImage
```

---

## Building from source
### Requirements
* **C++17** compatible compiler (MSVC, GCC, or Clang)
* **CMake** (v3.16 or higher)
* **Qt 6** (Core, Widgets, Network)
### Building
Clone the repository and run:
```
mkdir build
cd build
cmake ..
cmake --build .
```