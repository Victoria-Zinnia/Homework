#include "HallScene.h"
#include "GameData.h"
#include "GameEngine.h"
#include "ResourceManager.h"
#include "SaveLoadManager.h"
#include <QPushButton>
#include <QLabel>
#include <QVariantMap>

HallScene::HallScene(QWidget* parent) : SceneBase(parent)
{
}

void HallScene::onEnter()
{
    setBackground(QString::fromUtf8(u8"轮回大厅"));
    playBGM(QString::fromUtf8(u8"轮回大厅.mp3"));
    setupUI();
}

void HallScene::onExit()
{
    stopBGM();
}

void HallScene::setupUI()
{
    if (m_narrator) { m_narrator->deleteLater(); m_narrator = nullptr; }
    if (m_skinLabel) { m_skinLabel->deleteLater(); m_skinLabel = nullptr; }
    for (auto btn : m_levelButtons) btn->deleteLater();
    m_levelButtons.clear();
    if (m_finalBtn) { m_finalBtn->deleteLater(); m_finalBtn = nullptr; }
    if (m_backBtn) { m_backBtn->deleteLater(); m_backBtn = nullptr; }
    if (m_wardrobeBtn) { m_wardrobeBtn->deleteLater(); m_wardrobeBtn = nullptr; }

    m_narrator = new QLabel(QString::fromUtf8(u8"轮回之庭，四时交替，由此开始"), this);
    m_narrator->setAlignment(Qt::AlignCenter);
    m_narrator->setGeometry(0, 60, width(), 40);
    m_narrator->setStyleSheet("color: #CCCCCC; font-size: 18px; font-family: 'Microsoft YaHei'; background: transparent;");
    m_narrator->raise();

    SkinInfo info = GameData::instance()->getSkinInfo(GameData::instance()->currentSkin());
    m_skinLabel = new QLabel(this);
    m_skinLabel->setGeometry(30, 30, 100, 150);
    QPixmap skinPix = ResourceManager::instance()->getSkin(info.imageFile)
        .scaled(100, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_skinLabel->setPixmap(skinPix);
    m_skinLabel->setStyleSheet("border: 2px solid #8B7355; background-color: rgba(0,0,0,100);");
    m_skinLabel->lower();

    QStringList levelNames = {
        QString::fromUtf8(u8"春之庭"),
        QString::fromUtf8(u8"夏之阁"),
        QString::fromUtf8(u8"秋之祠"),
        QString::fromUtf8(u8"冬之坛")
    };
    int startX = (width() - 4 * 220) / 2;
    int startY = height() / 2 - 100;

    for (int i = 0; i < 4; ++i) {
        QPushButton* btn = new QPushButton(levelNames[i], this);
        btn->setGeometry(startX + i * 240, startY, 200, 280);

        bool completed = GameData::instance()->isLevelCompleted(static_cast<LevelID>(i));
        QString borderColor = completed ? "#4CAF50" : "#8B7355";

        btn->setStyleSheet(QString(
            "QPushButton {"
            "  background-color: rgba(30, 30, 45, 180);"
            "  color: #E0D0B0;"
            "  border: 3px solid %1;"
            "  border-radius: 15px;"
            "  font-size: 22px;"
            "  font-family: 'Microsoft YaHei';"
            "}"
            "QPushButton:hover {"
            "  background-color: rgba(50, 50, 70, 200);"
            "  border: 3px solid #C8A882;"
            "}"
        ).arg(borderColor));

        connect(btn, &QPushButton::clicked, this, [this, i]() {
            onLevelClicked(static_cast<LevelID>(i));
            });

        btn->show();
        btn->raise();
        m_levelButtons.append(btn);
    }

    m_finalBtn = new QPushButton(QString::fromUtf8(u8"终章·轮回大厅"), this);
    m_finalBtn->setGeometry(width() / 2 - 150, height() - 150, 300, 60);
    m_finalBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(180, 40, 40, 200);"
        "  color: #FFD700;"
        "  border: 3px solid #FFD700;"
        "  border-radius: 12px;"
        "  font-size: 20px;"
        "  font-family: 'Microsoft YaHei';"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(220, 60, 60, 220);"
        "}"
    );
    m_finalBtn->hide();
    connect(m_finalBtn, &QPushButton::clicked, this, [this]() {
        checkFinalChapter();
        });

    if (GameData::instance()->allLevelsCompleted()) {
        m_finalBtn->show();
    }
    m_finalBtn->raise();

    // 返回标题按钮
    m_backBtn = new QPushButton(QString::fromUtf8(u8"返回标题"), this);
    m_backBtn->setGeometry(width() - 150, 30, 120, 40);
    m_backBtn->setStyleSheet(
        "QPushButton { background-color: rgba(60,50,40,200); color: #F0E6D2; "
        "border: 2px solid #8B7355; border-radius: 8px; font-size: 14px; }"
    );
    connect(m_backBtn, &QPushButton::clicked, this, []() {
        GameEngine::instance()->changeScene("title");
        });
    m_backBtn->raise();

    // 衣柜按钮（新增）
    m_wardrobeBtn = new QPushButton(QString::fromUtf8(u8"衣柜"), this);
    m_wardrobeBtn->setGeometry(width() - 150, 80, 120, 40);
    m_wardrobeBtn->setStyleSheet(
        "QPushButton { background-color: rgba(60,50,40,200); color: #F0E6D2; "
        "border: 2px solid #8B7355; border-radius: 8px; font-size: 14px; }"
    );
    connect(m_wardrobeBtn, &QPushButton::clicked, this, []() {
        GameEngine::instance()->changeScene("wardrobe");
        });
    m_wardrobeBtn->raise();

    // 存档按钮（新增）
    m_saveBtn = new QPushButton(QString::fromUtf8(u8"存档"), this);
    m_saveBtn->setGeometry(width() - 150, 130, 120, 40);
    m_saveBtn->setStyleSheet(
        "QPushButton { background-color: rgba(60,50,40,200); color: #F0E6D2; "
        "border: 2px solid #8B7355; border-radius: 8px; font-size: 14px; }"
    );
    connect(m_saveBtn, &QPushButton::clicked, this, [this]() {
        SaveLoadManager* sl = new SaveLoadManager(this);
        sl->showSaveDialog();
        });
    m_saveBtn->raise();
}

void HallScene::checkFinalChapter()
{
    GameData* gd = GameData::instance();
    FinalEnding ending = FINAL_NONE;

    if (gd->anyCorrupt()) {
        ending = FINAL_BROKEN;
    }
    else if (!gd->allAEnding()) {
        ending = FINAL_DETAINED;
    }
    else if (!gd->allDFlag()) {
        ending = FINAL_RECONCILED;
    }
    else {
        ending = FINAL_TRUE;
    }

    QVariantMap params;
    params["finalEnding"] = ending;
    GameEngine::instance()->changeScene("ending", params);
}

void HallScene::onLevelClicked(LevelID level)
{
    QVariantMap params;
    params["level"] = level;
    GameEngine::instance()->changeScene("level", params);
}

void HallScene::resizeEvent(QResizeEvent* event)
{
    SceneBase::resizeEvent(event);

    if (m_narrator) {
        m_narrator->setGeometry(0, 60, width(), 40);
    }
    if (m_skinLabel) {
        m_skinLabel->setGeometry(30, 30, 100, 150);
    }

    int startX = (width() - 4 * 220) / 2;
    int startY = height() / 2 - 100;
    for (int i = 0; i < m_levelButtons.size(); ++i) {
        m_levelButtons[i]->setGeometry(startX + i * 240, startY, 200, 280);
    }

    if (m_finalBtn) {
        m_finalBtn->setGeometry(width() / 2 - 150, height() - 150, 300, 60);
    }
    if (m_backBtn) {
        m_backBtn->setGeometry(width() - 150, 30, 120, 40);
    }
    if (m_wardrobeBtn) {
        m_wardrobeBtn->setGeometry(width() - 150, 80, 120, 40);
    }
    if (m_saveBtn) {
        m_saveBtn->setGeometry(width() - 150, 130, 120, 40);
    }
}