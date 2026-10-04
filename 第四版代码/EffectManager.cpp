#include "EffectManager.h"
#include <QGraphicsBlurEffect>
#include <QPainter>
#include <QLabel>
#include <QRandomGenerator>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

EffectManager::EffectManager(QWidget* parentWidget, QObject* parent)
    : QObject(parent), m_parentWidget(parentWidget)
{
    m_particleTimer = new QTimer(this);
}

EffectManager::~EffectManager()
{
    stopParticleEffect();
}

void EffectManager::fadeIn(QWidget* widget, int duration)
{
    if (!widget) return;
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);
    effect->setOpacity(0);

    // ← parent = this，确保 EffectManager 删除时动画也被删
    QPropertyAnimation* anim = new QPropertyAnimation(effect, "opacity", this);
    anim->setDuration(duration);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::InOutQuad);
    connect(anim, &QPropertyAnimation::finished, this, [this, anim]() {
        anim->deleteLater();
        emit effectFinished();
        });
    anim->start();
}

void EffectManager::fadeOut(QWidget* widget, int duration, std::function<void()> onFinished)
{
    if (!widget) return;
    QGraphicsOpacityEffect* effect = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(widget);
        widget->setGraphicsEffect(effect);
    }

    QPropertyAnimation* anim = new QPropertyAnimation(effect, "opacity", this);
    anim->setDuration(duration);
    anim->setStartValue(effect->opacity());
    anim->setEndValue(0.0);
    anim->setEasingCurve(QEasingCurve::InOutQuad);
    connect(anim, &QPropertyAnimation::finished, this, [this, anim, onFinished]() {
        if (onFinished) onFinished();
        anim->deleteLater();
        emit effectFinished();
        });
    anim->start();
}

void EffectManager::flashback(QWidget* widget, int duration)
{
    if (!widget) return;
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);

    QPropertyAnimation* flash = new QPropertyAnimation(effect, "opacity", this);
    flash->setDuration(duration);
    flash->setKeyValueAt(0, 1.0);
    flash->setKeyValueAt(0.1, 0.3);
    flash->setKeyValueAt(0.2, 1.0);
    flash->setKeyValueAt(0.3, 0.2);
    flash->setKeyValueAt(0.4, 1.0);
    flash->setKeyValueAt(0.5, 0.5);
    flash->setKeyValueAt(0.6, 1.0);
    flash->setKeyValueAt(1.0, 1.0);
    flash->setEasingCurve(QEasingCurve::Linear);
    QPointer<QWidget> safeWidget(widget);
    connect(flash, &QPropertyAnimation::finished, this, [flash, safeWidget]() {
        if (safeWidget) {
            safeWidget->setGraphicsEffect(nullptr);  // ← 关键：清除残留 effect
        }
        flash->deleteLater();
        });
    flash->start();
}

void EffectManager::startParticleEffect(const QString& type)
{
    stopParticleEffect();
    m_particleTimer->disconnect();
    m_particleTimer->setInterval(100);
    connect(m_particleTimer, &QTimer::timeout, this, [this, type]() {
        createParticle(type);
        });
    m_particleTimer->start();
}

void EffectManager::stopParticleEffect()
{
    m_particleTimer->stop();
    auto copy = m_particles;
    m_particles.clear();
    for (QPointer<QLabel> p : copy) {
        if (p) {
            p->deleteLater();
        }
    }
}

void EffectManager::createParticle(const QString& type)
{
    if (!m_parentWidget) return;
    QLabel* particle = new QLabel(m_parentWidget);
    int size = QRandomGenerator::global()->bounded(10, 30);
    particle->setFixedSize(size, size);

    int startX = QRandomGenerator::global()->bounded(m_parentWidget->width());
    particle->move(startX, -size);

    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);

    if (type == "petal") {
        painter.setBrush(QColor(255, 182, 193));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(0, 0, size, size);
    }
    else if (type == "snow") {
        painter.setBrush(Qt::white);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(0, 0, size, size);
    }
    else if (type == "ginkgo") {
        painter.setBrush(QColor(255, 215, 0));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(0, 0, size, size * 2);
    }
    else {
        painter.setBrush(QColor(200, 200, 255, 100));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(0, 0, size, size / 2);
    }
    painter.end();
    particle->setPixmap(pix);

    // ← parent = this，确保 EffectManager 删除时动画也被删
    QPropertyAnimation* anim = new QPropertyAnimation(particle, "pos", this);
    anim->setDuration(QRandomGenerator::global()->bounded(3000, 6000));
    anim->setStartValue(particle->pos());
    anim->setEndValue(QPoint(
        startX + QRandomGenerator::global()->bounded(-100, 100),
        m_parentWidget->height() + size
    ));
    anim->setEasingCurve(QEasingCurve::Linear);

    QPointer<QLabel> safeParticle(particle);
    connect(anim, &QPropertyAnimation::finished, this, [this, safeParticle, anim]() {
        m_particles.removeAll(safeParticle);
        if (safeParticle) {
            safeParticle->deleteLater();
        }
        anim->deleteLater();
        });
    anim->start();

    m_particles.append(QPointer<QLabel>(particle));
}

void EffectManager::blurBackground(QWidget* bgWidget, bool enable)
{
    if (!bgWidget) return;
    if (enable) {
        QGraphicsBlurEffect* blur = new QGraphicsBlurEffect(bgWidget);
        blur->setBlurRadius(20);
        bgWidget->setGraphicsEffect(blur);
    }
    else {
        bgWidget->setGraphicsEffect(nullptr);
    }
}

void EffectManager::ghostFadeOut(QWidget* charWidget, int duration)
{
    if (!charWidget) return;
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
    charWidget->setGraphicsEffect(effect);

    QPropertyAnimation* fade = new QPropertyAnimation(effect, "opacity", this);
    fade->setDuration(duration);
    fade->setStartValue(1.0);
    fade->setEndValue(0.0);
    fade->setEasingCurve(QEasingCurve::InOutQuad);

    QPropertyAnimation* rise = new QPropertyAnimation(charWidget, "pos", this);
    rise->setDuration(duration);
    rise->setStartValue(charWidget->pos());
    rise->setEndValue(QPoint(charWidget->x(), charWidget->y() - 100));
    rise->setEasingCurve(QEasingCurve::InOutQuad);

    QParallelAnimationGroup* group = new QParallelAnimationGroup(this);
    group->addAnimation(fade);
    group->addAnimation(rise);

    // ← 关键：用 QPointer 保护 charWidget，防止场景删除后访问悬空指针
    QPointer<QWidget> safeWidget(charWidget);
    connect(group, &QParallelAnimationGroup::finished, this, [this, group, fade, rise, safeWidget]() {
        fade->setTargetObject(nullptr);
        rise->setTargetObject(nullptr);
        if (safeWidget) {
            safeWidget->setGraphicsEffect(nullptr);
        }
        group->deleteLater();
        emit effectFinished();
        });
    group->start();
}

void EffectManager::screenShake(QWidget* widget, int intensity, int duration)
{
    if (!widget) return;
    QPoint originalPos = widget->pos();
    QTimer* timer = new QTimer(this);
    auto elapsed = std::make_shared<int>(0);

    // ← 关键：用 QPointer 保护 widget
    QPointer<QWidget> safeWidget(widget);
    connect(timer, &QTimer::timeout, this, [safeWidget, originalPos, intensity, duration, timer, elapsed]() {
        if (!safeWidget) {
            timer->stop();
            timer->deleteLater();
            return;
        }
        *elapsed += 50;
        if (*elapsed > duration) {
            safeWidget->move(originalPos);
            timer->stop();
            timer->deleteLater();
            return;
        }
        int dx = QRandomGenerator::global()->bounded(-intensity, intensity);
        int dy = QRandomGenerator::global()->bounded(-intensity, intensity);
        safeWidget->move(originalPos.x() + dx, originalPos.y() + dy);
        });
    timer->start(50);
}

void EffectManager::blackFadeIn(QWidget* overlay, int duration)
{
    if (!overlay) return;
    overlay->setStyleSheet("background-color: black;");
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(overlay);
    overlay->setGraphicsEffect(effect);
    effect->setOpacity(1.0);

    QPropertyAnimation* anim = new QPropertyAnimation(effect, "opacity", this);
    anim->setDuration(duration);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    anim->setEasingCurve(QEasingCurve::InOutQuad);
    connect(anim, &QPropertyAnimation::finished, this, [this, anim]() {
        anim->deleteLater();
        emit effectFinished();
        });
    anim->start();
}