#pragma once

#include <filesystem>
#include <string>

namespace AssetManager
{
    // Returns the full path to the current running executable (.exe)
    std::filesystem::path getExecutablePath();

    // Returns the directory containing the current running executable
    std::filesystem::path getExecutableDir();

    // Returns the application assets directory (<exeDir>/assets or <exeDir>)
    std::filesystem::path getAssetsDir();

    // Resolves a relative asset path or file name against executable/asset candidate directories
    std::filesystem::path resolveAssetPath(const std::filesystem::path& relativeOrFileName);

    // Resolves a font file (e.g., "Cousine-Regular.ttf" or "Roboto-Medium.ttf")
    std::filesystem::path resolveFontPath(const std::string& fontName);

    // Resolves a sound file (e.g., "rain.wav" or "chime.wav")
    std::filesystem::path resolveSoundPath(const std::string& soundName);

    // Plays a sound by filename (from disk) or falls back to an embedded resource ID
    bool playSound(const std::string& soundName, int fallbackResourceId = 0);

    // Logging helpers writing to stderr and OutputDebugStringA
    void logError(const std::string& message);
    void logWarning(const std::string& message);
    void logInfo(const std::string& message);
}
