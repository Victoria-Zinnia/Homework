#include "WardrobeScene.h"
#include "GameData.h"
#include "GameEngine.h"
#include "ResourceManager.h"
#include <QPushButton>
#include <QLabel>
#include <QPainter>

WardrobeScene::WardrobeScene(QWidget* parent) : SceneBase(parent)
{
    m_selectedSkin = GameData::instance()->currentSkin();
}

void WardrobeScene::onEnter()
{
    setBackground(QString::fromUtf8(u8"标题页"));
    playBGM(QString::fromUtf8(u8"标题页.mp3"));
    setupGrid();

    QLabel* title = new QLabel(QString::fromUtf8(u8"衣 柜"), this);
    title->setAlignment(Qt::AlignCenter);
    QFont font("Microsoft YaHei", 24, QFont::Bold);
    title->setFont(font);
    title->setStyleSheet("color: #E0D0B0; background-color: transparent;");
    title->setGeometry(0, 20, width(), 50);
    title->raise();
}

void WardrobeScene::onExit()
{
    stopBGM();
}

void WardrobeScene::setupGrid()
{
    int cellW = 280;
    int cellH = 380;
    int startX = (width() - cellW * 3) / 2;
    int startY = 100;

    for (int i = 0; i < 9; ++i) {
        int row = i / 3;
        int col = i % 3;
        int x = startX + col * (cellW + 30);
        int y = startY + row * (cellH + 30);

        QPushButton* btn = new QPushButton(this);
        btn->setGeometry(x, y, cellW, cellH);

        SkinID sid = static_cast<SkinID>(i);
        SkinInfo info = GameData::instance()->getSkinInfo(sid);
        bool unlocked = info.unlocked;

        QPixmap thumb = ResourceManager::instance()->getSkin(info.imageFile)
            .scaled(cellW - 20, cellH - 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        QString style;
        if (unlocked) {
            style = QString(
                "QPushButton { background-color: rgba(40,40,50,180); "
                "border: %1px solid %2; border-radius: 12px; }"
            ).arg(m_selectedSkin == i ? "4" : "2")
                .arg(m_selectedSkin == i ? "#FFD700" : "#666688");
        }
        else {
            style = "QPushButton { background-color: rgba(20,20,25,200); "
                "border: 2px solid #333; border-radius: 12px; }";
        }
        btn->setStyleSheet(style);

        QLabel* imgLabel = new QLabel(btn);
        imgLabel->setGeometry((cellW - thumb.width()) / 2, 10, thumb.width(), thumb.height());
        imgLabel->setPixmap(thumb);
        if (!unlocked) {
            imgLabel->setStyleSheet("background-color: rgba(0,0,0,120);");
        }

        QLabel* nameLabel = new QLabel(info.name, btn);
        nameLabel->setAlignment(Qt::AlignCenter);
        nameLabel->setGeometry(10, cellH - 50, cellW - 20, 40);
        nameLabel->setStyleSheet(unlocked ? "color: #FFFFFF; font-size: 16px; background: transparent;"
            : "color: #666666; font-size: 16px; background: transparent;");

        if (!unlocked) {
            QLabel* lockLabel = new QLabel(QString::fromUtf8(u8"🔒"), btn);
            lockLabel->setAlignment(Qt::AlignCenter);
            lockLabel->setGeometry((cellW - 50) / 2, (cellH - 100) / 2, 50, 50);
            lockLabel->setStyleSheet("color: #555; font-size: 28px; background: transparent;");
        }

        connect(btn, &QPushButton::clicked, this, [this, i, unlocked]() {
            if (unlocked) onSkinSelected(i);
            });

        btn->show();
        btn->raise();
        m_skinButtons.append(btn);
    }

    // 进入游戏按钮（直接进大厅，不用回开机页）
    QPushButton* startBtn = new QPushButton(QString::fromUtf8(u8"进入游戏"), this);
    startBtn->setGeometry(width() / 2 - 100, height() - 80, 200, 50);
    startBtn->setStyleSheet(
        "QPushButton { background-color: #4a6fa5; color: white; "
        "border-radius: 10px; font-size: 16px; font-family: 'Microsoft YaHei'; }"
        "QPushButton:hover { background-color: #5a8fc5; }"
    );
    connect(startBtn, &QPushButton::clicked, this, []() {
        GameEngine::instance()->changeScene("hall");
        });
    startBtn->raise();
}

void WardrobeScene::onSkinSelected(int skinId)
{
    m_selectedSkin = skinId;
    GameData::instance()->setCurrentSkin(static_cast<SkinID>(skinId));
    GameData::instance()->saveToFile();   // ← 新增：立即写入存档
    updateSelection();
}

void WardrobeScene::updateSelection()
{
    for (int i = 0; i < m_skinButtons.size(); ++i) {
        SkinID sid = static_cast<SkinID>(i);
        bool unlocked = GameData::instance()->isSkinUnlocked(sid);
        QString style;
        if (unlocked) {
            style = QString(
                "QPushButton { background-color: rgba(40,40,50,180); "
                "border: %1px solid %2; border-radius: 12px; }"
            ).arg(m_selectedSkin == i ? "4" : "2")
                .arg(m_selectedSkin == i ? "#FFD700" : "#666688");
        }
        else {
            style = "QPushButton { background-color: rgba(20,20,25,200); "
                "border: 2px solid #333; border-radius: 12px; }";
        }
        m_skinButtons[i]->setStyleSheet(style);
    }
}