<img src="assets/icon.png?raw=true" width="400" style="display: block; margin: auto;"/>

# 👀 Look Away!

## 😅 Just kidding! Please read this.

**Look Away!** is a Windows utility that helps you prevent digital eye strain and *Computer Vision Syndrome (CVS)* while working on a screen for hours.

## 🤔 Have you ever thought:

> **How do I prevent digital eye strain and Computer Vision Syndrome (CVS) while working on a screen for hours?**

To achieve that, **Look Away!** is designed around:

- 🕒 **The 20-20-20 Rule** (Every 20 minutes, look at something 20 feet away, for 20 seconds)
- 🧘 **Minimalist Intervention** (A system-tray utility that stays out of your way until it's time)
- 🌑 **Immersive Overlays** (A pitch-black, fullscreen overlay to eliminate distractions during breaks)
- ✨ **Glassmorphism Aesthetics** (A premium, dark-mode UI with smooth fade transitions)
- 🚀 **Zero-Config Performance** (Lightweight C++ core with a tiny memory footprint)


## 🖥️ The Rest Experience

Look Away! isn't just a timer; it's a dedicated environment for your eyes to recover.

Instead of a jarring pop-up, the app gently fades in a fullscreen overlay:

1. **Automatic Timing**: Tracks your screen time silently in the background.
2. **Smooth Transitions**: Uses ease-in-out animations for non-disruptive entry.
3. **Motivational Guidance**: Displays randomized, gentle reminders to rest.
4. **Countdown Pulse**: A live 20-second timer keeps you informed of your progress.
5. **Auto-Resume**: Once the 20 seconds are up, the overlay fades out, returning you to your flow.

## 🧠 The Workflow:
1. Launch `LookAway.exe`.
2. Find the icon in your **System Tray**.
3. Work until the screen fades to black.
4. **Follow the instructions**: Look far away, blink, and breathe.
5. Return to work refreshed.

## 🛠️ Tech Stack

- **C++17**
- **CMake**
- **GLFW**
- **GLAD**
- **ImGui**
- **Win32 API**

## 🗂️ Project Structure

```yml
/include               <-- Architecture definitions
    App.h              # Main application orchestrator
    TimerManager.h     # Logic for work/break cycles
    TrayIcon.h         # Native Win32 system tray integration
    UI.h               # ImGui-based rendering interface
/src                   <-- Implementation
    main.cpp           # Windows entry point (WinMain)
    App.cpp            # Animation & workflow logic
    TimerManager.cpp   # Accurate second-tracking
    TrayIcon.cpp       # System tray & menu handling
    UI.cpp             # Custom rendering
/assets                <-- Branding
    app.ico            # High-resolution application icons
CMakeLists.txt         # Modern CMake build configuration
build.bat              # One-click Windows build script
```

### Why this architecture?
- **Separation of Concerns**: Logic is decoupled from the rendering engine.
- **Native Integration**: Uses Win32 APIs directly for the tray to minimize overhead.
- **Cross-Platform Potential**: While currently optimized for Windows, the core logic is designed for portability.

## 🛠️ Build Instructions

Ensure you have **CMake** and a **C++17** compatible compiler installed.

1. **Clone the repository**:
   ```bash
   git clone https://github.com/HeX-ecutioner/look-away.git
   cd look-away
   ```

2. **Initialize Submodules** (for GLFW, Glad, and ImGui):
   ```bash
   git submodule update --init --recursive
   ```

3. **Build using the script**:
   ```powershell
   ./build.bat
   ```

4. **Run**:
   Find `LookAway.exe` in the `build` folder.

## 📌 Core Features

### 🕒 Smart Timer Logic
- Tracks active work time accurately.
- Resets break count upon completion.
- Debug mode available for rapid testing (`-DDEBUG_TIMER=ON`).

### 🎨 Minimalist UI
- **Dark Mode**: Optimized for reduced light emission.
- **Anti-Distraction**: Hides the mouse cursor during breaks.
- **Smooth Animations**: 600ms ease-in-out fade durations.

### 🍱 System Tray Integration
- **Right-click menu**: Force a break or quit the app.
- **Silent Background**: No taskbar icon, only tray presence.

## ⚖️ License

This project is licensed under the ***[MIT License](LICENSE)***

## 💡 Pro Tip for Healthy Eyes

When the overlay appears, don't just close your eyes. **Look at something at least 20 feet (6 meters) away.** This allows the ciliary muscles in your eyes to relax from the constant "zoom" required by screen reading.

## 📝 Final Notes

This repository is designed to be:

- **Quiet**
- **Effective**
- **Focused**
- **Lightweight**

There is no data collection. There are no ads. Just you and your health.

### *“The screen will wait. Your eyes won't.” Happy resting!*