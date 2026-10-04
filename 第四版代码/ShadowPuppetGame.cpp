#include "ShadowPuppetGame.h"
#include "ResourceManager.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QTimer>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

ShadowPuppetGame::ShadowPuppetGame(QWidget* parent) : QWidget(parent),
m_running(false), m_round(0), m_sequenceLen(0), m_playerStep(0),
m_showTimer(0), m_inputEnabled(false), m_errorShake(0),
m_frame(0), m_winGlow(0.0f), m_cellSize(110), m_gap(24), m_lockOpen(0.0f)
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

    m_jadeNames.append(QString::fromUtf8(u8"龙纹"));
    m_jadeNames.append(QString::fromUtf8(u8"雷纹"));
    m_jadeNames.append(QString::fromUtf8(u8"凤纹"));
    m_jadeNames.append(QString::fromUtf8(u8"龟纹"));
    m_jadeNames.append(QString::fromUtf8(u8"云纹"));
    m_jadeNames.append(QString::fromUtf8(u8"星纹"));
    m_jadeNames.append(QString::fromUtf8(u8"水纹"));
    m_jadeNames.append(QString::fromUtf8(u8"火纹"));
    m_jadeNames.append(QString::fromUtf8(u8"山纹"));
}

void ShadowPuppetGame::startGame(const QString& itemName, const QString& bgName)
{
    m_itemName = itemName;
    m_running = true;
    m_round = 0;
    m_winGlow = 0.0f;
    m_lockOpen = 0.0f;

    if (!bgName.isEmpty())
        m_bgPixmap = ResourceManager::instance()->getBackground(bgName);

    resize(parentWidget() ? parentWidget()->size() : QSize(1280, 720));
    move(0, 0);
    initPuzzle();
    show(); raise(); grabKeyboard();
    m_animTimer->start(30);
}

void ShadowPuppetGame::initPuzzle()
{
    m_cellSize = 110;
    m_gap = 24;

    int boardW = 3 * m_cellSize + 2 * m_gap;
    int boardH = 3 * m_cellSize + 2 * m_gap;
    m_boardOffset = QPoint((width() - boardW) / 2, (height() - boardH) / 2 + 10);

    for (int i = 0; i < JADE_COUNT; ++i) {
        m_jadeGlow[i] = 0.0f;
        m_clickRipple[i] = 0.0f;
    }

    m_snowflakes.clear();
    m_snowSpeed.clear();
    for (int i = 0; i < 40; ++i) {
        m_snowflakes.append(QPointF(QRandomGenerator::global()->bounded(width()),
            QRandomGenerator::global()->bounded(height())));
        m_snowSpeed.append(0.3f + QRandomGenerator::global()->bounded(100) / 150.0f);
    }

    nextRound();
}

void ShadowPuppetGame::nextRound()
{
    if (m_round >= 5) {
        m_running = false;
        releaseKeyboard();
        m_animTimer->stop();
        QTimer::singleShot(800, this, [this]() {
            emit gameFinished(m_itemName);
            hide();
            });
        return;
    }

    m_round++;
    m_sequenceLen = 2 + m_round;
    m_sequence.clear();

    for (int i = 0; i < m_sequenceLen; ++i) {
        int prev = m_sequence.isEmpty() ? -1 : m_sequence.last();
        int next;
        do {
            next = QRandomGenerator::global()->bounded(JADE_COUNT);
        } while (next == prev);
        m_sequence.append(next);
    }

    m_showTimer = m_sequenceLen * 25;
    m_inputEnabled = false;
    m_playerStep = 0;
    for (int i = 0; i < JADE_COUNT; ++i) m_jadeGlow[i] = 0.0f;
}

void ShadowPuppetGame::playerClick(int idx)
{
    if (!m_inputEnabled || !m_running) return;

    if (idx != m_sequence[m_playerStep]) {
        m_errorShake = 24;
        m_inputEnabled = false;
        for (int i = 0; i < JADE_COUNT; ++i) m_jadeGlow[i] = 0.0f;
        m_jadeGlow[idx] = 1.0f;
        QTimer::singleShot(800, this, [this]() {
            if (!m_running) return;
            m_showTimer = m_sequenceLen * 25;
            });
        return;
    }

    m_clickRipple[idx] = 1.0f;
    m_playerStep++;

    if (m_playerStep >= m_sequence.size()) {
        m_inputEnabled = false;
        QTimer::singleShot(600, this, [this]() {
            if (!m_running) return;
            nextRound();
            });
    }
}

QPoint ShadowPuppetGame::jadeCenter(int idx) const
{
    int row = idx / 3;
    int col = idx % 3;
    int x = m_boardOffset.x() + col * (m_cellSize + m_gap) + m_cellSize / 2;
    int y = m_boardOffset.y() + row * (m_cellSize + m_gap) + m_cellSize / 2;
    return QPoint(x, y);
}

int ShadowPuppetGame::hitTestJade(const QPoint& pos) const
{
    for (int i = 0; i < JADE_COUNT; ++i) {
        QPoint c = jadeCenter(i);
        QPoint d = pos - c;
        if (d.x() * d.x() + d.y() * d.y() < (m_cellSize / 2) * (m_cellSize / 2))
            return i;
    }
    return -1;
}

void ShadowPuppetGame::updateAnimations()
{
    for (int i = 0; i < m_snowflakes.size(); ++i) {
        m_snowflakes[i].setY(m_snowflakes[i].y() + m_snowSpeed[i]);
        m_snowflakes[i].setX(m_snowflakes[i].x() + std::sin(m_frame * 0.015 + i) * 0.4);
        if (m_snowflakes[i].y() > height()) {
            m_snowflakes[i].setY(-5);
            m_snowflakes[i].setX(QRandomGenerator::global()->bounded(width()));
        }
    }

    if (m_showTimer > 0) {
        m_showTimer--;
        int framesPerHole = 25;
        int totalFrames = m_sequenceLen * framesPerHole;
        int step = (totalFrames - m_showTimer) / framesPerHole;

        for (int i = 0; i < JADE_COUNT; ++i) m_jadeGlow[i] = 0.0f;

        if (step < m_sequence.size()) {
            int jade = m_sequence[step];
            int progress = (totalFrames - m_showTimer) % framesPerHole;
            if (progress >= 5 && progress < 18) {
                m_jadeGlow[jade] = 1.0f;
            }
        }

        if (m_showTimer == 0) {
            m_inputEnabled = true;
            m_playerStep = 0;
        }
    }

    if (m_errorShake > 0) m_errorShake--;

    for (int i = 0; i < JADE_COUNT; ++i) {
        if (m_clickRipple[i] > 0.0f) {
            m_clickRipple[i] -= 0.05f;
            if (m_clickRipple[i] < 0.0f) m_clickRipple[i] = 0.0f;
        }
        if (m_jadeGlow[i] > 0.0f && m_showTimer == 0 && !m_inputEnabled && m_errorShake == 0) {
            m_jadeGlow[i] -= 0.05f;
            if (m_jadeGlow[i] < 0.0f) m_jadeGlow[i] = 0.0f;
        }
    }

    if (!m_running) {
        if (m_winGlow < 1.0f) m_winGlow += 0.02f;
        if (m_lockOpen < 1.0f) m_lockOpen += 0.015f;
    }
}

void ShadowPuppetGame::drawBackground(QPainter& p)
{
    QRadialGradient bg(QPointF(width() / 2, height() / 2), qMax(width(), height()));
    bg.setColorAt(0, QColor(35, 25, 18));
    bg.setColorAt(0.6, QColor(20, 14, 10));
    bg.setColorAt(1, QColor(10, 7, 5));
    p.fillRect(rect(), bg);

    if (!m_bgPixmap.isNull()) {
        p.setOpacity(0.06);
        p.drawPixmap(rect(), m_bgPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        p.setOpacity(1.0);
    }

    // 四角装饰
    p.setPen(QPen(QColor(160, 140, 80, 120), 3));
    p.setBrush(Qt::NoBrush);
    int cornerSize = 60;
    p.drawLine(20, 20, 20 + cornerSize, 20);
    p.drawLine(20, 20, 20, 20 + cornerSize);
    p.drawArc(20, 20, cornerSize, cornerSize, 90 * 16, 90 * 16);
    p.drawLine(width() - 20, 20, width() - 20 - cornerSize, 20);
    p.drawLine(width() - 20, 20, width() - 20, 20 + cornerSize);
    p.drawArc(width() - 20 - cornerSize, 20, cornerSize, cornerSize, 0 * 16, 90 * 16);
    p.drawLine(20, height() - 20, 20 + cornerSize, height() - 20);
    p.drawLine(20, height() - 20, 20, height() - 20 - cornerSize);
    p.drawArc(20, height() - 20 - cornerSize, cornerSize, cornerSize, 180 * 16, 90 * 16);
    p.drawLine(width() - 20, height() - 20, width() - 20 - cornerSize, height() - 20);
    p.drawLine(width() - 20, height() - 20, width() - 20, height() - 20 - cornerSize);
    p.drawArc(width() - 20 - cornerSize, height() - 20 - cornerSize, cornerSize, cornerSize, 270 * 16, 90 * 16);
}

void ShadowPuppetGame::drawJade(QPainter& p, int idx)
{
    QPoint center = jadeCenter(idx);
    int cx = center.x();
    int cy = center.y();
    int half = m_cellSize / 2;
    bool isError = (m_errorShake > 0 && m_jadeGlow[idx] > 0.5f && !m_inputEnabled);

    // 外圈阴影
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 80));
    p.drawEllipse(cx + 4, cy + 4, half - 2, half - 2);

    // 玉石底座凹槽
    QRadialGradient base(cx, cy, half);
    base.setColorAt(0, QColor(32, 24, 16));
    base.setColorAt(1, QColor(48, 38, 26));
    p.setBrush(base);
    p.setPen(QPen(QColor(65, 50, 35), 2));
    p.drawEllipse(cx, cy, half - 2, half - 2);

    // 发光效果
    if (m_jadeGlow[idx] > 0.01f || m_clickRipple[idx] > 0.01f) {
        float glowIntensity = qMax(m_jadeGlow[idx], m_clickRipple[idx] * 0.6f);
        int breath = int(6 + 4 * std::sin(m_frame * 0.12f + idx));
        QColor glowColor = isError ? QColor(220, 30, 30, int(140 * glowIntensity))
            : QColor(255, 230, 150, int(110 * glowIntensity));
        QRadialGradient glow(cx, cy, half + 12 + breath);
        glow.setColorAt(0, glowColor);
        glow.setColorAt(1, QColor(255, 220, 100, 0));
        p.setBrush(glow);
        p.setPen(Qt::NoPen);
        p.drawEllipse(cx, cy, half + 12 + breath, half + 12 + breath);
    }

    // 贴图
    QPixmap itemPix = ResourceManager::instance()->getItem(m_jadeNames[idx]);
    if (!itemPix.isNull()) {
        int imgSize = half * 2.0f;
        QPixmap scaled = itemPix.scaled(imgSize, imgSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        QRect imgRect(cx - scaled.width() / 2, cy - scaled.height() / 2, scaled.width(), scaled.height());
        p.drawPixmap(imgRect, scaled);
    }
    else {
        // 无贴图时显示元素名
        p.setPen(QColor(180, 160, 120));
        p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 10));
        p.drawText(QRect(cx - half + 4, cy - 8, half * 2 - 8, 16), Qt::AlignCenter, m_jadeNames[idx]);
    }

    // 点击涟漪
    if (m_clickRipple[idx] > 0.01f) {
        int rippleR = int(half * (0.9f + 0.4f * m_clickRipple[idx]));
        p.setPen(QPen(QColor(255, 255, 255, int(160 * m_clickRipple[idx])), 2));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(cx, cy, rippleR, rippleR);
    }
}

void ShadowPuppetGame::drawProgress(QPainter& p)
{
    QRect pr(width() / 2 - 150, 110, 300, 36);
    p.setBrush(QColor(0, 0, 0, 80));
    p.setPen(QPen(QColor(140, 120, 90, 100), 1));
    p.drawRoundedRect(pr, 18, 18);

    p.setPen(QColor(220, 200, 170));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 13));
    QString text = QString::fromUtf8(u8"当前轮次：%1 / 5    序列长度：%2").arg(m_round).arg(m_sequenceLen);
    p.drawText(pr, Qt::AlignCenter, text);

    int dotY = 152;
    int dotStartX = width() / 2 - 5 * 14 / 2;
    for (int i = 1; i <= 5; ++i) {
        QRect dr(dotStartX + (i - 1) * 18, dotY, 10, 10);
        if (i < m_round) {
            p.setBrush(QColor(180, 220, 120));
            p.setPen(QPen(QColor(140, 180, 90), 1));
        }
        else if (i == m_round) {
            p.setBrush(QColor(255, 220, 120));
            p.setPen(QPen(QColor(200, 170, 80), 2));
        }
        else {
            p.setBrush(QColor(60, 50, 40));
            p.setPen(QPen(QColor(45, 38, 30), 1));
        }
        p.drawEllipse(dr);
    }
}

void ShadowPuppetGame::drawSnow(QPainter& p)
{
    p.setPen(Qt::NoPen);
    for (int i = 0; i < m_snowflakes.size(); ++i) {
        int alpha = 40 + int(30 * std::sin(m_frame * 0.04 + i * 0.5));
        p.setBrush(QColor(200, 180, 140, alpha));
        int sz = 2 + (i % 3);
        p.drawEllipse(int(m_snowflakes[i].x()), int(m_snowflakes[i].y()), sz, sz);
    }
}

void ShadowPuppetGame::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    m_frame++;

    if (m_errorShake > 0) {
        int dx = (m_errorShake % 4 < 2) ? 5 : -5;
        p.translate(dx, 0);
        m_errorShake--;
    }

    drawBackground(p);
    drawSnow(p);

    // 标题
    p.setPen(QColor(230, 210, 180));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 24, QFont::Bold));
    p.drawText(QRect(0, 25, width(), 55), Qt::AlignCenter,
        QString::fromUtf8(u8"梳影归位 · 嵌玉图阵"));

    // 进度
    drawProgress(p);

    // 计算布局
    m_cellSize = qMin(180, qMin(width(), height()) / 4);
    m_gap = qMin(24, m_cellSize / 4);
    int boardW = 3 * m_cellSize + 2 * m_gap;
    int boardH = 3 * m_cellSize + 2 * m_gap;
    m_boardOffset = QPoint((width() - boardW) / 2, (height() - boardH) / 2 + 10);

    // 绘制 3x3 九宫格玉石
    for (int i = 0; i < JADE_COUNT; ++i) {
        drawJade(p, i);
    }

    // 胜利效果
    if (!m_running && m_lockOpen > 0.01f) {
        int cx = m_boardOffset.x() + boardW / 2;
        int cy = m_boardOffset.y() + boardH / 2;

        int openR = int(60 * m_lockOpen);
        QRadialGradient openGlow(cx, cy, openR + 60);
        openGlow.setColorAt(0, QColor(255, 240, 200, int(180 * m_winGlow)));
        openGlow.setColorAt(0.5, QColor(255, 220, 140, int(90 * m_winGlow)));
        openGlow.setColorAt(1, QColor(255, 200, 100, 0));
        p.setBrush(openGlow);
        p.setPen(Qt::NoPen);
        p.drawEllipse(cx, cy, openR + 60, openR + 60);

        if (m_winGlow > 0.3f) {
            p.setPen(QColor(255, 240, 200, int(255 * m_winGlow)));
            p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 26, QFont::Bold));
            p.drawText(QRect(0, height() / 2 - 40, width(), 40), Qt::AlignCenter,
                QString::fromUtf8(u8"梳影归位"));

            p.setPen(QColor(255, 220, 150, int(255 * m_winGlow)));
            p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 15));
            p.drawText(QRect(0, height() / 2 + 15, width(), 30), Qt::AlignCenter,
                QString::fromUtf8(u8"获得【嵌玉发梳】"));
        }
    }

    // 状态提示
    if (m_running) {
        QString status;
        if (m_showTimer > 0) {
            status = QString::fromUtf8(u8"请仔细观察玉石亮起顺序……");
        }
        else if (m_inputEnabled) {
            status = QString::fromUtf8(u8"轮到你了，按顺序点击图案");
        }
        else if (m_errorShake > 0) {
            status = QString::fromUtf8(u8"顺序错误，请重新观察");
        }
        else {
            status = QString::fromUtf8(u8"请稍候……");
        }

        QRect hintR(width() / 2 - 280, height() - 70, 560, 38);
        p.setBrush(QColor(0, 0, 0, 100));
        p.setPen(QPen(QColor(120, 100, 70, 80), 1));
        p.drawRoundedRect(hintR, 19, 19);
        p.setPen(QColor(200, 185, 160));
        p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 11));
        p.drawText(hintR, Qt::AlignCenter, status);
    }
}

void ShadowPuppetGame::mousePressEvent(QMouseEvent* event)
{
    if (!m_running || !m_inputEnabled) return;
    int idx = hitTestJade(event->pos());
    if (idx >= 0) {
        playerClick(idx);
        update();
    }
}

void ShadowPuppetGame::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        m_running = false;
        releaseKeyboard();
        m_animTimer->stop();
        emit gameSkipped(m_itemName);
        hide();
    }
}

void ShadowPuppetGame::showEvent(QShowEvent*)
{
    if (parentWidget()) {
        resize(parentWidget()->size());
        move(0, 0);
    }
    initPuzzle();
}