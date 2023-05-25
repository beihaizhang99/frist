#include "timereminder.h"

TimeReminder::TimeReminder(QWidget *parent)
    : QLabel(parent),
      m_timer(new QTimer(this)),
      m_remainingTime(0)
{
    // 设置时间标签的位置、大小、样式和对齐方式
    QRect screenRect = QGuiApplication::primaryScreen()->geometry();
    int screenWidth = screenRect.width();
    setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);// 设置窗口无边框置顶
    setGeometry((screenWidth / 2) - 200, -130, 400, 130);
    //setStyleSheet("font-size: 24pt; color: black; background-color: transparent; border: 1px solid black; border-radius: 50%; padding: 20px;");
    QColor backgroundColor = QColor::fromRgb(225, 214, 98);
    QPalette qpalette = palette();
    qpalette.setColor(QPalette::Background, backgroundColor);
    setPalette(qpalette);
    QPainterPath path;
    path.addRoundedRect(rect(), 35, 35);
    this->setMask(path.toFillPolygon().toPolygon());
    // 启动定时器，每秒更新时间标签
    connect(m_timer, &QTimer::timeout, this, &TimeReminder::updateTime);
    m_timer->start(500);
}

TimeReminder::~TimeReminder()
{
    delete m_timer;
}

void TimeReminder::updateTime()
{
    // 获取当前时间并设置到标签中
    QDateTime currentDateTime = QDateTime::currentDateTime();

    // 如果当前是整点，则显示标签
    if (!isVisible() && currentDateTime.time().minute() == 59 && currentDateTime.time().second() > 45) {
        show();
        m_timer2 = new QTimer();
        connect(m_timer2, &QTimer::timeout, this, &TimeReminder::GradualShow);
        m_timer2->start(2000/height());
        // 每500ms更新一次时间并持续45秒
        m_remainingTime = 90;
        qDebug() << "updateTime  show" ;
    }

    // 如果标签正在显示中，则更新剩余时间，并在倒计时结束后隐藏标签
    if (isVisible()) {
        if (m_remainingTime == 0) {
            m_timer2 = new QTimer();
            connect(m_timer2, &QTimer::timeout, this, &TimeReminder::GradualHide);
            m_timer2->start(2000/height());
        }
        m_remainingTime--;
        update();
    }
}

void TimeReminder::mousePressEvent(QMouseEvent *event)
{
    // 如果鼠标点击了标签，则立即隐藏标签
    if (isVisible() && rect().contains(event->pos())) {
        if (m_timer2)
        {
            delete m_timer2;
        }
        m_remainingTime = 0;
    }
}

void TimeReminder::paintEvent(QPaintEvent *)
{

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    //painter.fillRect(this->rect(), QColor(0, 0, 0, 0));

    // 获取当前时间
    QDateTime dateTime = QDateTime::currentDateTime();
    int hour = dateTime.time().hour();
    int minute = dateTime.time().minute();
    int second = dateTime.time().second();
    // 计算时针、分针、秒针的角度
    qreal hourAngle = (hour % 12 + minute / 60.0) * 30.0;
    qreal minuteAngle = minute * 6.0;
    qreal secondAngle = second * 6.0;

    // 绘制时钟
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    painter.drawEllipse(10, 10, 110, 110);
    painter.drawRoundedRect(140, 20, 250, 90, 35, 35); // 绘制圆角矩形
    painter.translate(65, 65);
    painter.setBrush(Qt::black);
    painter.rotate(180);
    painter.rotate(hourAngle);
    painter.drawRect(-2, -2, 4, 33);
    painter.rotate(minuteAngle - hourAngle);
    painter.drawRect(-1, -1, 3, 45);
    painter.rotate(secondAngle - minuteAngle);
    painter.setBrush(Qt::red);
    painter.drawRect(-1, -1, 2, 52);
    painter.setPen(Qt::red);
    painter.drawEllipse(-3, -3, 6, 6);
    painter.restore();
    QFont font("微软雅黑", 40);
    painter.setFont(font);
    QRectF textRect(140, 15, 250, 100);
    QTextOption textOption(Qt::AlignCenter);
    painter.drawText(textRect, dateTime.toString("hh:mm:ss"), textOption);

}


void TimeReminder::GradualShow()
{
    if (y() >= 10)
    {
        delete m_timer2;
        m_timer2 = nullptr;
    }
    move(x(), y() + 1);

}

void TimeReminder::GradualHide()
{
    if (y() <= -height())
    {
        delete m_timer2;
        m_timer2 = nullptr;
        hide();
    }
    move(x(), y() - 1);
}
