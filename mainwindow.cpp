#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPushButton>
#include <QDebug>
#include <QObject>
#include <Windows.h>
#include <QMessageBox>
#include <time.h>
#include <ctime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    SetSystemTray();
    InitTextEdit();
    InitCtrlMenu();
    InitCtrlButton();
    QObject::connect(ui->ButtonModifyTime, &QPushButton::clicked, [=]()
    {
        ModifyTime();
    });
    QObject::connect(ui->ButtonRecoverTime, &QPushButton::clicked, [=]()
    {
        StartWindowsTimeService();
    });
    QObject::connect(ui->ButtonTimeReminder, &QPushButton::clicked, [=]()
    {
        if (!m_timeReminder)
        {
            m_timeReminder = new TimeReminder(parent);
        }
    });
    QObject::connect(ui->ButtonTimeReminderCancel, &QPushButton::clicked, [=]()
    {
        if (m_timeReminder)
        {
            delete m_timeReminder;
            m_timeReminder = NULL;
        }
    });
    QObject::connect(ui->ButtonStamp2Visual, &QPushButton::clicked, [=]()
    {
        Stamp2Visual();
    });
    QObject::connect(ui->ButtonVisual2Stamp, &QPushButton::clicked, [=]()
    {
        Visual2Stamp();
    });
    RegisterHotKey((HWND)winId(), 1, MOD_CONTROL | MOD_SHIFT, 'T');

}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_timeReminder;
    delete m_trayIcon;
    delete m_restoreAction;
    delete m_trayMenu;
    delete m_quitAction;
    qDebug() << "~~~~~~~~~~~~~MainWindow" ;
}

void MainWindow::InitTextEdit()
{

    // 获取当前本地时间
    SYSTEMTIME localTime;
    GetLocalTime(&localTime);
    QString stYear = QString::number(localTime.wYear);
    QString stMonth = QString::number(localTime.wMonth);
    QString stDay = QString::number(localTime.wDay);
    ui->textEditYear->setText(stYear);
    ui->textEditMonth->setText(stMonth);
    ui->textEditDay->setText(stDay);
    ui->textEditHour->setText("23");
    ui->textEditMin->setText("59");
    ui->textEditSecond->setText("30");
    time_t nowStamp = time(nullptr);
    tm beijingTime = *localtime(&nowStamp);
    char buffer[80];
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", &beijingTime);
    ui->textTimeStamp->setText(QString::number(nowStamp));
    ui->textTimeBeijing->setText(buffer);

    ui->textEditYear->setAlignment(Qt::AlignHCenter);
    ui->textEditYear->moveCursor(QTextCursor::End);
    ui->textEditMonth->setAlignment(Qt::AlignHCenter);
    ui->textEditDay->setAlignment(Qt::AlignHCenter);
    ui->textEditHour->setAlignment(Qt::AlignHCenter);
    ui->textEditMin->setAlignment(Qt::AlignHCenter);
    ui->textEditSecond->setAlignment(Qt::AlignHCenter);
    ui->textTimeStamp->setAlignment(Qt::AlignHCenter);
    ui->textTimeBeijing->setAlignment(Qt::AlignHCenter);
}

void MainWindow::InitCtrlMenu()
{
    m_ctrlMenu = new QMenu("ctrlMenu");
    m_setCtrlBTitle = m_ctrlMenu->addAction("设置文本");
    m_setCtrlBPath = m_ctrlMenu->addAction("设置路径");
    connect(m_setCtrlBTitle, &QAction::triggered, this, [=]()
    {
        bool ok;
        QString text = QInputDialog::getText(this, "输入文本", "请输入文本:", QLineEdit::Normal, "", &ok);
        if (ok && !text.isEmpty()) {
            // 文本输入成功，执行相应操作
            QDir::setCurrent(m_defaultPath);
            m_curCtrlButton->setText(text);
            QSettings settings("buttonCfg.ini", QSettings::IniFormat);
            settings.setIniCodec("UTF-8");
            settings.setValue(m_curCtrlButton->objectName() + "Title", text);
        }
    });
    connect(m_setCtrlBPath, &QAction::triggered, this, [=]()
    {

        QString filePath = QFileDialog::getOpenFileName(this, "选择文件", m_curCtrlButton->toolTip(), "所有文件(*.*)");
        if (!filePath.isEmpty()) {
            // 文件选择成功，执行相应操作
            QDir::setCurrent(m_defaultPath);
            m_curCtrlButton->setToolTip(filePath);
            QSettings settings("buttonCfg.ini", QSettings::IniFormat);
            settings.setIniCodec("UTF-8");
            settings.setValue(m_curCtrlButton->objectName() + "Path", filePath);
        }
    });
}

void MainWindow::InitCtrlButton()
{
    m_defaultPath = QDir::currentPath();
    QList<QPushButton *> buttonList = ui->FrameControl->findChildren<QPushButton*>();
    foreach (QPushButton* button, buttonList) {
        QString stButtonName = button->objectName();
        QSettings settings("buttonCfg.ini", QSettings::IniFormat);
        settings.setIniCodec("UTF-8");
        QString title = settings.value(stButtonName + "Title").toString();
        QString path = settings.value(stButtonName + "Path").toString();
        button->setText(title);
        button->setToolTip(path);
        QObject::connect(button, &QPushButton::clicked, [=]()
        {
            QString stPath = button->toolTip();
            if (stPath != "")
            {
                QFileInfo fileInfo(stPath);
                QString folderPath = fileInfo.path();
                QString fileName = fileInfo.fileName();
                QDir::setCurrent(folderPath);
                QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
            }
        });
        QObject::connect(button, &QPushButton::customContextMenuRequested, [=]()
        {
            m_curCtrlButton = button;
            m_ctrlMenu->exec(QCursor::pos());
        });
    }
}

// 停止并禁用Windows Time服务
void MainWindow::StopWindowsTimeService()
{
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM) {
        SC_HANDLE hService = OpenService(hSCM, L"W32Time", SERVICE_STOP | SERVICE_QUERY_STATUS| SERVICE_CHANGE_CONFIG);
        if (hService) {
            SERVICE_STATUS status;
            ControlService(hService, SERVICE_CONTROL_STOP, &status);
            ChangeServiceConfig(hService, SERVICE_NO_CHANGE, SERVICE_DISABLED, SERVICE_NO_CHANGE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
            CloseServiceHandle(hService);
        }
        CloseServiceHandle(hSCM);
    }
}

// 启动Windows Time服务
void MainWindow::StartWindowsTimeService()
{
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM) {
        SC_HANDLE hService = OpenService(hSCM, L"W32Time", SERVICE_ALL_ACCESS);
        if (hService) {
            // 修改服务启动类型为自动
            ChangeServiceConfig(hService, SERVICE_NO_CHANGE, SERVICE_AUTO_START, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
            // 启动服务
            StartService(hService, NULL, NULL);
            CloseServiceHandle(hService);
        }
        CloseServiceHandle(hSCM);
    }
}

void MainWindow::ModifyTime()
{
    bool ok1, ok2, ok3, ok4, ok5, ok6;
    int year = ui->textEditYear->toPlainText().toInt(&ok1);
    int month = ui->textEditMonth->toPlainText().toInt(&ok2);
    int day = ui->textEditDay->toPlainText().toInt(&ok3);
    int hour = ui->textEditHour->toPlainText().toInt(&ok4);
    int minute = ui->textEditMin->toPlainText().toInt(&ok5);
    int second = ui->textEditSecond->toPlainText().toInt(&ok6);
    if (ok1&&ok2&&ok3&&ok4&&ok5&&ok6)
    {

        SYSTEMTIME st;
        st.wYear = year;
        st.wMonth = month;
        st.wDay = day;
        st.wHour = hour;
        st.wMinute = minute;
        st.wSecond = second;
        TIME_ZONE_INFORMATION timeZoneInfo;
        GetTimeZoneInformation(&timeZoneInfo);
        SYSTEMTIME utcTime;
        TzSpecificLocalTimeToSystemTime(&timeZoneInfo, &st, &utcTime);
        if (!SetSystemTime(&utcTime))
        {
            DWORD err = GetLastError();
            QMessageBox::information(this, "", QString("设置失败，请检查时间 err = %1").arg(err));
            return;
        }
        StopWindowsTimeService();
    }
    else
    {
         QMessageBox::critical(this, "", "请输入数字");
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 将窗口最小化时隐藏到系统托盘中
    hide();
    event->ignore();

}

void MainWindow::SetSystemTray()
{
    // 创建系统托盘
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QIcon("clock.ico"));

    // 创建托盘菜单
    m_trayMenu = new QMenu(this);
    m_restoreAction = new QAction(tr("显示"), this);
    m_quitAction = new QAction(tr("退出"), this);
    m_trayMenu->addAction(m_restoreAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(m_quitAction);

    // 将托盘菜单设置到系统托盘中
    m_trayIcon->setContextMenu(m_trayMenu);

    // 设置系统托盘图标的提示信息
    m_trayIcon->setToolTip("Time_Control");

    // 显示系统托盘
    m_trayIcon->show();
    // 双击系统托盘图标还原窗口
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [=](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick) {
            showNormal();
        }
    });
    // 从系统托盘还原窗口
    connect(m_restoreAction, &QAction::triggered, [=](){
        showNormal();
    });

    // 退出应用程序
    connect(m_quitAction, &QAction::triggered, qApp, [=](){
        QApplication::quit();
    });
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(eventType);
    Q_UNUSED(result);

    MSG *msg = static_cast<MSG*>(message);
    if (msg->message == WM_HOTKEY)
    {
        // 判断是哪个快捷键被按下了
        switch (msg->wParam)
        {
        case 1:
            // Ctrl+shift+T被按下了
            if (isActiveWindow())
            {
                hide();
            }
            else
            {
                showNormal();
                activateWindow();
            }
            break;
        }
    }
    return false;
}

void MainWindow::Stamp2Visual()
{
    QString stTimeStamp = ui->textTimeStamp->toPlainText();
    time_t timeStamp = stTimeStamp.toLongLong();
    if (timeStamp < 0)
    {
        timeStamp = 0;
    }
    tm beijingTime = *localtime(&timeStamp);
    char buffer[80];
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", &beijingTime);
    ui->textTimeBeijing->setText(buffer);
    ui->textTimeBeijing->setAlignment(Qt::AlignHCenter);
}

void MainWindow::Visual2Stamp()
{
    QString stTimeBeijing = ui->textTimeBeijing->toPlainText();
    QDateTime dateTime = QDateTime::fromString(stTimeBeijing, "yyyy-MM-dd hh:mm:ss");
    time_t timeStamp = dateTime.toTime_t();
    ui->textTimeStamp->setText(QString::number(timeStamp));
    ui->textTimeStamp->setAlignment(Qt::AlignHCenter);
}

