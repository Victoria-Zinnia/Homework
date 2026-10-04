#include "MirrorPetalGame.h"
#include "ResourceManager.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QTimer>
#include <cmath>

MirrorPetalGame::MirrorPetalGame(QWidget* parent) : QWidget(parent),
m_gridSize(3), m_cellSize(100), m_running(false),
m_animTimer(nullptr), m_frame(0)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setFocusPolicy(Qt::StrongFocus);

    m_animTimer = new QTimer(this);
    connect(m_animTimer, &QTimer::timeout, this, [this]() {
        m_frame++;
        bool needUpdate = false;
        for (int r = 0; r < m_gridSize; ++r) {
            for (int c = 0; c < m_gridSize; ++c) {
                if (m_grid[r][c].pulse > 0) {
                    m_grid[r][c].pulse -= 0.06f;
                    if (m_grid[r][c].pulse < 0) m_grid[r][c].pulse = 0;
                    needUpdate = true;
                }
            }
        }
        if (needUpdate) update();
        });
    hide();
}

void MirrorPetalGame::startGame(const QString& itemName, const QString& bgName)
{
    Q_UNUSED(bgName)
        m_itemName = itemName;
    m_frame = 0;
    m_running = true;
    m_gridSize = 3;

    initPuzzle();

    resize(parentWidget() ? parentWidget()->size() : QSize(1280, 720));
    move(0, 0);
    m_animTimer->start(40);
    show(); raise(); grabKeyboard();
}

void MirrorPetalGame::initPuzzle()
{
    m_grid.resize(m_gridSize);
    for (int r = 0; r < m_gridSize; ++r) {
        m_grid[r].resize(m_gridSize);
        for (int c = 0; c < m_gridSize; ++c)
            m_grid[r][c] = { false, 0.0f };
    }

    // 简单打乱：随机点 2~4 下，保证可解且容易
    int steps = QRandomGenerator::global()->bounded(3) + 2;
    for (int i = 0; i < steps; ++i) {
        int r = QRandomGenerator::global()->bounded(m_gridSize);
        int c = QRandomGenerator::global()->bounded(m_gridSize);
        applyToggle(r, c);
    }

    // 如果碰巧全亮了，灭掉一个
    if (checkWin()) {
        applyToggle(1, 1);
    }

    for (int r = 0; r < m_gridSize; ++r)
        for (int c = 0; c < m_gridSize; ++c)
            m_grid[r][c].pulse = 0;
}

void MirrorPetalGame::applyToggle(int row, int col)
{
    auto flip = [&](int r, int c) {
        if (r >= 0 && r < m_gridSize && c >= 0 && c < m_gridSize) {
            m_grid[r][c].lit = !m_grid[r][c].lit;
            m_grid[r][c].pulse = 1.0f;
        }
        };
    flip(row, col);
    flip(row - 1, col);
    flip(row + 1, col);
    flip(row, col - 1);
    flip(row, col + 1);
}

bool MirrorPetalGame::checkWin() const
{
    for (int r = 0; r < m_gridSize; ++r)
        for (int c = 0; c < m_gridSize; ++c)
            if (!m_grid[r][c].lit) return false;
    return true;
}

int MirrorPetalGame::countLit() const
{
    int n = 0;
    for (int r = 0; r < m_gridSize; ++r)
        for (int c = 0; c < m_gridSize; ++c)
            if (m_grid[r][c].lit) ++n;
    return n;
}

QRect MirrorPetalGame::cellRect(int row, int col) const
{
    return QRect(m_boardOffset.x() + col * m_cellSize,
        m_boardOffset.y() + row * m_cellSize,
        m_cellSize, m_cellSize);
}

QPoint MirrorPetalGame::cellAt(const QPoint& pos) const
{
    int x = pos.x() - m_boardOffset.x();
    int y = pos.y() - m_boardOffset.y();
    int c = x / m_cellSize;
    int r = y / m_cellSize;
    if (c < 0 || c >= m_gridSize || r < 0 || r >= m_gridSize) return QPoint(-1, -1);
    return QPoint(r, c);
}

void MirrorPetalGame::drawLantern(QPainter& p, const QRect& rect, bool lit, float pulse)
{
    int cx = rect.center().x();
    int cy = rect.center().y();
    float baseR = rect.width() * 0.32f;
    float r = baseR * (1.0f - pulse * 0.15f);

    // 挂钩
    p.setPen(QPen(lit ? QColor(210, 180, 110) : QColor(75, 55, 45), lit ? 3 : 2));
    p.setBrush(Qt::NoBrush);
    int hookW = int(r * 0.6f);
    int hookH = int(r * 0.4f);
    p.drawArc(cx - hookW / 2, int(cy - r * 1.3f), hookW, hookH, 0, 180 * 16);

    // 发光效果
    if (lit) {
        QRadialGradient glow(cx, cy + r * 0.1f, r * 2.8f);
        glow.setColorAt(0, QColor(255, 200, 50, 100));
        glow.setColorAt(0.35, QColor(255, 160, 30, 50));
        glow.setColorAt(1, QColor(255, 100, 10, 0));
        p.setBrush(glow);
        p.setPen(Qt::NoPen);
        p.drawEllipse(cx, cy + r * 0.1f, int(r * 2.8f), int(r * 3.0f));
    }

    // 灯身
    QRectF bodyRect(cx - r, cy - r * 1.05f, r * 2.0f, r * 2.4f);
    if (lit) {
        QRadialGradient body(cx, cy - r * 0.1f, r * 1.3f);
        body.setColorAt(0, QColor(255, 252, 235));
        body.setColorAt(0.3, QColor(255, 235, 155));
        body.setColorAt(0.7, QColor(255, 195, 70));
        body.setColorAt(1, QColor(205, 145, 20));
        p.setBrush(body);
        p.setPen(QPen(QColor(170, 100, 15), 2));
    }
    else {
        QRadialGradient body(cx, cy + r * 0.1f, r);
        body.setColorAt(0, QColor(55, 38, 38));
        body.setColorAt(1, QColor(28, 16, 16));
        p.setBrush(body);
        p.setPen(QPen(QColor(40, 24, 24), 2));
    }
    p.drawEllipse(bodyRect);

    // 装饰横纹
    if (lit) {
        p.setPen(QPen(QColor(230, 195, 80), 2));
    }
    else {
        p.setPen(QPen(QColor(55, 38, 32), 1));
    }
    QRectF bandRect = bodyRect.adjusted(r * 0.15f, r * 0.05f, -r * 0.15f, -r * 0.05f);
    p.drawArc(bandRect, 0, 180 * 16);
    p.drawArc(bandRect, 180 * 16, 180 * 16);

    // 竖纹
    if (lit) {
        p.setPen(QPen(QColor(200, 160, 60, 120), 1));
    }
    else {
        p.setPen(QPen(QColor(45, 28, 28, 100), 1));
    }
    p.drawLine(cx, int(cy - r * 0.95f), cx, int(cy + r * 0.95f));
    p.drawLine(int(cx - r * 0.45f), int(cy - r * 0.5f), int(cx - r * 0.45f), int(cy + r * 0.5f));
    p.drawLine(int(cx + r * 0.45f), int(cy - r * 0.5f), int(cx + r * 0.45f), int(cy + r * 0.5f));

    // 灯芯
    if (lit) {
        p.setBrush(QColor(255, 255, 245));
        p.setPen(Qt::NoPen);
        p.drawEllipse(cx, cy - r * 0.05f, r * 0.28f, r * 0.28f);
        p.setBrush(QColor(255, 210, 80, 200));
        p.drawEllipse(cx, cy - r * 0.1f, r * 0.45f, r * 0.45f);
    }
    else {
        p.setBrush(QColor(45, 28, 28));
        p.setPen(Qt::NoPen);
        p.drawEllipse(cx, cy, r * 0.18f, r * 0.18f);
    }

    // 流苏
    if (lit) {
        p.setPen(QPen(QColor(220, 185, 70), 2));
    }
    else {
        p.setPen(QPen(QColor(55, 38, 32), 1));
    }
    int fy = int(cy + r * 1.15f);
    int flen = int(r * 0.35f);
    p.drawLine(cx, fy, cx, fy + flen);
    p.drawLine(int(cx - r * 0.2f), fy, int(cx - r * 0.1f), fy + int(flen * 0.8f));
    p.drawLine(int(cx + r * 0.2f), fy, int(cx + r * 0.1f), fy + int(flen * 0.8f));
}

void MirrorPetalGame::drawBoard(QPainter& p)
{
    int boardW = m_gridSize * m_cellSize;
    int boardH = m_gridSize * m_cellSize;

    // 阴影
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 140));
    p.drawRoundedRect(m_boardOffset.x() + 8, m_boardOffset.y() + 8, boardW, boardH, 14, 14);

    // 木板底
    QLinearGradient wood(m_boardOffset.x(), m_boardOffset.y(),
        m_boardOffset.x(), m_boardOffset.y() + boardH);
    wood.setColorAt(0, QColor(58, 40, 30));
    wood.setColorAt(1, QColor(38, 25, 18));
    p.setBrush(wood);
    p.setPen(QPen(QColor(115, 85, 55), 4));
    p.drawRoundedRect(m_boardOffset.x(), m_boardOffset.y(), boardW, boardH, 12, 12);

    // 内边框
    p.setPen(QPen(QColor(80, 58, 40), 2));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(m_boardOffset.x() + 6, m_boardOffset.y() + 6,
        boardW - 12, boardH - 12, 8, 8);

    // 网格线
    p.setPen(QPen(QColor(45, 32, 24), 1));
    for (int i = 1; i < m_gridSize; ++i) {
        int x = m_boardOffset.x() + i * m_cellSize;
        p.drawLine(x, m_boardOffset.y() + 8, x, m_boardOffset.y() + boardH - 8);
        int y = m_boardOffset.y() + i * m_cellSize;
        p.drawLine(m_boardOffset.x() + 8, y, m_boardOffset.x() + boardW - 8, y);
    }
}

void MirrorPetalGame::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 背景
    QRadialGradient bgg(QPointF(width() / 2, height() / 2), qMax(width(), height()));
    bgg.setColorAt(0, QColor(42, 26, 36));
    bgg.setColorAt(0.6, QColor(22, 14, 20));
    bgg.setColorAt(1, QColor(10, 6, 10));
    p.fillRect(rect(), bgg);

    // 飘落花瓣
    p.setPen(Qt::NoPen);
    for (int i = 0; i < 15; ++i) {
        float t = ((m_frame + i * 43) % 360) / 360.0f;
        int mx = int(width() * 0.05f + ((i * 97) % int(width() * 0.9f)));
        int my = int(height() * 0.05f + t * height() * 0.88f);
        int alpha = int(20 + 25 * std::sin((m_frame + i * 17) * 0.06));
        p.setBrush(QColor(255, 180, 60, alpha));
        p.drawEllipse(mx, my, 2 + (i % 3), 2 + (i % 3));
    }

    // 标题区
    QRect tr(width() / 2 - 340, 18, 680, 90);
    p.setBrush(QColor(0, 0, 0, 85));
    p.setPen(QPen(QColor(160, 130, 80, 70), 1));
    p.drawRoundedRect(tr, 16, 16);

    p.setPen(QColor(0, 0, 0, 160));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 26, QFont::Bold));
    p.drawText(tr.adjusted(2, 2, 2, 2), Qt::AlignHCenter | Qt::AlignTop,
        QString::fromUtf8(u8"春庭点灯"));
    p.setPen(QColor(255, 235, 200));
    p.drawText(tr, Qt::AlignHCenter | Qt::AlignTop,
        QString::fromUtf8(u8"春庭点灯"));

    p.setPen(QColor(0, 0, 0, 120));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 13));
    p.drawText(tr.adjusted(1, 40, 1, 40), Qt::AlignHCenter | Qt::AlignTop,
        QString::fromUtf8(u8"点击花灯，点亮庭院中的所有灯火"));
    p.setPen(QColor(210, 190, 165));
    p.drawText(tr.adjusted(0, 38, 0, 38), Qt::AlignHCenter | Qt::AlignTop,
        QString::fromUtf8(u8"点击花灯，点亮庭院中的所有灯火"));

    // 点亮计数
    int litCount = countLit();
    int dotY = 116;
    int dotStartX = width() / 2 - m_gridSize * m_gridSize * 14 / 2;
    for (int i = 0; i < m_gridSize * m_gridSize; ++i) {
        QRect dr(dotStartX + i * 28, dotY, 16, 16);
        if (i < litCount) {
            p.setBrush(QColor(255, 210, 80));
            p.setPen(QPen(QColor(200, 160, 50), 2));
            p.drawEllipse(dr);
            p.setBrush(QColor(255, 250, 220));
            p.setPen(Qt::NoPen);
            p.drawEllipse(dr.adjusted(4, 4, -4, -4));
        }
        else {
            p.setBrush(QColor(50, 38, 38));
            p.setPen(QPen(QColor(35, 25, 25), 1));
            p.drawEllipse(dr);
        }
    }
    p.setPen(QColor(185, 165, 140));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 11));
    p.drawText(QRect(width() / 2 - 120, dotY + 22, 240, 20), Qt::AlignCenter,
        QString::fromUtf8(u8"已点亮 %1 / %2").arg(litCount).arg(m_gridSize * m_gridSize));

    // 棋盘
    m_cellSize = qMin(120, qMin(width(), height()) / (m_gridSize + 4));
    int boardW = m_gridSize * m_cellSize;
    int boardH = m_gridSize * m_cellSize;
    m_boardOffset = QPoint((width() - boardW) / 2, (height() - boardH) / 2 + 20);

    drawBoard(p);

    for (int r = 0; r < m_gridSize; ++r) {
        for (int c = 0; c < m_gridSize; ++c) {
            QRect cr = cellRect(r, c).adjusted(6, 6, -6, -6);
            drawLantern(p, cr, m_grid[r][c].lit, m_grid[r][c].pulse);
        }
    }

    // 底部提示
    QRect btm(width() / 2 - 240, height() - 68, 480, 36);
    p.setBrush(QColor(0, 0, 0, 110));
    p.setPen(QPen(QColor(140, 115, 75, 90), 1));
    p.drawRoundedRect(btm, 18, 18);
    p.setPen(QColor(200, 185, 160));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 11));
    p.drawText(btm, Qt::AlignCenter,
        QString::fromUtf8(u8"点击花灯切换亮灭（带动四邻）    ESC 跳过"));
}

void MirrorPetalGame::mousePressEvent(QMouseEvent* event)
{
    if (!m_running) return;
    QPoint cell = cellAt(event->pos());
    if (cell.x() < 0) return;
    applyToggle(cell.x(), cell.y());
    update();
    if (checkWin()) {
        m_running = false;
        releaseKeyboard();
        m_animTimer->stop();
        QTimer::singleShot(600, this, [this]() {
            emit gameFinished(m_itemName);
            hide();
            });
    }
}

void MirrorPetalGame::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        m_running = false;
        releaseKeyboard();
        m_animTimer->stop();
        emit gameSkipped(m_itemName);
        hide();
    }
}

void MirrorPetalGame::showEvent(QShowEvent*)
{
    if (parentWidget()) {
        resize(parentWidget()->size());
        move(0, 0);
    }
}