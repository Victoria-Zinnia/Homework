#include "DialogBox.h"
#include "AudioManager.h"
#include <QPainter>
#include <QMouseEvent>
#include <QFont>

DialogBox::DialogBox(QWidget* parent) : QWidget(parent), m_side(SideCenter)
{
    setFixedHeight(200);
    setStyleSheet("background-color: transparent;");

    m_nameLabel = new QLabel(this);
    m_nameLabel->setGeometry(100, 20, 300, 35);
    QFont nameFont("Microsoft YaHei", 14, QFont::Bold);
    m_nameLabel->setFont(nameFont);
    m_nameLabel->setStyleSheet("color: #FFD700; background-color: transparent;");

    m_textLabel = new QLabel(this);
    m_textLabel->setGeometry(40, 55, 1100, 120);
    m_textLabel->setWordWrap(true);
    QFont textFont("Microsoft YaHei", 13);
    m_textLabel->setFont(textFont);
    m_textLabel->setStyleSheet("color: #FFFFFF; background-color: transparent;");
    m_textLabel->setAlignment(Qt::AlignCenter);

    m_typingTimer = new QTimer(this);
    connect(m_typingTimer, &QTimer::timeout, this, &DialogBox::onTypingTimer);

    m_currentIndex = 0;
    m_typing = false;
    m_finished = false;
}

void DialogBox::setSpeaker(const QString& name)
{
    if (name.isEmpty()) {
        m_nameLabel->hide();
    }
    else {
        m_nameLabel->show();
        m_nameLabel->setText(QString::fromUtf8(u8"【") + name + QString::fromUtf8(u8"】"));
    }
}

void DialogBox::setText(const QString& text)
{
    m_fullText = text;
    m_textLabel->setText("");
    m_currentIndex = 0;
    m_typing = false;
    m_finished = false;
}

void DialogBox::setVoice(const QString& voiceFile)
{
    m_voiceFile = voiceFile;
}

void DialogBox::setSpeakerSide(SpeakerSide side)
{
    m_side = side;
}

void DialogBox::setBackgroundImage(const QPixmap& pix)
{
    if (!pix.isNull()) {
        m_bgPixmap = pix;
        update();
        updateLayout();   // ← 新增：立即更新 nameLabel/textLabel 位置
    }
}

void DialogBox::startTyping()
{
    if (m_fullText.isEmpty()) return;

    AudioManager::instance()->stopVoice();

    // ← 新增：先清理旧的 effect，防止残留动画崩溃
    QGraphicsEffect* oldEffect = graphicsEffect();
    if (oldEffect) {
        setGraphicsEffect(nullptr);  // 这会 delete oldEffect
    }

    m_currentIndex = 0;
    m_typing = true;
    m_finished = false;
    m_textLabel->setText("");
    m_typingTimer->start(TYPING_INTERVAL);

    if (!m_voiceFile.isEmpty()) {
        AudioManager::instance()->playVoice(m_voiceFile);
    }
}

void DialogBox::skipTyping()
{
    if (!m_typing) return;
    m_typingTimer->stop();
    m_textLabel->setText(m_fullText);
    m_typing = false;
    m_finished = true;

    AudioManager::instance()->stopVoice();

    emit textFinished();
}

void DialogBox::clear()
{
    m_typingTimer->stop();
    m_textLabel->setText("");
    m_nameLabel->setText("");
    m_fullText.clear();
    m_voiceFile.clear();
    m_currentIndex = 0;
    m_typing = false;
    m_finished = false;
    m_bgPixmap = QPixmap();
    m_side = SideCenter;
    setGraphicsEffect(nullptr);
}

bool DialogBox::isTyping() const { return m_typing; }
bool DialogBox::isFinished() const { return m_finished; }

void DialogBox::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event)
        if (m_typing) {
            skipTyping();
        }
        else {
            emit clicked();
        }
}

void DialogBox::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
        QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (!m_bgPixmap.isNull()) {
        painter.drawPixmap(rect(), m_bgPixmap.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
    else {
        QRect boxRect = rect().adjusted(20, 0, -20, -10);
        painter.fillRect(boxRect, QColor(0, 0, 0, 180));
        painter.setPen(QPen(QColor(100, 100, 120), 2));
        painter.drawRect(boxRect);
    }
}

void DialogBox::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateLayout();   // ← 替换掉原来的 if/else 代码块
}

void DialogBox::onTypingTimer()
{
    if (m_currentIndex >= m_fullText.length()) {
        m_typingTimer->stop();
        m_typing = false;
        m_finished = true;
        emit textFinished();
        return;
    }

    m_currentIndex++;
    m_textLabel->setText(m_fullText.left(m_currentIndex));
}

void DialogBox::updateLayout()
{
    if (!m_bgPixmap.isNull()) {
        int padX = 350;      // ← 从 50 提到 350，彻底避开左右花纹
        int padTop = 26;
        int padBottom = 30;
        m_nameLabel->setGeometry(padX, padTop, width() - padX * 2, 34);
        m_textLabel->setGeometry(padX, padTop + 38, width() - padX * 2,
            qMax(80, height() - padTop - padBottom - 38));
    }
    else {
        m_nameLabel->setGeometry(100, 20, width() - 200, 34);
        m_textLabel->setGeometry(100, 58, width() - 200, qMax(80, height() - 80));
    }
}