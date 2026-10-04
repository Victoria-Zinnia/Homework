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
m_frame(0), m_winGlow(0.0f), m_cellSize(90), m_lockOpen(0.0f)
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
    m_cellSize = qMin(90, qMin(width(), height()) / 6);
    m_boardCenter = QPoint(width() / 2, height() / 2 + 20);

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

int ShadowPuppetGame::hitTestJade(const QPoint& pos) const
{
    int gap = 12;
    int boardW = 3 * m_cellSize + 2 * gap;
    int startX = m_boardCenter.x() - boardW / 2;
    int startY = m_boardCenter.y() - boardW / 2;

    for (int i = 0; i < JADE_COUNT; ++i) {
        int col = i % 3;
        int row = i / 3;
        int cx = startX + col * (m_cellSize + gap) + m_cellSize / 2;
        int cy = startY + row * (m_cellSize + gap) + m_cellSize / 2;
        QPoint d = pos - QPoint(cx, cy);
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

    p.setPen(QPen(QColor(160, 140, 80, 120), 3));
    p.setBrush(Qt::NoBrush);
    // 四角金属装饰
    int cornerSize = 60;
    // 左上
    p.drawLine(20, 20, 20 + cornerSize, 20);
    p.drawLine(20, 20, 20, 20 + cornerSize);
    p.drawArc(20, 20, cornerSize, cornerSize, 90 * 16, 90 * 16);
    // 右上
    p.drawLine(width() - 20, 20, width() - 20 - cornerSize, 20);
    p.drawLine(width() - 20, 20, width() - 20, 20 + cornerSize);
    p.drawArc(width() - 20 - cornerSize, 20, cornerSize, cornerSize, 0 * 16, 90 * 16);
    // 左下
    p.drawLine(20, height() - 20, 20 + cornerSize, height() - 20);
    p.drawLine(20, height() - 20, 20, height() - 20 - cornerSize);
    p.drawArc(20, height() - 20 - cornerSize, cornerSize, cornerSize, 180 * 16, 90 * 16);
    // 右下
    p.drawLine(width() - 20, height() - 20, width() - 20 - cornerSize, height() - 20);
    p.drawLine(width() - 20, height() - 20, width() - 20, height() - 20 - cornerSize);
    p.drawArc(width() - 20 - cornerSize, height() - 20 - cornerSize, cornerSize, cornerSize, 270 * 16, 90 * 16);
}

void ShadowPuppetGame::drawLockPlate(QPainter& p)
{
    int gap = 12;
    int boardW = 3 * m_cellSize + 2 * gap;
    int cx = m_boardCenter.x();
    int cy = m_boardCenter.y();
    int outerR = boardW / 2 + 30;

    // 外圈阴影
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 100));
    p.drawEllipse(cx, cy, outerR + 6, outerR + 6);

    // 青铜外圈
    QRadialGradient bronze(cx, cy, outerR);
    bronze.setColorAt(0, QColor(95, 78, 58));
    bronze.setColorAt(0.7, QColor(68, 54, 38));
    bronze.setColorAt(1, QColor(48, 38, 25));
    p.setBrush(bronze);
    p.setPen(QPen(QColor(125, 105, 72), 5));
    p.drawEllipse(cx, cy, outerR, outerR);

    // 内圈凹槽
    int innerR = outerR - 14;
    QRadialGradient groove(cx, cy, innerR);
    groove.setColorAt(0, QColor(38, 28, 18));
    groove.setColorAt(1, QColor(58, 44, 30));
    p.setBrush(groove);
    p.setPen(QPen(QColor(75, 58, 40), 3));
    p.drawEllipse(cx, cy, innerR, innerR);

    // 青铜装饰纹样（放射短线）
    p.setPen(QPen(QColor(115, 95, 65, 100), 2));
    p.setBrush(Qt::NoBrush);
    for (int angle = 0; angle < 360; angle += 20) {
        double rad = angle * M_PI / 180.0;
        int x1 = cx + int((innerR - 6) * std::cos(rad));
        int y1 = cy + int((innerR - 6) * std::sin(rad));
        int x2 = cx + int((outerR - 4) * std::cos(rad));
        int y2 = cy + int((outerR - 4) * std::sin(rad));
        p.drawLine(x1, y1, x2, y2);
    }

    // 中心枢轴（平时盖住，胜利时打开）
    if (m_lockOpen < 0.3f) {
        int pivotR = int(18 * (1.0f - m_lockOpen / 0.3f));
        if (pivotR > 0) {
            QRadialGradient pivot(cx, cy, pivotR);
            pivot.setColorAt(0, QColor(80, 65, 45));
            pivot.setColorAt(1, QColor(55, 42, 28));
            p.setBrush(pivot);
            p.setPen(QPen(QColor(100, 82, 58), 2));
            p.drawEllipse(cx, cy, pivotR, pivotR);
        }
    }
}

void ShadowPuppetGame::drawJade(QPainter& p, int idx, int cx, int cy, int size)
{
    bool isError = (m_errorShake > 0 && m_jadeGlow[idx] > 0.5f && !m_inputEnabled);
    int half = size / 2;

    // 玉石基座凹槽
    QRadialGradient base(cx, cy, half);
    base.setColorAt(0, QColor(32, 24, 16));
    base.setColorAt(1, QColor(48, 38, 26));
    p.setBrush(base);
    p.setPen(QPen(QColor(65, 50, 35), 2));
    p.drawEllipse(cx, cy, half - 2, half - 2);

    // 发光效果（系统提示或错误）
    if (m_jadeGlow[idx] > 0.01f || m_clickRipple[idx] > 0.01f) {
        float glowIntensity = qMax(m_jadeGlow[idx], m_clickRipple[idx] * 0.6f);
        int breath = int(8 + 5 * std::sin(m_frame * 0.12f + idx));
        QColor glowColor = isError ? QColor(220, 30, 30, int(140 * glowIntensity))
            : QColor(255, 230, 150, int(110 * glowIntensity));
        QRadialGradient glow(cx, cy, half + 14 + breath);
        glow.setColorAt(0, glowColor);
        glow.setColorAt(1, QColor(255, 220, 100, 0));
        p.setBrush(glow);
        p.setPen(Qt::NoPen);
        p.drawEllipse(cx, cy, half + 14 + breath, half + 14 + breath);
    }

    // 玉石颜色表
    const QColor jadeColors[9] = {
        QColor(185, 55, 55),   // 龙
        QColor(205, 165, 65),  // 虎
        QColor(225, 85, 150),  // 凤
        QColor(55, 140, 100),  // 龟
        QColor(190, 190, 210), // 云
        QColor(160, 95, 200),  // 雷
        QColor(55, 120, 180),  // 水
        QColor(215, 95, 45),   // 火
        QColor(140, 110, 75)   // 山
    };

    QColor jadeColor = jadeColors[idx];
    if (isError) jadeColor = QColor(190, 45, 45);

    // 玉石本体
    QRadialGradient jade(cx - 2, cy - 2, half * 0.7f);
    jade.setColorAt(0, jadeColor.lighter(145));
    jade.setColorAt(0.6, jadeColor);
    jade.setColorAt(1, jadeColor.darker(125));
    p.setBrush(jade);
    p.setPen(QPen(isError ? QColor(150, 35, 35) : QColor(175, 155, 115), 2));
    p.drawEllipse(cx, cy, half - 6, half - 6);

    // 高光
    p.setBrush(QColor(255, 255, 255, 170));
    p.setPen(Qt::NoPen);
    p.drawEllipse(cx - half / 5, cy - half / 4, half / 4, half / 5);

    // 图腾图案
    drawPattern(p, idx, cx, cy, half - 10);

    // 点击涟漪
    if (m_clickRipple[idx] > 0.01f) {
        int rippleR = int(half * (0.9f + 0.5f * m_clickRipple[idx]));
        p.setPen(QPen(QColor(255, 255, 255, int(160 * m_clickRipple[idx])), 2));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(cx, cy, rippleR, rippleR);
    }
}

void ShadowPuppetGame::drawPattern(QPainter& p, int idx, int cx, int cy, int size)
{
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    int r = size / 2;
    QColor lightColor(255, 255, 255, 200);
    QColor darkColor(40, 25, 15, 180);
    p.setPen(QPen(lightColor, 2));
    p.setBrush(Qt::NoBrush);

    switch (idx) {
    case 0: // 龙 - 蜿蜒S形
    {
        QPainterPath path;
        path.moveTo(cx - r * 0.5f, cy + r * 0.3f);
        path.cubicTo(cx - r * 0.2f, cy - r * 0.6f, cx + r * 0.2f, cy + r * 0.6f, cx + r * 0.5f, cy - r * 0.3f);
        p.drawPath(path);
        p.drawEllipse(int(cx + r * 0.5f - 3), int(cy - r * 0.3f - 3), 6, 6); // 龙睛
    }
    break;
    case 1: // 虎 - 王字纹
        p.drawLine(cx - r * 0.4f, cy - r * 0.3f, cx + r * 0.4f, cy - r * 0.3f);
        p.drawLine(cx - r * 0.3f, cy, cx + r * 0.3f, cy);
        p.drawLine(cx - r * 0.4f, cy + r * 0.3f, cx + r * 0.4f, cy + r * 0.3f);
        p.drawLine(cx, cy - r * 0.3f, cx, cy + r * 0.3f);
        break;
    case 2: // 凤 - 羽冠与尾
    {
        QPainterPath path;
        path.moveTo(cx, cy - r * 0.5f);
        path.quadTo(cx - r * 0.4f, cy, cx - r * 0.2f, cy + r * 0.4f);
        path.moveTo(cx, cy - r * 0.5f);
        path.quadTo(cx + r * 0.4f, cy, cx + r * 0.2f, cy + r * 0.4f);
        p.drawPath(path);
        p.drawLine(cx, cy - r * 0.5f, cx, cy + r * 0.2f);
    }
    break;
    case 3: // 龟 - 六角甲纹
    {
        QPolygon hex;
        for (int i = 0; i < 6; ++i) {
            double angle = M_PI / 3.0 * i - M_PI / 6.0;
            hex << QPoint(int(cx + r * 0.35f * std::cos(angle)), int(cy + r * 0.35f * std::sin(angle)));
        }
        p.drawPolygon(hex);
        p.drawEllipse(cx, cy, r * 0.2f, r * 0.2f);
    }
    break;
    case 4: // 云 - 卷云纹
    {
        p.drawArc(int(cx - r * 0.35f), int(cy - r * 0.2f), int(r * 0.5f), int(r * 0.4f), 0, 180 * 16);
        p.drawArc(int(cx - r * 0.1f), int(cy - r * 0.25f), int(r * 0.5f), int(r * 0.4f), 180 * 16, 180 * 16);
    }
    break;
    case 5: // 雷 - 闪电折线
    {
        QPainterPath path;
        path.moveTo(cx - r * 0.2f, cy - r * 0.5f);
        path.lineTo(cx + r * 0.1f, cy - r * 0.1f);
        path.lineTo(cx - r * 0.1f, cy + r * 0.1f);
        path.lineTo(cx + r * 0.2f, cy + r * 0.5f);
        p.drawPath(path);
    }
    break;
    case 6: // 水 - 波浪
    {
        QPainterPath path;
        path.moveTo(cx - r * 0.5f, cy);
        path.quadTo(cx - r * 0.25f, cy - r * 0.35f, cx, cy);
        path.quadTo(cx + r * 0.25f, cy + r * 0.35f, cx + r * 0.5f, cy);
        p.drawPath(path);
    }
    break;
    case 7: // 火 - 焰苗
    {
        QPainterPath path;
        path.moveTo(cx, cy + r * 0.4f);
        path.quadTo(cx - r * 0.25f, cy, cx - r * 0.1f, cy - r * 0.3f);
        path.quadTo(cx, cy - r * 0.5f, cx + r * 0.1f, cy - r * 0.3f);
        path.quadTo(cx + r * 0.25f, cy, cx, cy + r * 0.4f);
        p.drawPath(path);
        p.drawLine(cx, cy + r * 0.4f, cx, cy - r * 0.1f);
    }
    break;
    case 8: // 山 - 三峰
    {
        QPainterPath path;
        path.moveTo(cx - r * 0.5f, cy + r * 0.4f);
        path.lineTo(cx - r * 0.2f, cy - r * 0.2f);
        path.lineTo(cx, cy + r * 0.1f);
        path.lineTo(cx + r * 0.2f, cy - r * 0.35f);
        path.lineTo(cx + r * 0.5f, cy + r * 0.4f);
        p.drawPath(path);
    }
    break;
    }
    p.restore();
}

void ShadowPuppetGame::drawProgress(QPainter& p)
{
    // 锁盘上方进度指示器
    QRect pr(width() / 2 - 150, 110, 300, 36);
    p.setBrush(QColor(0, 0, 0, 80));
    p.setPen(QPen(QColor(140, 120, 90, 100), 1));
    p.drawRoundedRect(pr, 18, 18);

    p.setPen(QColor(220, 200, 170));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 13));
    QString text = QString::fromUtf8(u8"璇玑玉锁 · 第 %1 / 5 轮 · 序列 %2").arg(m_round).arg(m_sequenceLen);
    p.drawText(pr, Qt::AlignCenter, text);

    // 轮次圆点
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
        QString::fromUtf8(u8"璇玑玉锁 · 复现图腾之序"));

    // 进度
    drawProgress(p);

    // 更新棋盘尺寸与中心
    m_cellSize = qMin(90, qMin(width(), height()) / 6);
    m_boardCenter = QPoint(width() / 2, height() / 2 + 25);

    // 绘制锁盘与玉石
    drawLockPlate(p);
    int gap = 12;
    int boardW = 3 * m_cellSize + 2 * gap;
    int startX = m_boardCenter.x() - boardW / 2;
    int startY = m_boardCenter.y() - boardW / 2;

    for (int i = 0; i < JADE_COUNT; ++i) {
        int col = i % 3;
        int row = i / 3;
        int cx = startX + col * (m_cellSize + gap) + m_cellSize / 2;
        int cy = startY + row * (m_cellSize + gap) + m_cellSize / 2;
        drawJade(p, i, cx, cy, m_cellSize);
    }

    // 胜利效果：锁盘中央打开，露出"嵌玉发梳"
    if (!m_running && m_lockOpen > 0.01f) {
        int cx = m_boardCenter.x();
        int cy = m_boardCenter.y();
        int openR = int(30 * m_lockOpen);

        // 中央裂口光芒
        QRadialGradient openGlow(cx, cy, openR + 40);
        openGlow.setColorAt(0, QColor(255, 240, 200, int(180 * m_winGlow)));
        openGlow.setColorAt(0.5, QColor(255, 220, 140, int(90 * m_winGlow)));
        openGlow.setColorAt(1, QColor(255, 200, 100, 0));
        p.setBrush(openGlow);
        p.setPen(Qt::NoPen);
        p.drawEllipse(cx, cy, openR + 40, openR + 40);

        // 发梳轮廓（简化为金色梳形）
        if (m_lockOpen > 0.4f) {
            p.setBrush(QColor(255, 220, 120, int(255 * (m_lockOpen - 0.4f) / 0.6f)));
            p.setPen(QPen(QColor(200, 170, 80, int(200 * (m_lockOpen - 0.4f) / 0.6f)), 2));
            int combW = int(24 * m_lockOpen);
            int combH = int(36 * m_lockOpen);
            QRect combRect(cx - combW / 2, cy - combH / 2, combW, combH);
            p.drawRoundedRect(combRect, 4, 4);
            // 梳齿
            for (int k = 0; k < 4; ++k) {
                int sx = combRect.left() + 4 + k * (combW - 8) / 3;
                p.drawLine(sx, combRect.top() + 8, sx, combRect.bottom() - 4);
            }
        }

        // 胜利文字
        if (m_winGlow > 0.3f) {
            p.setPen(QColor(255, 240, 200, int(255 * m_winGlow)));
            p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 26, QFont::Bold));
            p.drawText(QRect(0, height() / 2 - 80, width(), 40), Qt::AlignCenter,
                QString::fromUtf8(u8"锁开玉现"));

            p.setPen(QColor(255, 220, 150, int(255 * m_winGlow)));
            p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 15));
            p.drawText(QRect(0, height() / 2 + 55, width(), 30), Qt::AlignCenter,
                QString::fromUtf8(u8"获得【嵌玉发梳】"));
        }
    }

    // 状态提示
    if (m_running) {
        QString status;
        if (m_showTimer > 0) {
            status = QString::fromUtf8(u8"仔细观察玉石亮起顺序……");
        }
        else if (m_inputEnabled) {
            status = QString::fromUtf8(u8"轮到你了：按顺序点击图腾");
        }
        else if (m_errorShake > 0) {
            status = QString::fromUtf8(u8"顺序有误，玉锁震颤……");
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