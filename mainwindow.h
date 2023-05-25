#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QWidget>
#include <QFileDialog>
#include <QDir>
#include <QInputDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QSettings>
#include "timereminder.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void ModifyTime();
    void StopWindowsTimeService();
    void StartWindowsTimeService();
    void InitTextEdit();
    void InitCtrlMenu();
    void InitCtrlButton();
    void closeEvent(QCloseEvent *event) override;
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
    void SetSystemTray();
    void Stamp2Visual();
    void Visual2Stamp();
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTimer *m_timer;
    TimeReminder *m_timeReminder = NULL;
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    QAction *m_restoreAction;
    QAction *m_quitAction;
    QMenu *m_ctrlMenu;
    QAction *m_setCtrlBPath;
    QAction *m_setCtrlBTitle;
    QPushButton *m_curCtrlButton;
    QString m_defaultPath;
};


#endif // MAINWINDOW_H
