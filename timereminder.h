#ifndef TIMEREMINDER_H
#define TIMEREMINDER_H

#include <QTimer>
#include <QLabel>
#include <QDateTime>
#include <QMouseEvent>
#include <QDebug>
#include <QPainter>
#include <QHBoxLayout>
#include <QGuiApplication>
#include <QScreen>
#include <QBitmap>

class TimeReminder : public QLabel
{
public:
    explicit TimeReminder(QWidget *parent = 0);
    ~TimeReminder();

protected:
    void updateTime();
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *);
    void GradualShow();
    void GradualHide();

private:
    QTimer *m_timer;
    QTimer *m_timer2;
    int m_remainingTime;

};

#endif // TIMEREMINDER_H


