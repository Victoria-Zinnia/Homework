#include "PorcelainRestoreGame.h"
#include "ResourceManager.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

PorcelainRestoreGame::PorcelainRestoreGame(QWidget* parent) : QWidget(parent),
m_running(false), m_selectedCell(-1, -1), m_cellSize(60), m_frame(0), m_winGlow(0.0f)
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

void PorcelainRestoreGame::startGame(const QString& itemName, const QString& bgName)
{
    m_itemName = itemName;
    m_running = true;
    m_selectedCell = QPoint(-1, -1);
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

void PorcelainRestoreGame::initPuzzle()
{
    for (int r = 0; r < GRID; ++r)
        for (int c = 0; c < GRID; ++c)
            m_grid[r][c].type = 0;

    // 外围冰墙
    for (int i = 0; i < GRID; ++i) {
        m_grid[0][i].type = 1;
        m_grid[GRID - 1][i].type = 1;
        m_grid[i][0].type = 1;
        m_grid[i][GRID - 1].type = 1;
    }

    // 内部障碍
    m_grid[2][2].type = 1; m_grid[2][3].type = 1;
    m_grid[4][4].type = 1;
    m_grid[6][2].type = 1; m_grid[6][4].type = 1;
    m_grid[3][5].type = 1;

    // 雪团
    m_grid[1][1].type = 2;
    m_grid[3][3].type = 2;
    m_grid[5][5].type = 2;

    // 梅花坑
    m_grid[1][6].type = 3;
    m_grid[5][6].type = 3;
    m_grid[6][1].type = 3;

    m_cellSize = qMin(70, qMin(width(), height()) / (GRID + 4));
    int boardW = GRID * m_cellSize;
    int boardH = GRID * m_cellSize;
    m_boardOffset = QPoint((width() - boardW) / 2, (height() - boardH) / 2 + 20);

    m_snowflakes.clear();
    m_snowSpeed.clear();
    for (int i = 0; i < 50; ++i) {
        m_snowflakes.append(QPointF(QRandomGenerator::global()->bounded(width()),
            QRandomGenerator::global()->bounded(height())));
        m_snowSpeed.append(0.3f + QRandomGenerator::global()->bounded(100) / 150.0f);
    }
}

QPoint PorcelainRestoreGame::cellAt(const QPoint& pos) const
{
    int x = pos.x() - m_boardOffset.x();
    int y = pos.y() - m_boardOffset.y();
    int c = x / m_cellSize;
    int r = y / m_cellSize;
    if (c < 0 || c >= GRID || r < 0 || r >= GRID) return QPoint(-1, -1);
    return QPoint(r, c);
}

QRect PorcelainRestoreGame::cellRect(int row, int col) const
{
    int x = m_boardOffset.x() + col * m_cellSize;
    int y = m_boardOffset.y() + row * m_cellSize;
    return QRect(x, y, m_cellSize, m_cellSize);
}

void PorcelainRestoreGame::trySlide(int row, int col)
{
    if (m_selectedCell.x() < 0) return;
    int sr = m_selectedCell.x();
    int sc = m_selectedCell.y();
    if (m_grid[sr][sc].type != 2) return;

    int dr = 0, dc = 0;
    if (row == sr && col > sc) dc = 1;
    else if (row == sr && col < sc) dc = -1;
    else if (col == sc && row > sr) dr = 1;
    else if (col == sc && row < sr) dr = -1;
    else return;

    int nr = sr + dr;
    int nc = sc + dc;

    while (nr >= 0 && nr < GRID && nc >= 0 && nc < GRID) {
        int t = m_grid[nr][nc].type;
        if (t == 1 || t == 2 || t == 4) break;
        if (t == 3) {
            m_grid[sr][sc].type = 0;
            m_grid[nr][nc].type = 4;
            m_selectedCell = QPoint(-1, -1);
            if (checkWin()) {
                m_running = false;
                releaseKeyboard();
                m_animTimer->stop();
                QTimer::singleShot(800, this, [this]() {
                    emit gameFinished(m_itemName);
                    hide();
                    });
            }
            return;
        }
        nr += dr;
        nc += dc;
    }

    int finalR = nr - dr;
    int finalC = nc - dc;
    if (finalR == sr && finalC == sc) return;

    m_grid[sr][sc].type = 0;
    m_grid[finalR][finalC].type = 2;
    m_selectedCell = QPoint(-1, -1);
}

bool PorcelainRestoreGame::checkWin() const
{
    for (int r = 0; r < GRID; ++r)
        for (int c = 0; c < GRID; ++c)
            if (m_grid[r][c].type == 3) return false;
    return true;
}

void PorcelainRestoreGame::updateAnimations()
{
    for (int i = 0; i < m_snowflakes.size(); ++i) {
        m_snowflakes[i].setY(m_snowflakes[i].y() + m_snowSpeed[i]);
        m_snowflakes[i].setX(m_snowflakes[i].x() + std::sin(m_frame * 0.015 + i) * 0.4);
        if (m_snowflakes[i].y() > height()) {
            m_snowflakes[i].setY(-5);
            m_snowflakes[i].setX(QRandomGenerator::global()->bounded(width()));
        }
    }
    if (!m_running && m_winGlow < 1.0f) m_winGlow += 0.02f;
}

void PorcelainRestoreGame::drawBackground(QPainter& p)
{
    QRadialGradient bg(QPointF(width() / 2, height() / 2), qMax(width(), height()));
    bg.setColorAt(0, QColor(35, 30, 45));
    bg.setColorAt(0.6, QColor(18, 15, 25));
    bg.setColorAt(1, QColor(8, 6, 12));
    p.fillRect(rect(), bg);

    if (!m_bgPixmap.isNull()) {
        p.setOpacity(0.05);
        p.drawPixmap(rect(), m_bgPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        p.setOpacity(1.0);
    }
}

void PorcelainRestoreGame::drawSnow(QPainter& p)
{
    p.setPen(Qt::NoPen);
    for (int i = 0; i < m_snowflakes.size(); ++i) {
        int alpha = 60 + int(40 * std::sin(m_frame * 0.04 + i * 0.5));
        p.setBrush(QColor(255, 255, 255, alpha));
        int sz = 2 + (i % 4);
        p.drawEllipse(int(m_snowflakes[i].x()), int(m_snowflakes[i].y()), sz, sz);
    }
}

void PorcelainRestoreGame::drawGrid(QPainter& p)
{
    int boardW = GRID * m_cellSize;
    int boardH = GRID * m_cellSize;

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 120));
    p.drawRoundedRect(m_boardOffset.x() + 6, m_boardOffset.y() + 6, boardW, boardH, 12, 12);

    QLinearGradient ice(m_boardOffset.x(), m_boardOffset.y(), m_boardOffset.x(), m_boardOffset.y() + boardH);
    ice.setColorAt(0, QColor(60, 70, 85));
    ice.setColorAt(1, QColor(40, 50, 65));
    p.setBrush(ice);
    p.setPen(QPen(QColor(100, 115, 130), 2));
    p.drawRoundedRect(m_boardOffset.x(), m_boardOffset.y(), boardW, boardH, 10, 10);

    for (int r = 0; r < GRID; ++r) {
        for (int c = 0; c < GRID; ++c) {
            QRect rc = cellRect(r, c).adjusted(1, 1, -1, -1);
            int t = m_grid[r][c].type;

            if (t == 1) {
                QRadialGradient rock(rc.center(), rc.width() * 0.7);
                rock.setColorAt(0, QColor(80, 85, 95));
                rock.setColorAt(1, QColor(50, 55, 65));
                p.setBrush(rock);
                p.setPen(QPen(QColor(120, 125, 135), 1));
                p.drawRoundedRect(rc, 4, 4);
                p.setPen(QPen(QColor(150, 160, 175, 80), 1));
                p.drawLine(rc.left() + 4, rc.top() + 4, rc.right() - 4, rc.bottom() - 4);
            }
            else if (t == 0 || t == 2) {
                p.setBrush(QColor(55, 65, 80, 60));
                p.setPen(QPen(QColor(80, 95, 110, 80), 1));
                p.drawRect(rc);
            }

            if (t == 3) {
                p.setBrush(QColor(80, 50, 55, 120));
                p.setPen(QPen(QColor(160, 80, 90), 2, Qt::DashLine));
                p.drawEllipse(rc.center(), rc.width() / 3, rc.height() / 3);
                p.setPen(QColor(180, 100, 110, 150));
                p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 10));
                p.drawText(rc, Qt::AlignCenter, QString::fromUtf8(u8"梅"));
            }
            else if (t == 4) {
                QRadialGradient full(rc.center(), rc.width() * 0.45);
                full.setColorAt(0, QColor(200, 220, 240));
                full.setColorAt(1, QColor(140, 170, 200));
                p.setBrush(full);
                p.setPen(QPen(QColor(180, 210, 230), 2));
                p.drawEllipse(rc.center(), rc.width() / 3, rc.height() / 3);
                p.setPen(QColor(80, 50, 60));
                p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 10, QFont::Bold));
                p.drawText(rc, Qt::AlignCenter, QString::fromUtf8(u8"满"));
            }

            if (t == 2) {
                QRadialGradient snow(rc.center(), rc.width() * 0.35);
                snow.setColorAt(0, QColor(255, 255, 255));
                snow.setColorAt(0.6, QColor(230, 240, 250));
                snow.setColorAt(1, QColor(180, 200, 220));
                p.setBrush(snow);
                p.setPen(QPen(QColor(160, 180, 200), 2));
                p.drawEllipse(rc.center(), int(rc.width() * 0.32), int(rc.height() * 0.32));
                p.setBrush(QColor(255, 255, 255, 200));
                p.setPen(Qt::NoPen);
                p.drawEllipse(rc.center().x() - rc.width() / 6, rc.center().y() - rc.height() / 6,
                    rc.width() / 5, rc.height() / 5);
            }

            if (m_selectedCell.x() == r && m_selectedCell.y() == c && t == 2) {
                p.setPen(QPen(QColor(255, 220, 100), 3));
                p.setBrush(Qt::NoBrush);
                p.drawRoundedRect(rc.adjusted(-2, -2, 2, 2), 6, 6);
            }
        }
    }
}

void PorcelainRestoreGame::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    drawBackground(p);
    drawSnow(p);

    p.setPen(QColor(230, 225, 215));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 22, QFont::Bold));
    p.drawText(QRect(0, 25, width(), 50), Qt::AlignCenter,
        QString::fromUtf8(u8"雪团归位 · 将雪团滑入梅花坑"));

    p.setPen(QColor(180, 175, 165));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 13));
    p.drawText(QRect(0, 72, width(), 28), Qt::AlignCenter,
        QString::fromUtf8(u8"点击雪团选中，再点击同行或同列的空地使其滑行"));

    m_cellSize = qMin(70, qMin(width(), height()) / (GRID + 4));
    int boardW = GRID * m_cellSize;
    int boardH = GRID * m_cellSize;
    m_boardOffset = QPoint((width() - boardW) / 2, (height() - boardH) / 2 + 20);

    drawGrid(p);

    if (!m_running && m_winGlow > 0.01f) {
        int cx = m_boardOffset.x() + boardW / 2;
        int cy = m_boardOffset.y() + boardH / 2;
        QRadialGradient winGlow(cx, cy, boardW);
        winGlow.setColorAt(0, QColor(255, 230, 180, int(50 * m_winGlow)));
        winGlow.setColorAt(0.5, QColor(255, 210, 120, int(25 * m_winGlow)));
        winGlow.setColorAt(1, QColor(255, 180, 80, 0));
        p.setBrush(winGlow);
        p.setPen(Qt::NoPen);
        p.drawEllipse(cx, cy, boardW, boardH);

        p.setPen(QColor(255, 240, 200));
        p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 32, QFont::Bold));
        p.drawText(QRect(0, height() / 2 - 40, width(), 60), Qt::AlignCenter,
            QString::fromUtf8(u8"雪团归位"));
    }

    if (m_running) {
        QRect hintR(width() / 2 - 280, height() - 65, 560, 38);
        p.setBrush(QColor(0, 0, 0, 100));
        p.setPen(QPen(QColor(120, 115, 105, 80), 1));
        p.drawRoundedRect(hintR, 19, 19);
        p.setPen(QColor(200, 195, 185));
        p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 11));
        p.drawText(hintR, Qt::AlignCenter,
            QString::fromUtf8(u8"点击雪团选中    再点同行/列空地滑行    ESC 跳过"));
    }
}

void PorcelainRestoreGame::mousePressEvent(QMouseEvent* event)
{
    if (!m_running) return;
    QPoint cell = cellAt(event->pos());
    if (cell.x() < 0) return;
    int r = cell.x(), c = cell.y();

    if (m_grid[r][c].type == 2) {
        m_selectedCell = cell;
        update();
    }
    else if (m_grid[r][c].type == 0 || m_grid[r][c].type == 3) {
        trySlide(r, c);
        update();
    }
}

void PorcelainRestoreGame::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        m_running = false;
        releaseKeyboard();
        m_animTimer->stop();
        emit gameSkipped(m_itemName);
        hide();
    }
}

void PorcelainRestoreGame::showEvent(QShowEvent*)
{
    if (parentWidget()) {
        resize(parentWidget()->size());
        move(0, 0);
    }
}