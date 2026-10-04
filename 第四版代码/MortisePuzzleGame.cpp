#include "MortisePuzzleGame.h"
#include "ResourceManager.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <cmath>

MortisePuzzleGame::MortisePuzzleGame(QWidget* parent) : QWidget(parent),
m_running(false), m_frame(0), m_winGlow(0.0f), m_cellSize(60), m_hintSpace(50)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setFocusPolicy(Qt::StrongFocus);

    m_animTimer = new QTimer(this);
    connect(m_animTimer, &QTimer::timeout, this, [this]() {
        m_frame++;
        updateAnimations();
        update();
        });
    hide();
}

void MortisePuzzleGame::startGame(const QString& itemName, const QString& bgName)
{
    m_itemName = itemName;
    m_running = true;
    m_frame = 0;
    m_winGlow = 0.0f;

    if (!bgName.isEmpty())
        m_bgPixmap = ResourceManager::instance()->getBackground(bgName);

    resize(parentWidget() ? parentWidget()->size() : QSize(1280, 720));
    move(0, 0);
    initPuzzle();
    show(); raise(); grabKeyboard();
    m_animTimer->start(30);
}

void MortisePuzzleGame::initPuzzle()
{
    // 旧簪轮廓图案 (1=填充)
    //   . X X X .
    //   X X X X X
    //   . X X X .
    //   . . X . .
    //   . . X . .
    bool sol[SIZE][SIZE] = {
        { false, true,  true,  true,  false },
        { true,  true,  true,  true,  true  },
        { false, true,  true,  true,  false },
        { false, false, true,  false, false },
        { false, false, true,  false, false }
    };
    for (int r = 0; r < SIZE; ++r)
        for (int c = 0; c < SIZE; ++c)
            m_solution[r][c] = sol[r][c];

    for (int r = 0; r < SIZE; ++r)
        for (int c = 0; c < SIZE; ++c)
            m_grid[r][c] = 0;

    // 生成行提示
    m_rowHints.clear();
    for (int r = 0; r < SIZE; ++r) {
        QList<int> hints;
        int cnt = 0;
        for (int c = 0; c < SIZE; ++c) {
            if (m_solution[r][c]) {
                cnt++;
            }
            else {
                if (cnt > 0) { hints.append(cnt); cnt = 0; }
            }
        }
        if (cnt > 0) hints.append(cnt);
        if (hints.isEmpty()) hints.append(0);
        m_rowHints.append(hints);
    }

    // 生成列提示
    m_colHints.clear();
    for (int c = 0; c < SIZE; ++c) {
        QList<int> hints;
        int cnt = 0;
        for (int r = 0; r < SIZE; ++r) {
            if (m_solution[r][c]) {
                cnt++;
            }
            else {
                if (cnt > 0) { hints.append(cnt); cnt = 0; }
            }
        }
        if (cnt > 0) hints.append(cnt);
        if (hints.isEmpty()) hints.append(0);
        m_colHints.append(hints);
    }
}

QRect MortisePuzzleGame::cellRect(int row, int col) const
{
    int x = m_boardOffset.x() + col * m_cellSize;
    int y = m_boardOffset.y() + row * m_cellSize;
    return QRect(x, y, m_cellSize, m_cellSize);
}

int MortisePuzzleGame::cellAt(const QPoint& pos, int& outRow, int& outCol) const
{
    int relX = pos.x() - m_boardOffset.x();
    int relY = pos.y() - m_boardOffset.y();
    int c = relX / m_cellSize;
    int r = relY / m_cellSize;
    if (c < 0 || c >= SIZE || r < 0 || r >= SIZE) return -1;
    outRow = r; outCol = c;
    return r * SIZE + c;
}

void MortisePuzzleGame::toggleCell(int row, int col)
{
    m_grid[row][col] = (m_grid[row][col] + 1) % 3;
    update();
    if (checkWin()) {
        m_running = false;
        releaseKeyboard();
        QTimer::singleShot(800, this, [this]() {
            m_animTimer->stop();
            emit gameFinished(m_itemName);
            hide();
            });
    }
}

bool MortisePuzzleGame::checkWin() const
{
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            bool shouldFill = m_solution[r][c];
            bool isFilled = (m_grid[r][c] == 1);
            if (shouldFill != isFilled) return false;
        }
    }
    return true;
}

void MortisePuzzleGame::updateAnimations()
{
    if (!m_running && m_winGlow < 1.0f) m_winGlow += 0.025f;
}

void MortisePuzzleGame::drawBackground(QPainter& p)
{
    QRadialGradient bg(QPointF(width() / 2, height() / 2), qMax(width(), height()));
    bg.setColorAt(0, QColor(48, 38, 32));
    bg.setColorAt(0.6, QColor(28, 20, 16));
    bg.setColorAt(1, QColor(14, 8, 6));
    p.fillRect(rect(), bg);

    if (!m_bgPixmap.isNull()) {
        p.setOpacity(0.05);
        p.drawPixmap(rect(), m_bgPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        p.setOpacity(1.0);
    }
}

void MortisePuzzleGame::drawHints(QPainter& p)
{
    p.setPen(QColor(220, 200, 170));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 11, QFont::Bold));

    // 行提示（左侧）
    for (int r = 0; r < SIZE; ++r) {
        QString text;
        for (int i = 0; i < m_rowHints[r].size(); ++i) {
            if (i > 0) text += " ";
            text += QString::number(m_rowHints[r][i]);
        }
        QRect hr(m_boardOffset.x() - m_hintSpace + 5, m_boardOffset.y() + r * m_cellSize, m_hintSpace - 10, m_cellSize);
        p.drawText(hr, Qt::AlignRight | Qt::AlignVCenter, text);
    }

    // 列提示（上方）
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 10, QFont::Bold));
    for (int c = 0; c < SIZE; ++c) {
        QString text;
        for (int i = 0; i < m_colHints[c].size(); ++i) {
            if (i > 0) text += "\n";
            text += QString::number(m_colHints[c][i]);
        }
        QRect hc(m_boardOffset.x() + c * m_cellSize, m_boardOffset.y() - m_hintSpace + 4, m_cellSize, m_hintSpace - 8);
        p.drawText(hc, Qt::AlignCenter | Qt::AlignBottom, text);
    }
}

void MortisePuzzleGame::drawGrid(QPainter& p)
{
    int boardW = SIZE * m_cellSize;
    int boardH = SIZE * m_cellSize;

    // 背景板
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 100));
    p.drawRoundedRect(m_boardOffset.x() + 4, m_boardOffset.y() + 4, boardW, boardH, 8, 8);

    p.setBrush(QColor(55, 42, 35));
    p.setPen(QPen(QColor(90, 70, 55), 2));
    p.drawRoundedRect(m_boardOffset.x(), m_boardOffset.y(), boardW, boardH, 6, 6);
}

void MortisePuzzleGame::drawCell(QPainter& p, int r, int c, const QRect& rect)
{
    int state = m_grid[r][c];

    // 底色
    if (state == 1) {
        // 已填充：暗红木色
        p.setBrush(QColor(140, 60, 50));
        p.setPen(QPen(QColor(100, 40, 35), 1));
    }
    else if (state == 2) {
        // 标记为空：淡灰叉
        p.setBrush(QColor(45, 38, 32));
        p.setPen(QPen(QColor(80, 70, 60), 1));
    }
    else {
        // 空白
        p.setBrush(QColor(65, 52, 42));
        p.setPen(QPen(QColor(85, 68, 55), 1));
    }
    p.drawRect(rect);

    if (state == 1) {
        // 填充格：木纹效果
        p.setPen(QPen(QColor(160, 80, 70, 60), 1));
        p.drawLine(rect.left() + 4, rect.top() + 8, rect.right() - 4, rect.bottom() - 8);
        p.drawLine(rect.left() + 4, rect.bottom() - 8, rect.right() - 4, rect.top() + 8);
    }
    else if (state == 2) {
        // 叉号
        p.setPen(QPen(QColor(120, 100, 85), 2));
        int margin = 8;
        p.drawLine(rect.left() + margin, rect.top() + margin, rect.right() - margin, rect.bottom() - margin);
        p.drawLine(rect.right() - margin, rect.top() + margin, rect.left() + margin, rect.bottom() - margin);
    }

    // 5x5 分隔粗线（每5格，虽然这里只有5格）
    if (r == 0 || c == 0) {
        p.setPen(QPen(QColor(160, 130, 100), 2));
        if (r == 0) p.drawLine(rect.left(), rect.top(), rect.right(), rect.top());
        if (c == 0) p.drawLine(rect.left(), rect.top(), rect.left(), rect.bottom());
    }
}

void MortisePuzzleGame::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    drawBackground(p);

    // 标题
    p.setPen(QColor(230, 215, 185));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 22, QFont::Bold));
    p.drawText(QRect(0, 18, width(), 50), Qt::AlignCenter,
        QString::fromUtf8(u8"旧簪纹 · 数织迷阵"));

    p.setPen(QColor(180, 165, 145));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 13));
    p.drawText(QRect(0, 64, width(), 28), Qt::AlignCenter,
        QString::fromUtf8(u8"根据行列数字提示，推理出哪些格子应该填充（左键点击循环：空→填→叉）"));

    // 计算布局
    m_cellSize = qMin(90, qMin(width() - 200, height() - 220) / SIZE);
    m_hintSpace = qMin(70, m_cellSize + 10);
    int boardW = SIZE * m_cellSize;
    int boardH = SIZE * m_cellSize;
    m_boardOffset = QPoint((width() - boardW - m_hintSpace) / 2 + m_hintSpace,
        (height() - boardH - m_hintSpace) / 2 + m_hintSpace);

    // 绘制提示数字
    drawHints(p);

    // 绘制棋盘
    drawGrid(p);

    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            drawCell(p, r, c, cellRect(r, c));
        }
    }

    // 胜利效果
    if (!m_running && m_winGlow > 0.01f) {
        int cx = m_boardOffset.x() + boardW;
            int cy = m_boardOffset.y() + boardH / 2;
        QRadialGradient winGlow(cx, cy, boardW);
        winGlow.setColorAt(0, QColor(255, 240, 200, int(70 * m_winGlow)));
        winGlow.setColorAt(0.5, QColor(255, 220, 140, int(35 * m_winGlow)));
        winGlow.setColorAt(1, QColor(255, 200, 100, 0));
        p.setBrush(winGlow);
        p.setPen(Qt::NoPen);
        p.drawEllipse(cx, cy, boardW, boardH);

        p.setPen(QColor(255, 240, 200, int(255 * m_winGlow)));
        p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 32, QFont::Bold));
        p.drawText(QRect(0, height() / 2 - 30, width(), 50), Qt::AlignCenter,
            QString::fromUtf8(u8"旧簪复原"));

        p.setPen(QColor(255, 230, 170, int(255 * m_winGlow)));
        p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 15));
        p.drawText(QRect(0, height() / 2 + 25, width(), 30), Qt::AlignCenter,
            QString::fromUtf8(u8"获得【春时旧簪】"));
    }

    // 底部提示
    if (m_running) {
        QRect hintR(width() / 2 - 280, height() - 65, 560, 38);
        p.setBrush(QColor(0, 0, 0, 90));
        p.setPen(QPen(QColor(140, 120, 90, 80), 1));
        p.drawRoundedRect(hintR, 19, 19);
        p.setPen(QColor(200, 185, 160));
        p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 11));
        p.drawText(hintR, Qt::AlignCenter,
            QString::fromUtf8(u8"左键点击循环：空白 → 填充 → 叉号    ESC 跳过"));
    }
}

void MortisePuzzleGame::mousePressEvent(QMouseEvent* event)
{
    if (!m_running) return;
    int r, c;
    if (cellAt(event->pos(), r, c) >= 0) {
        toggleCell(r, c);
    }
}

void MortisePuzzleGame::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        m_running = false;
        releaseKeyboard();
        m_animTimer->stop();
        emit gameSkipped(m_itemName);
        hide();
    }
}

void MortisePuzzleGame::showEvent(QShowEvent*)
{
    if (parentWidget()) {
        resize(parentWidget()->size());
        move(0, 0);
    }
}