#include "CGPlayer.h"
#include "ResourceManager.h"
#include "EffectManager.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

CGPlayer::CGPlayer(QWidget* parent) : QWidget(parent)
{
    setGeometry(0, 0, 1280, 720);
    hide();
    m_showing = false;
    m_fadeAnim = nullptr;
}

void CGPlayer::playCG(const QString& cgName)
{
    // Í£Ö¹¾É¶¯»­£¬·ÀÖ¹¿ìËÙÇÐ»»Ê±²ÐÁô
    if (m_fadeAnim) {
        m_fadeAnim->stop();
        delete m_fadeAnim;
        m_fadeAnim = nullptr;
    }

    m_cgPixmap = ResourceManager::instance()->getCG(cgName);
    m_showing = true;
    show();
    raise();

    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(effect);
    effect->setOpacity(0);

    // ¡û ¹Ø¼ü£ºparent = this£¬È·±£ CGPlayer É¾³ýÊ±¶¯»­Ò²±»É¾
    m_fadeAnim = new QPropertyAnimation(effect, "opacity", this);
    m_fadeAnim->setDuration(1500);
    m_fadeAnim->setStartValue(0.0);
    m_fadeAnim->setEndValue(1.0);
    connect(m_fadeAnim, &QPropertyAnimation::finished, this, [this]() {
        m_fadeAnim = nullptr;
        });
    m_fadeAnim->start();
}

void CGPlayer::setOnFinished(std::function<void()> callback)
{
    m_callback = callback;
}

void CGPlayer::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event)
        if (!m_showing) return;

    // Í£Ö¹¾É¶¯»­
    if (m_fadeAnim) {
        m_fadeAnim->stop();
        delete m_fadeAnim;
        m_fadeAnim = nullptr;
    }

    QGraphicsOpacityEffect* effect = qobject_cast<QGraphicsOpacityEffect*>(graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(effect);
    }

    // ¡û ¹Ø¼ü£ºparent = this
    m_fadeAnim = new QPropertyAnimation(effect, "opacity", this);
    m_fadeAnim->setDuration(800);
    m_fadeAnim->setStartValue(1.0);
    m_fadeAnim->setEndValue(0.0);
    connect(m_fadeAnim, &QPropertyAnimation::finished, this, [this]() {
        hide();
        m_showing = false;
        m_fadeAnim = nullptr;
        if (m_callback) m_callback();
        emit cgFinished();
        });
    m_fadeAnim->start();
}

void CGPlayer::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
        QPainter painter(this);
    if (!m_cgPixmap.isNull()) {
        painter.drawPixmap(rect(), m_cgPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }
}