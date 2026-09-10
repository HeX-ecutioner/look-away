#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include "AssetManager.h"
#include <cstdio>
#include <vector>
#include <system_error>

namespace AssetManager
{
    void logError(const std::string& message)
    {
        std::string formatted = "[LookAway ERROR] " + message + "\n";
        fprintf(stderr, "%s", formatted.c_str());
        fflush(stderr);
        OutputDebugStringA(formatted.c_str());
    }

    void logWarning(const std::string& message)
    {
        std::string formatted = "[LookAway WARNING] " + message + "\n";
        fprintf(stderr, "%s", formatted.c_str());
        fflush(stderr);
        OutputDebugStringA(formatted.c_str());
    }

    void logInfo(const std::string& message)
    {
        std::string formatted = "[LookAway INFO] " + message + "\n";
        fprintf(stdout, "%s", formatted.c_str());
        fflush(stdout);
        OutputDebugStringA(formatted.c_str());
    }

    std::filesystem::path getExecutablePath()
    {
        static std::filesystem::path cachedPath = []() {
            std::wstring buffer;
            buffer.resize(MAX_PATH);
            DWORD length = GetModuleFileNameW(nullptr, &buffer[0], static_cast<DWORD>(buffer.size()));
            while (length == buffer.size())
            {
                buffer.resize(buffer.size() * 2);
                length = GetModuleFileNameW(nullptr, &buffer[0], static_cast<DWORD>(buffer.size()));
            }
            if (length == 0)
            {
                logError("GetModuleFileNameW failed with error code: " + std::to_string(GetLastError()));
                return std::filesystem::current_path() / "LookAway.exe";
            }
            buffer.resize(length);
            return std::filesystem::path(buffer);
        }();
        return cachedPath;
    }

    std::filesystem::path getExecutableDir()
    {
        static std::filesystem::path cachedDir = getExecutablePath().parent_path();
        return cachedDir;
    }

    std::filesystem::path getAssetsDir()
    {
        static std::filesystem::path cachedAssetsDir = []() {
            std::filesystem::path exeDir = getExecutableDir();
            std::error_code ec;

            // 1. Direct assets/ subfolder beside executable
            std::filesystem::path assetsSubdir = exeDir / "assets";
            if (std::filesystem::is_directory(assetsSubdir, ec))
                return assetsSubdir;

            // 2. Development tree fallbacks (e.g. when running from build/ or build/Release/)
            std::filesystem::path devAssets = exeDir.parent_path() / "assets";
            if (std::filesystem::is_directory(devAssets, ec))
                return devAssets;

            std::filesystem::path devAssetsMulti = exeDir.parent_path().parent_path() / "assets";
            if (std::filesystem::is_directory(devAssetsMulti, ec))
                return devAssetsMulti;

            return exeDir;
        }();
        return cachedAssetsDir;
    }

    std::filesystem::path resolveAssetPath(const std::filesystem::path& relativeOrFileName)
    {
        std::error_code ec;
        if (relativeOrFileName.is_absolute() && std::filesystem::exists(relativeOrFileName, ec))
            return relativeOrFileName;

        std::filesystem::path exeDir = getExecutableDir();
        std::vector<std::filesystem::path> candidates = {
            exeDir / relativeOrFileName,
            exeDir / "assets" / relativeOrFileName,
            exeDir / "fonts" / relativeOrFileName,
            exeDir.parent_path() / relativeOrFileName,
            exeDir.parent_path() / "assets" / relativeOrFileName,
            exeDir.parent_path() / "fonts" / relativeOrFileName,
            exeDir.parent_path().parent_path() / relativeOrFileName,
            exeDir.parent_path().parent_path() / "assets" / relativeOrFileName,
            exeDir.parent_path().parent_path() / "fonts" / relativeOrFileName,
        };

        for (const auto& candidate : candidates)
        {
            if (std::filesystem::exists(candidate, ec))
                return std::filesystem::weakly_canonical(candidate, ec);
        }

        return {};
    }

    std::filesystem::path resolveFontPath(const std::string& fontName)
    {
        std::filesystem::path exeDir = getExecutableDir();
        std::error_code ec;

        // Strip any leading directories (e.g. "fonts/") so we search purely by filename
        std::filesystem::path cleanName = std::filesystem::path(fontName).filename();

        std::vector<std::filesystem::path> candidateDirs = {
            exeDir / "fonts",
            exeDir / "assets" / "fonts",
            exeDir / "assets",
            exeDir,
            exeDir.parent_path() / "fonts",
            exeDir.parent_path() / "assets" / "fonts",
            exeDir.parent_path() / "external" / "imgui" / "misc" / "fonts",
            exeDir.parent_path().parent_path() / "fonts",
            exeDir.parent_path().parent_path() / "assets" / "fonts",
            exeDir.parent_path().parent_path() / "external" / "imgui" / "misc" / "fonts"
        };

        for (const auto& dir : candidateDirs)
        {
            std::filesystem::path candidate = dir / cleanName;
            if (std::filesystem::exists(candidate, ec))
                return std::filesystem::weakly_canonical(candidate, ec);
        }

        std::string triedList;
        for (const auto& dir : candidateDirs)
        {
            triedList += "\n  - " + (dir / cleanName).string();
        }
        logError("Failed to locate font '" + fontName + "'. Searched paths:" + triedList);
        return {};
    }

    std::filesystem::path resolveSoundPath(const std::string& soundName)
    {
        std::filesystem::path exeDir = getExecutableDir();
        std::error_code ec;

        std::filesystem::path cleanName = std::filesystem::path(soundName).filename();

        std::vector<std::filesystem::path> candidateDirs = {
            exeDir / "assets",
            exeDir / "sounds",
            exeDir,
            exeDir.parent_path() / "assets",
            exeDir.parent_path() / "sounds",
            exeDir.parent_path().parent_path() / "assets",
            exeDir.parent_path().parent_path() / "sounds"
        };

        for (const auto& dir : candidateDirs)
        {
            std::filesystem::path candidate = dir / cleanName;
            if (std::filesystem::exists(candidate, ec))
                return std::filesystem::weakly_canonical(candidate, ec);
        }

        return {};
    }

    bool playSound(const std::string& soundName, int fallbackResourceId)
    {
        std::filesystem::path soundPath = resolveSoundPath(soundName);
        if (!soundPath.empty())
        {
            if (PlaySoundW(soundPath.c_str(), NULL, SND_FILENAME | SND_ASYNC))
                return true;
            logWarning("PlaySoundW failed for audio file: " + soundPath.string() + " (Error code: " + std::to_string(GetLastError()) + ")");
        }

        if (fallbackResourceId > 0)
        {
            if (PlaySoundW(MAKEINTRESOURCEW(fallbackResourceId), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC))
                return true;
            logError("PlaySoundW failed for fallback resource ID " + std::to_string(fallbackResourceId) + " (Error code: " + std::to_string(GetLastError()) + ")");
        }
        else
        {
            logError("Failed to play sound '" + soundName + "': file not found on disk and no fallback resource ID provided.");
        }

        return false;
    }
}
