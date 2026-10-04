#include "TitleScene.h"
#include "GameEngine.h"
#include "GameData.h"
#include "AudioManager.h"
#include "EffectManager.h"
#include "SaveLoadManager.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPushButton>
#include <QLabel>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>

TitleScene::TitleScene(QWidget* parent) : SceneBase(parent)
{
    m_state = BOOT;
    m_effect = new EffectManager(this, this);
    m_bootAnim = nullptr;
    m_hintAnim = nullptr;
}

void TitleScene::onEnter()
{
    stopBootAnimations();

    for (auto btn : m_titleButtons) {
        btn->deleteLater();
    }
    m_titleButtons.clear();
    if (m_bootOverlay) {
        m_bootOverlay->setGraphicsEffect(nullptr);
        m_bootOverlay->deleteLater();
        m_bootOverlay = nullptr;
    }
    if (m_hintLabel) {
        m_hintLabel->setGraphicsEffect(nullptr);
        m_hintLabel->deleteLater();
        m_hintLabel = nullptr;
    }

    setupBoot();
    playBGM(QString::fromUtf8(u8"标题页.mp3"));
}

void TitleScene::onExit()
{
    stopBGM();
    stopBootAnimations();
    AudioManager::instance()->stopVoice();
}

void TitleScene::stopBootAnimations()
{
    if (m_bootAnim) {
        m_bootAnim->stop();
        delete m_bootAnim;
        m_bootAnim = nullptr;
    }
    if (m_hintAnim) {
        m_hintAnim->stop();
        delete m_hintAnim;
        m_hintAnim = nullptr;
    }
}

void TitleScene::setupBoot()
{
    m_state = BOOT;
    setBackground(QString::fromUtf8(u8"开机页"));

    m_bootOverlay = new QWidget(this);
    m_bootOverlay->setGeometry(rect());
    m_bootOverlay->setStyleSheet("background-color: black;");
    m_bootOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_bootOverlay->show();
    m_bootOverlay->raise();

    m_hintLabel = new QLabel(QString::fromUtf8(u8"进入尘封过往"), this);
    m_hintLabel->setAlignment(Qt::AlignCenter);
    QFont hintFont("Microsoft YaHei", 20);
    m_hintLabel->setFont(hintFont);
    m_hintLabel->setStyleSheet("color: #AAAAAA; background-color: transparent;");
    m_hintLabel->setGeometry(0, height() * 3 / 4, width(), 50);
    m_hintLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_hintLabel->hide();

    QGraphicsOpacityEffect* eff = new QGraphicsOpacityEffect(m_bootOverlay);
    m_bootOverlay->setGraphicsEffect(eff);

    // ← 关键：parent = this
    m_bootAnim = new QPropertyAnimation(eff, "opacity", this);
    m_bootAnim->setDuration(2000);
    m_bootAnim->setStartValue(1.0);
    m_bootAnim->setEndValue(0.0);
    connect(m_bootAnim, &QPropertyAnimation::finished, this, [this]() {
        if (m_bootOverlay) {
            m_bootOverlay->setGraphicsEffect(nullptr);
            m_bootOverlay->deleteLater();
            m_bootOverlay = nullptr;
        }
        m_bootAnim = nullptr;
        });

    QTimer::singleShot(500, this, [this]() {
        if (m_bootAnim) m_bootAnim->start();
        });

    QTimer::singleShot(2500, this, [this]() {
        if (!m_hintLabel) return;
        m_hintLabel->show();
        QGraphicsOpacityEffect* heff = new QGraphicsOpacityEffect(m_hintLabel);
        m_hintLabel->setGraphicsEffect(heff);
        heff->setOpacity(0);

        // ← 关键：parent = this
        m_hintAnim = new QPropertyAnimation(heff, "opacity", this);
        m_hintAnim->setDuration(1000);
        m_hintAnim->setStartValue(0);
        m_hintAnim->setEndValue(1);
        connect(m_hintAnim, &QPropertyAnimation::finished, this, [this]() {
            m_hintAnim = nullptr;
            });
        m_hintAnim->start();
        });

    QTimer::singleShot(1800, this, [this]() {
        playVoice("narrator/narrator_001.mp3");
        });
}

void TitleScene::setupTitle()
{
    AudioManager::instance()->stopVoice();
    m_state = TITLE;

    stopBootAnimations();
    if (m_bootOverlay) {
        m_bootOverlay->setGraphicsEffect(nullptr);
        m_bootOverlay->deleteLater();
        m_bootOverlay = nullptr;
    }
    if (m_hintLabel) {
        m_hintLabel->setGraphicsEffect(nullptr);
        m_hintLabel->deleteLater();
        m_hintLabel = nullptr;
    }

    setBackground(QString::fromUtf8(u8"标题页"));
    playBGM(QString::fromUtf8(u8"标题页.mp3"));

    for (auto btn : m_titleButtons) {
        btn->deleteLater();
    }
    m_titleButtons.clear();

    QStringList btnTexts = {
        QString::fromUtf8(u8"开始游戏"),
        QString::fromUtf8(u8"衣柜"),
        QString::fromUtf8(u8"读档"),
        QString::fromUtf8(u8"退出")
    };
    int startY = height() / 2;

    for (int i = 0; i < btnTexts.size(); ++i) {
        QPushButton* btn = new QPushButton(btnTexts[i], this);
        btn->setGeometry(width() / 2 - 150, startY + i * 70, 300, 50);
        btn->setStyleSheet(
            "QPushButton {"
            "  background-color: rgba(60, 50, 40, 200);"
            "  color: #F0E6D2;"
            "  border: 2px solid #8B7355;"
            "  border-radius: 10px;"
            "  font-size: 18px;"
            "  font-family: 'Microsoft YaHei';"
            "}"
            "QPushButton:hover {"
            "  background-color: rgba(100, 80, 60, 220);"
            "  border: 2px solid #C8A882;"
            "}"
        );

        connect(btn, &QPushButton::clicked, this, [this, i]() {
            switch (i) {
            case 0:
                GameData::instance()->fullReset();
                GameEngine::instance()->changeScene("hall");
                break;
            case 1:
                GameEngine::instance()->changeScene("wardrobe");
                break;
            case 2: {
                SaveLoadManager* sl = new SaveLoadManager(this);
                sl->showLoadDialog();
                break;
            }
            case 3:
                break;
            }
            });

        btn->show();
        btn->raise();
        m_titleButtons.append(btn);
    }
}

void TitleScene::mousePressEvent(QMouseEvent* event)
{
    if (m_state == BOOT) {
        setupTitle();
        return;
    }
    SceneBase::mousePressEvent(event);
}

void TitleScene::resizeEvent(QResizeEvent* event)
{
    SceneBase::resizeEvent(event);
    if (m_state == TITLE) {
        int startY = height() / 2;
        for (int i = 0; i < m_titleButtons.size(); ++i) {
            m_titleButtons[i]->setGeometry(width() / 2 - 150, startY + i * 70, 300, 50);
        }
    }
    if (m_bootOverlay) {
        m_bootOverlay->setGeometry(rect());
    }
    if (m_hintLabel) {
        m_hintLabel->setGeometry(0, height() * 3 / 4, width(), 50);
    }
}