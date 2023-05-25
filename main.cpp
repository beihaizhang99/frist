#include "mainwindow.h"
#include <QApplication>
#include <shlwapi.h>
#include <QLocalServer>
#include <QLocalSocket>

bool isRunAsAdmin()
{
    BOOL bElevated = FALSE;
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        TOKEN_ELEVATION elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &cbSize))
        {
            bElevated = elevation.TokenIsElevated;
        }
    }
    if (hToken)
    {
        CloseHandle(hToken);
    }
    return bElevated;
}

bool runAsAdmin(HWND hwnd, LPCWSTR lpFile, LPCWSTR lpParameters)
{
    SHELLEXECUTEINFO sei = { sizeof(sei), 0 };
    sei.lpVerb = isRunAsAdmin() ? NULL : L"runas";
    sei.hwnd = hwnd;
    sei.lpFile = lpFile;
    sei.lpParameters = lpParameters;
    sei.nShow = SW_NORMAL;
    return ShellExecuteEx(&sei) ? true : false;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //// 1. 创建本地服务器对象
    //QLocalServer server;
    //server.listen("Time_Control");
    //// 2. 检查是否有其他实例正在运行
    //QLocalSocket socket;
    //socket.connectToServer("Time_Control");
    //if (socket.waitForConnected(500)) {
    //    // 发送消息给其他实例并退出应用程序
    //    socket.write("Message from new instance\n");
    //    socket.waitForBytesWritten();
    //    return 0;
    //}

    if (!isRunAsAdmin())
    {
        // 获取程序路径
        wchar_t szModulePath[MAX_PATH];
        GetModuleFileNameW(NULL, szModulePath, MAX_PATH);
        // 以管理员权限重新启动程序
        if (!runAsAdmin(NULL, szModulePath, NULL))
        {
            MessageBox(NULL, L"启动失败！", L"错误", MB_OK | MB_ICONERROR);
            return 1;
        }
        return 0;
    }

    // 3. 创建主窗口
    MainWindow w;
    w.show();
    // 4. 在主窗口关闭时关闭本地服务器对象
    //QObject::connect(&w, &MainWindow::destroyed, [&](){
    //    server.close();
    //});

    return a.exec();
}
