#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cwchar>
#include "App.h"

namespace {

class SingleInstanceGuard
{
public:
    enum class Status
    {
        FirstInstance,
        AlreadyRunning,
        Error
    };

    explicit SingleInstanceGuard(const wchar_t* mutexName)
    {
        m_mutex = CreateMutexW(nullptr, TRUE, mutexName);
        if (!m_mutex)
        {
            m_status = Status::Error;
            m_lastError = GetLastError();
            return;
        }

        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            m_status = Status::AlreadyRunning;
            return;
        }

        m_status = Status::FirstInstance;
    }

    ~SingleInstanceGuard()
    {
        if (m_mutex)
        {
            if (m_status == Status::FirstInstance)
            {
                ReleaseMutex(m_mutex);
            }
            CloseHandle(m_mutex);
            m_mutex = nullptr;
        }
    }

    SingleInstanceGuard(const SingleInstanceGuard&) = delete;
    SingleInstanceGuard& operator=(const SingleInstanceGuard&) = delete;

    Status getStatus() const { return m_status; }
    DWORD getLastError() const { return m_lastError; }

private:
    HANDLE m_mutex = nullptr;
    Status m_status = Status::Error;
    DWORD m_lastError = 0;
};

} // anonymous namespace

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) // Uses WinMain so no console window appears when the .exe is launched
{
    SingleInstanceGuard guard(L"Local\\LookAway_SingleInstance_Mutex");

    if (guard.getStatus() == SingleInstanceGuard::Status::AlreadyRunning)
    {
        // Another instance is already running; exit immediately without initializing subsystems
        return 0;
    }

    if (guard.getStatus() == SingleInstanceGuard::Status::Error)
    {
        wchar_t errMsg[256];
        swprintf(errMsg, sizeof(errMsg) / sizeof(wchar_t),
                 L"Failed to initialize application instance guard (Error code: %lu).",
                 guard.getLastError());
        MessageBoxW(nullptr, errMsg, L"Look Away! Error", MB_ICONERROR | MB_OK);
        return -1;
    }

    App app;

    if (!app.init(hInstance))
        return -1;

    app.run();
    app.shutdown();

    return 0;
}
