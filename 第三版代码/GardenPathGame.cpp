#include "GardenPathGame.h"
#include "ResourceManager.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QTimer>
#include <QQueue>
#include <cmath>

GardenPathGame::GardenPathGame(QWidget* parent) : QWidget(parent),
m_running(false), m_waterFlowing(false), m_cellSize(90),
m_rows(5), m_cols(5), m_frame(0)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setFocusPolicy(Qt::StrongFocus);
    hide();
}

void GardenPathGame::startGame(const QString& itemName, const QString& bgName)
{
    m_itemName = itemName;
    m_running = true;
    m_waterFlowing = false;
    m_frame = 0;

    if (!bgName.isEmpty())
        m_bgPixmap = ResourceManager::instance()->getBackground(bgName);

    resize(parentWidget() ? parentWidget()->size() : QSize(1280, 720));
    move(0, 0);
    initPuzzle();

    show(); raise(); grabKeyboard();
}

void GardenPathGame::initPuzzle()
{
    m_grid.clear();
    m_rows = 5; m_cols = 5;
    m_startPos = QPoint(0, 0);
    m_endPos = QPoint(m_cols - 1, m_rows - 1);

    generatePuzzle();

    int boardW = m_cols * m_cellSize;
    int boardH = m_rows * m_cellSize;
    m_boardOffset = QPoint((width() - boardW) / 2, (height() - boardH) / 2 + 10);
}

void GardenPathGame::generatePuzzle()
{
    // ← 修复：不用 resize，改用 append 初始化
    m_grid.clear();
    for (int r = 0; r < m_rows; ++r) {
        QList<PipeCell> row;
        for (int c = 0; c < m_cols; ++c) {
            PipeCell cell;
            for (int d = 0; d < 4; ++d) cell.open[d] = false;
            cell.fixed = false;
            cell.flooded = false;
            cell.type = 0;
            cell.rotation = 0;
            row.append(cell);
        }
        m_grid.append(row);
    }

    // 生成单调路径（只能右/下），保证可达
    QList<QPoint> path;
    QPoint cur = m_startPos;
    path.append(cur);
    while (cur != m_endPos) {
        QList<QPoint> moves;
        if (cur.x() < m_endPos.x()) moves.append(QPoint(cur.x() + 1, cur.y()));
        if (cur.y() < m_endPos.y()) moves.append(QPoint(cur.x(), cur.y() + 1));
        if (moves.isEmpty()) break;
        cur = moves[QRandomGenerator::global()->bounded(moves.size())];
        path.append(cur);
    }

    // 根据路径设置开口
    for (int i = 0; i < path.size(); ++i) {
        int r = path[i].y(), c = path[i].x();
        if (i > 0) {
            QPoint prev = path[i - 1];
            if (prev.y() < r) m_grid[r][c].open[0] = true;
            else if (prev.y() > r) m_grid[r][c].open[2] = true;
            else if (prev.x() < c) m_grid[r][c].open[3] = true;
            else if (prev.x() > c) m_grid[r][c].open[1] = true;
        }
        if (i < path.size() - 1) {
            QPoint next = path[i + 1];
            if (next.y() < r) m_grid[r][c].open[0] = true;
            else if (next.y() > r) m_grid[r][c].open[2] = true;
            else if (next.x() < c) m_grid[r][c].open[3] = true;
            else if (next.x() > c) m_grid[r][c].open[1] = true;
        }
    }

    // 非路径格子：随机填充并打乱；其中 2~3 个变为障碍
    QSet<QString> pathSet;
    for (auto pt : path) pathSet.insert(QString("%1,%2").arg(pt.y()).arg(pt.x()));

    int obstacleCount = 0;
    int maxObstacles = 2 + QRandomGenerator::global()->bounded(2); // 2~3

    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            QString key = QString("%1,%2").arg(r).arg(c);
            if (pathSet.contains(key)) continue;

            if (obstacleCount < maxObstacles && QRandomGenerator::global()->bounded(100) < 35) {
                for (int d = 0; d < 4; ++d) m_grid[r][c].open[d] = false;
                m_grid[r][c].fixed = true;
                m_grid[r][c].type = 4;
                obstacleCount++;
                continue;
            }

            int t = QRandomGenerator::global()->bounded(6);
            if (t == 0) { m_grid[r][c].open[0] = true; m_grid[r][c].open[2] = true; }
            else if (t == 1) { m_grid[r][c].open[1] = true; m_grid[r][c].open[3] = true; }
            else if (t == 2) { m_grid[r][c].open[0] = true; m_grid[r][c].open[1] = true; }
            else if (t == 3) { m_grid[r][c].open[0] = true; m_grid[r][c].open[3] = true; }
            else if (t == 4) { m_grid[r][c].open[0] = true; m_grid[r][c].open[1] = true; m_grid[r][c].open[2] = true; }
            else { m_grid[r][c].open[0] = true; m_grid[r][c].open[1] = true; m_grid[r][c].open[2] = true; m_grid[r][c].open[3] = true; }

            m_grid[r][c].fixed = false;
        }
    }

    // 推断类型并随机旋转
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            if (m_grid[r][c].type == 4) continue;
            int cnt = 0;
            for (int d = 0; d < 4; ++d) if (m_grid[r][c].open[d]) cnt++;
            if (cnt == 2) {
                if ((m_grid[r][c].open[0] && m_grid[r][c].open[2]) ||
                    (m_grid[r][c].open[1] && m_grid[r][c].open[3]))
                    m_grid[r][c].type = 0;
                else
                    m_grid[r][c].type = 1;
            }
            else if (cnt == 3) {
                m_grid[r][c].type = 2;
            }
            else if (cnt == 4) {
                m_grid[r][c].type = 3;
            }

            if (!m_grid[r][c].fixed) {
                int rot = QRandomGenerator::global()->bounded(4);
                for (int k = 0; k < rot; ++k) {
                    bool tmp[4];
                    tmp[0] = m_grid[r][c].open[3];
                    tmp[1] = m_grid[r][c].open[0];
                    tmp[2] = m_grid[r][c].open[1];
                    tmp[3] = m_grid[r][c].open[2];
                    for (int d = 0; d < 4; ++d) m_grid[r][c].open[d] = tmp[d];
                }
                m_grid[r][c].rotation = rot;
            }
        }
    }

    // 起点
    m_grid[m_startPos.y()][m_startPos.x()].fixed = true;
    m_grid[m_startPos.y()][m_startPos.x()].type = 5;
    for (int d = 0; d < 4; ++d) m_grid[m_startPos.y()][m_startPos.x()].open[d] = false;
    if (path.size() > 1) {
        QPoint nxt = path[1];
        int r = m_startPos.y(), c = m_startPos.x();
        if (nxt.y() > r) m_grid[r][c].open[2] = true;
        else if (nxt.x() > c) m_grid[r][c].open[1] = true;
    }

    // 终点
    m_grid[m_endPos.y()][m_endPos.x()].fixed = true;
    m_grid[m_endPos.y()][m_endPos.x()].type = 6;
    for (int d = 0; d < 4; ++d) m_grid[m_endPos.y()][m_endPos.x()].open[d] = false;
    if (path.size() > 1) {
        QPoint prv = path[path.size() - 2];
        int r = m_endPos.y(), c = m_endPos.x();
        if (prv.y() < r) m_grid[r][c].open[0] = true;
        else if (prv.x() < c) m_grid[r][c].open[3] = true;
    }
}

void GardenPathGame::rotateCell(int row, int col)
{
    if (m_grid[row][col].fixed) return;
    bool tmp[4];
    tmp[0] = m_grid[row][col].open[3];
    tmp[1] = m_grid[row][col].open[0];
    tmp[2] = m_grid[row][col].open[1];
    tmp[3] = m_grid[row][col].open[2];
    for (int d = 0; d < 4; ++d) m_grid[row][col].open[d] = tmp[d];
    m_grid[row][col].rotation = (m_grid[row][col].rotation + 1) % 4;
}

void GardenPathGame::updateWaterFlow()
{
    for (int r = 0; r < m_rows; ++r)
        for (int c = 0; c < m_cols; ++c)
            m_grid[r][c].flooded = false;

    QQueue<QPoint> q;
    m_grid[m_startPos.y()][m_startPos.x()].flooded = true;
    q.enqueue(m_startPos);

    int dr[4] = { -1, 0, 1, 0 };
    int dc[4] = { 0, 1, 0, -1 };

    while (!q.isEmpty()) {
        QPoint cur = q.dequeue();
        int r = cur.y(), c = cur.x();
        for (int d = 0; d < 4; ++d) {
            if (!m_grid[r][c].open[d]) continue;
            int nr = r + dr[d], nc = c + dc[d];
            if (nr < 0 || nr >= m_rows || nc < 0 || nc >= m_cols) continue;
            int opp = (d + 2) % 4;
            if (m_grid[nr][nc].open[opp] && !m_grid[nr][nc].flooded) {
                m_grid[nr][nc].flooded = true;
                q.enqueue(QPoint(nc, nr));
            }
        }
    }
}

void GardenPathGame::checkWin()
{
    if (m_grid[m_endPos.y()][m_endPos.x()].flooded && !m_waterFlowing) {
        m_waterFlowing = true;
        QTimer::singleShot(1800, this, [this]() {
            m_running = false;
            releaseKeyboard();
            emit gameFinished(m_itemName);
            hide();
            });
    }
}

QRect GardenPathGame::cellRect(int row, int col) const
{
    return QRect(m_boardOffset.x() + col * m_cellSize,
        m_boardOffset.y() + row * m_cellSize,
        m_cellSize, m_cellSize);
}

void GardenPathGame::drawChannel(QPainter& p, const PipeCell& cell, const QRect& rect, bool water)
{
    int cx = rect.center().x();
    int cy = rect.center().y();
    int arm = m_cellSize / 2 - 8;
    int w = water ? 10 : 16;

    p.setPen(Qt::NoPen);
    if (water) {
        p.setBrush(QColor(50, 180, 155, 170));
    }
    else {
        p.setBrush(QColor(16, 26, 22));
    }

    for (int d = 0; d < 4; ++d) {
        if (!cell.open[d]) continue;
        QRect tube;
        if (d == 0) tube = QRect(cx - w / 2, cy - arm, w, arm);
        else if (d == 1) tube = QRect(cx, cy - w / 2, arm, w);
        else if (d == 2) tube = QRect(cx - w / 2, cy, w, arm);
        else if (d == 3) tube = QRect(cx - arm, cy - w / 2, arm, w);
        p.drawRoundedRect(tube, w / 2, w / 2);
    }
    p.drawEllipse(cx - w / 2, cy - w / 2, w, w);

    if (water) {
        float t = (m_frame * 0.12f + cx * 0.01f + cy * 0.01f);
        float pulse = 0.5f + 0.5f * std::sin(t);
        p.setBrush(QColor(200, 255, 240, int(100 * pulse)));
        p.drawEllipse(cx - 4, cy - 4, 8, 8);

        p.setBrush(QColor(220, 255, 245, 140));
        for (int d = 0; d < 4; ++d) {
            if (!cell.open[d]) continue;
            int dist = int(arm * 0.6f + 6 * std::sin(t + d * 1.57f));
            int dx = 0, dy = 0;
            if (d == 0) dy = -dist;
            else if (d == 1) dx = dist;
            else if (d == 2) dy = dist;
            else if (d == 3) dx = -dist;
            p.drawEllipse(cx + dx - 3, cy + dy - 3, 6, 6);
        }
    }
    else {
        p.setPen(QPen(QColor(70, 85, 78), 1));
        p.setBrush(Qt::NoBrush);
        for (int d = 0; d < 4; ++d) {
            if (!cell.open[d]) continue;
            if (d == 0) p.drawLine(cx - w / 2 - 2, cy - arm, cx - w / 2 - 2, cy);
            else if (d == 1) p.drawLine(cx, cy - w / 2 - 2, cx + arm, cy - w / 2 - 2);
            else if (d == 2) p.drawLine(cx - w / 2 - 2, cy, cx - w / 2 - 2, cy + arm);
            else if (d == 3) p.drawLine(cx - arm, cy - w / 2 - 2, cx, cy - w / 2 - 2);
        }
    }
}

void GardenPathGame::drawCell(QPainter& p, int row, int col, const QRect& rect)
{
    const PipeCell& cell = m_grid[row][col];

    QRadialGradient stone(rect.center(), rect.width() * 0.7);
    stone.setColorAt(0, QColor(58, 70, 66));
    stone.setColorAt(1, QColor(38, 50, 46));
    p.setBrush(stone);
    p.setPen(QPen(QColor(26, 36, 32), 2));
    p.drawRoundedRect(rect.adjusted(2, 2, -2, -2), 10, 10);

    if (cell.type == 4) {
        p.setBrush(QColor(62, 48, 32));
        p.setPen(QPen(QColor(42, 32, 22), 2));
        int cx = rect.center().x(), cy = rect.center().y();
        p.drawEllipse(cx - 20, cy - 16, 40, 32);
        p.setPen(QPen(QColor(90, 72, 52), 1));
        p.drawLine(cx - 8, cy - 6, cx + 6, cy + 4);
        p.drawLine(cx - 6, cy + 8, cx + 4, cy - 6);
        p.setBrush(QColor(60, 80, 55, 120));
        p.setPen(Qt::NoPen);
        p.drawEllipse(cx - 14, cy + 4, 12, 8);
        return;
    }

    drawChannel(p, cell, rect, false);
    if (cell.flooded) drawChannel(p, cell, rect, true);

    if (cell.type == 5) {
        int cx = rect.center().x(), cy = rect.center().y();
        float ripple = 8 + 4 * std::sin(m_frame * 0.1f);
        QRadialGradient grad(cx, cy, ripple + 12);
        grad.setColorAt(0, QColor(180, 255, 245, 200));
        grad.setColorAt(0.5, QColor(80, 200, 180, 100));
        grad.setColorAt(1, QColor(60, 180, 160, 0));
        p.setBrush(grad);
        p.setPen(Qt::NoPen);
        p.drawEllipse(cx, cy, int(ripple + 12), int(ripple + 12));
        p.setBrush(QColor(220, 255, 250));
        p.drawEllipse(cx - 6, cy - 6, 12, 12);
        p.setPen(QColor(255, 255, 255));
        p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 9, QFont::Bold));
        p.drawText(rect, Qt::AlignCenter, QString::fromUtf8(u8"泉"));
    }

    if (cell.type == 6) {
        int cx = rect.center().x(), cy = rect.center().y();
        if (cell.flooded) {
            QRadialGradient grad(cx, cy, 30);
            grad.setColorAt(0, QColor(255, 230, 150, 180));
            grad.setColorAt(1, QColor(255, 200, 80, 0));
            p.setBrush(grad);
            p.setPen(Qt::NoPen);
            p.drawEllipse(cx, cy, 30, 30);
        }
        p.setBrush(cell.flooded ? QColor(255, 220, 120) : QColor(120, 110, 80));
        p.setPen(QPen(cell.flooded ? QColor(255, 240, 180) : QColor(80, 75, 55), 2));
        p.drawEllipse(cx - 10, cy - 10, 20, 20);
        p.setPen(QColor(255, 255, 255));
        p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 9, QFont::Bold));
        p.drawText(rect, Qt::AlignCenter, QString::fromUtf8(u8"田"));
    }
}

void GardenPathGame::drawBackground(QPainter& p)
{
    QRadialGradient bg(QPointF(width() / 2, height() / 2), qMax(width(), height()));
    bg.setColorAt(0, QColor(30, 42, 38));
    bg.setColorAt(0.6, QColor(16, 24, 20));
    bg.setColorAt(1, QColor(8, 14, 10));
    p.fillRect(rect(), bg);

    if (!m_bgPixmap.isNull()) {
        p.setOpacity(0.05);
        p.drawPixmap(rect(), m_bgPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        p.setOpacity(1.0);
    }

    p.setPen(Qt::NoPen);
    for (int i = 0; i < 20; ++i) {
        int x = (i * 137) % width();
        int y = (i * 97) % height();
        p.setBrush(QColor(40, 70, 45, 30));
        p.drawEllipse(x, y, 40, 25);
    }
}

void GardenPathGame::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    m_frame++;

    updateWaterFlow();
    checkWin();

    drawBackground(p);

    QRect titleR(width() / 2 - 360, 22, 720, 55);
    p.setPen(QColor(0, 0, 0, 150));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 24, QFont::Bold));
    p.drawText(titleR.adjusted(2, 2, 2, 2), Qt::AlignCenter,
        QString::fromUtf8(u8"曲水通渠 · 接通泉眼与花田"));
    p.setPen(QColor(200, 230, 210));
    p.drawText(titleR, Qt::AlignCenter,
        QString::fromUtf8(u8"曲水通渠 · 接通泉眼与花田"));

    p.setPen(QColor(170, 200, 180));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 14));
    p.drawText(QRect(0, 78, width(), 30), Qt::AlignCenter,
        QString::fromUtf8(u8"点击石板旋转渠道，让水流从泉眼抵达玉盏"));

    m_cellSize = qMin(110, qMin(width(), height()) / (m_rows + 4));
    int boardW = m_cols * m_cellSize;
    int boardH = m_rows * m_cellSize;
    m_boardOffset = QPoint((width() - boardW) / 2, (height() - boardH) / 2 + 20);

    p.fillRect(m_boardOffset.x() - 8, m_boardOffset.y() - 8,
        boardW + 16, boardH + 16, QColor(28, 40, 34));

    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            drawCell(p, r, c, cellRect(r, c));
        }
    }

    if (m_waterFlowing) {
        p.setBrush(QColor(180, 255, 230, 40));
        p.setPen(Qt::NoPen);
        for (int r = 0; r < m_rows; ++r) {
            for (int c = 0; c < m_cols; ++c) {
                if (m_grid[r][c].flooded) {
                    QRect rct = cellRect(r, c);
                    p.drawRoundedRect(rct.adjusted(4, 4, -4, -4), 8, 8);
                }
            }
        }
        p.setPen(QColor(255, 245, 200));
        p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 28, QFont::Bold));
        p.drawText(QRect(0, height() / 2 - 30, width(), 50), Qt::AlignCenter,
            QString::fromUtf8(u8"渠通水至，万物生焉"));
    }

    if (!m_waterFlowing) {
        QRect hintR(width() / 2 - 300, height() - 70, 600, 38);
        p.setBrush(QColor(0, 0, 0, 110));
        p.setPen(QPen(QColor(100, 130, 110, 90), 1));
        p.drawRoundedRect(hintR, 19, 19);
        p.setPen(QColor(190, 220, 200));
        p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 11));
        p.drawText(hintR, Qt::AlignCenter,
            QString::fromUtf8(u8"点击旋转石板    枯木不可动    ESC 跳过"));
    }
}

void GardenPathGame::mousePressEvent(QMouseEvent* event)
{
    if (!m_running || m_waterFlowing) return;
    int relX = event->pos().x() - m_boardOffset.x();
    int relY = event->pos().y() - m_boardOffset.y();
    int c = relX / m_cellSize;
    int r = relY / m_cellSize;
    if (c < 0 || c >= m_cols || r < 0 || r >= m_rows) return;
    rotateCell(r, c);
    update();
}

void GardenPathGame::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        m_running = false;
        releaseKeyboard();
        emit gameSkipped(m_itemName);
        hide();
    }
}

void GardenPathGame::showEvent(QShowEvent*)
{
    if (parentWidget()) {
        resize(parentWidget()->size());
        move(0, 0);
    }
    m_cellSize = qMin(110, qMin(width(), height()) / (m_rows + 4));
    int boardW = m_cols * m_cellSize;
    int boardH = m_rows * m_cellSize;
    m_boardOffset = QPoint((width() - boardW) / 2, (height() - boardH) / 2 + 20);
}