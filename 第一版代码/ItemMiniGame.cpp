#include "ItemMiniGame.h"
#include "ResourceManager.h"
#include "GameData.h"
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <QtMath>

ItemMiniGame::ItemMiniGame(QWidget* parent) : QWidget(parent),
m_timerId(0), m_time(0), m_running(false)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    hide();
}

void ItemMiniGame::startGame(const QStringList& itemNames, const QString& bgName)
{
    m_collected.clear();
    m_items.clear();
    m_time = 0;
    m_running = true;

    // 加载关卡背景图
    if (!bgName.isEmpty()) {
        m_backgroundPixmap = ResourceManager::instance()->getBackground(bgName);
    }
    else {
        m_backgroundPixmap = QPixmap();
    }

    // 玩家形象：当前皮肤，放大到 100×100
    SkinInfo skin = GameData::instance()->getSkinInfo(GameData::instance()->currentSkin());
    m_playerPixmap = ResourceManager::instance()->getSkin(skin.imageFile)
        .scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 随机生成道具位置
    for (const QString& name : itemNames) {
        GameItem item;
        item.name = name;
        item.pos = randomPos();
        item.collected = false;
        // 道具图标放大到 72×72
        item.icon = ResourceManager::instance()->getItem(name)
            .scaled(72, 72, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        item.phase = QRandomGenerator::global()->bounded(0, 628) / 100.0f;
        m_items.append(item);
    }

    resize(parentWidget() ? parentWidget()->size() : QSize(1280, 720));
    move(0, 0);
    m_playerPos = QPoint(width() / 2, height() / 2);

    show();
    raise();
    grabKeyboard();
    m_timerId = startTimer(16);
}

QPoint ItemMiniGame::randomPos() const
{
    int margin = 120;
    int w = qMax(100, width() - margin * 2);
    int h = qMax(100, height() - margin * 2);
    int x = margin + QRandomGenerator::global()->bounded(w);
    int y = margin + QRandomGenerator::global()->bounded(h);
    return QPoint(x, y);
}

void ItemMiniGame::timerEvent(QTimerEvent*)
{
    if (!m_running) return;

    m_time += 0.05f;

    // 边界限制
    int half = 50;
    m_playerPos.setX(qBound(half, m_playerPos.x(), width() - half));
    m_playerPos.setY(qBound(half, m_playerPos.y(), height() - half));

    // 碰撞检测
    checkCollisions();

    // 检查是否全部收集
    bool allCollected = true;
    for (const auto& item : m_items) {
        if (!item.collected) { allCollected = false; break; }
    }

    if (allCollected) {
        m_running = false;
        killTimer(m_timerId);
        releaseKeyboard();
        emit gameFinished(m_collected);
        hide();
    }

    update();
}

void ItemMiniGame::checkCollisions()
{
    int playerR = 45;
    for (auto& item : m_items) {
        if (item.collected) continue;

        int floatY = qSin(m_time + item.phase) * 8;
        QPoint itemCenter(item.pos.x(), item.pos.y() + floatY);

        int dx = m_playerPos.x() - itemCenter.x();
        int dy = m_playerPos.y() - itemCenter.y();
        int dist = qSqrt(dx * dx + dy * dy);

        // 拾取半径：玩家半径 + 道具半径(36) + 缓冲
        if (dist < playerR + 36 + 15) {
            item.collected = true;
            m_collected.append(item.name);
        }
    }
}

void ItemMiniGame::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 绘制背景图（全屏拉伸）
    if (!m_backgroundPixmap.isNull()) {
        p.drawPixmap(rect(), m_backgroundPixmap.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
    else {
        p.fillRect(rect(), QColor(10, 10, 20));
    }

    // 绘制未收集的道具（轻微浮动，融入背景）
    for (const auto& item : m_items) {
        if (item.collected) continue;

        int floatY = qSin(m_time + item.phase) * 8;
        int x = item.pos.x() - 36;
        int y = item.pos.y() - 36 + floatY;

        // 极淡的光晕，不破坏背景沉浸感
        p.setBrush(QColor(255, 215, 0, 25));
        p.setPen(Qt::NoPen);
        p.drawEllipse(x - 10, y - 10, 92, 92);

        p.drawPixmap(x, y, item.icon);

        // 道具名称
        p.setPen(QColor(240, 230, 210));
        p.setFont(QFont("Microsoft YaHei", 11));
        p.drawText(x - 14, y + 78, 100, 22, Qt::AlignCenter, item.name);
    }

    // 绘制玩家（较大，易于辨认）
    if (!m_playerPixmap.isNull()) {
        int px = m_playerPos.x() - m_playerPixmap.width() / 2;
        int py = m_playerPos.y() - m_playerPixmap.height() / 2;

        // 脚下淡淡光圈
        p.setBrush(QColor(100, 200, 255, 40));
        p.setPen(Qt::NoPen);
        p.drawEllipse(m_playerPos.x() - 50, m_playerPos.y() - 50, 100, 100);

        p.drawPixmap(px, py, m_playerPixmap);
    }
    else {
        p.setBrush(QColor(100, 200, 255));
        p.setPen(Qt::NoPen);
        p.drawEllipse(m_playerPos.x() - 25, m_playerPos.y() - 25, 50, 50);
    }

    // 顶部 UI 栏
    QRect uiRect(0, 0, width(), 48);
    p.fillRect(uiRect, QColor(0, 0, 0, 160));

    p.setPen(QColor(224, 208, 176));
    p.setFont(QFont("Microsoft YaHei", 15, QFont::Bold));
    p.drawText(uiRect.adjusted(20, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter,
        QString::fromUtf8(u8"流光拾忆 — 移动鼠标触碰道具"));

    p.setPen(QColor(255, 215, 0));
    p.setFont(QFont("Microsoft YaHei", 13, QFont::Bold));
    QString progress = QString::fromUtf8(u8"已收集：%1 / %2")
        .arg(m_collected.size()).arg(m_items.size());
    p.drawText(uiRect.adjusted(0, 0, -20, 0), Qt::AlignRight | Qt::AlignVCenter, progress);

    // 底部提示
    p.setPen(QColor(180, 180, 180));
    p.setFont(QFont("Microsoft YaHei", 11));
    p.drawText(rect().adjusted(0, -28, 0, -8), Qt::AlignBottom | Qt::AlignHCenter,
        QString::fromUtf8(u8"移动鼠标控制角色移动，ESC 跳过"));
}

void ItemMiniGame::mouseMoveEvent(QMouseEvent* event)
{
    // 角色中心直接跟随鼠标
    m_playerPos = event->pos();
    update();
}

void ItemMiniGame::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        m_running = false;
        killTimer(m_timerId);
        releaseKeyboard();
        QStringList allItems;
        for (const auto& item : m_items) allItems.append(item.name);
        emit gameSkipped(allItems);
        hide();
    }
}

void ItemMiniGame::showEvent(QShowEvent*)
{
    if (parentWidget()) {
        resize(parentWidget()->size());
        move(0, 0);
    }
}