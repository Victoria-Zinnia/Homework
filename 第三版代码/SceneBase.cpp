#include "SceneBase.h"
#include "AudioManager.h"
#include "ResourceManager.h"
#include "EffectManager.h"
#include <QPainter>

SceneBase::SceneBase(QWidget* parent) : QWidget(parent)
{
    m_charDisplay = new QLabel(this);
    m_charDisplay->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_charDisplay->hide();
    m_charX = 0;
    m_charY = 0;
    m_audio = AudioManager::instance();
    m_res = ResourceManager::instance();
    m_effect = nullptr;
    setAutoFillBackground(false);
}

SceneBase::~SceneBase()
{
}

void SceneBase::setBackground(const QString& bgName)
{
    m_bgPixmap = m_res->getBackground(bgName);
    update();
}

void SceneBase::setBackground(const QPixmap& pixmap)
{
    m_bgPixmap = pixmap;
    update();
}

void SceneBase::showCharacter(const QString& name, const QString& state, int x, int y)
{
    QPixmap pix = m_res->getCharacter(name, state);
    showCharacterPixmap(pix, x, y);
}

void SceneBase::showCharacterPixmap(const QPixmap& pix, int x, int y)
{
    if (pix.isNull()) return;
    m_charDisplay->setPixmap(pix);
    if (x < 0) m_charX = width() - pix.width() - 50;
    else m_charX = x;
    if (y < 0) m_charY = height() - pix.height();
    else m_charY = y;
    m_charDisplay->move(m_charX, m_charY);
    m_charDisplay->resize(pix.size());
    m_charDisplay->show();
    m_charDisplay->raise();
}

void SceneBase::hideCharacter()
{
    m_charDisplay->hide();
}

void SceneBase::playBGM(const QString& bgmName)
{
    m_audio->playBGM(bgmName);
}

void SceneBase::stopBGM()
{
    m_audio->stopBGM();
}

void SceneBase::playVoice(const QString& voiceFile)
{
    m_audio->playVoice(voiceFile);
}

void SceneBase::playEffect(const QString& effectType)
{
    if (m_effect) {
        if (effectType == "flashback") m_effect->flashback(this);
        else if (effectType == "petal") m_effect->startParticleEffect("petal");
        else if (effectType == "snow") m_effect->startParticleEffect("snow");
        else if (effectType == "ripple") m_effect->startParticleEffect("ripple");
        else if (effectType == "ginkgo") m_effect->startParticleEffect("ginkgo");
        else if (effectType == "blur") m_effect->blurBackground(this, true);
        else if (effectType == "unblur") m_effect->blurBackground(this, false);
        else if (effectType == "shake") m_effect->screenShake(this);
    }
}

void SceneBase::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
        QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    if (!m_bgPixmap.isNull()) {
        painter.drawPixmap(rect(), m_bgPixmap);
    }
    else {
        painter.fillRect(rect(), QColor(20, 20, 30));
    }
}

void SceneBase::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (!m_charDisplay->isHidden() && m_charDisplay->pixmap() && !m_charDisplay->pixmap()->isNull()) {
        QPixmap pix = *m_charDisplay->pixmap();
        if (m_charX < width() / 2) {
            m_charX = 50;
        }
        else {
            m_charX = width() - pix.width() - 50;
        }
        m_charY = height() - pix.height();
        m_charDisplay->move(m_charX, m_charY);
    }
}