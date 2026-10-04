#include "MortisePuzzleGame.h"
#include "ResourceManager.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

MortisePuzzleGame::MortisePuzzleGame(QWidget* parent) : QWidget(parent),
m_running(false), m_frame(0), m_winGlow(0.0f)
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
    m_rings.clear();
    m_center = QPoint(width() / 2, height() / 2 + 10);

    struct RingDef { int inner; int outer; };
    QList<RingDef> defs = { {40, 82}, {92, 148}, {158, 220} };

    for (int i = 0; i < 3; ++i) {
        Ring r;
        r.innerR = defs[i].inner;
        r.outerR = defs[i].outer;
        int steps = QRandomGenerator::global()->bounded(1, 6);
        r.angle = steps * 60;
        r.targetAngle = 0;
        r.locked = false;
        r.clickRipple = 0.0f;
        m_rings.append(r);
    }

    generateRingPixmaps();

    m_snowflakes.clear();
    m_snowSpeed.clear();
    for (int i = 0; i < 50; ++i) {
        m_snowflakes.append(QPointF(QRandomGenerator::global()->bounded(width()),
            QRandomGenerator::global()->bounded(height())));
        m_snowSpeed.append(0.3f + QRandomGenerator::global()->bounded(100) / 150.0f);
    }
}

void MortisePuzzleGame::generateRingPixmaps()
{
    for (int i = 0; i < m_rings.size(); ++i) {
        int size = m_rings[i].outerR * 2 + 24;
        QPixmap pix(size, size);
        pix.fill(Qt::transparent);
        QPainter pp(&pix);
        pp.setRenderHint(QPainter::Antialiasing);
        pp.translate(size / 2, size / 2);

        QPainterPath ringPath;
        ringPath.addEllipse(-m_rings[i].outerR, -m_rings[i].outerR,
            m_rings[i].outerR * 2, m_rings[i].outerR * 2);
        ringPath.addEllipse(-m_rings[i].innerR, -m_rings[i].innerR,
            m_rings[i].innerR * 2, m_rings[i].innerR * 2);
        ringPath.setFillRule(Qt::OddEvenFill);

        // 三层材质：内=紫檀，中=花梨，外=青铜
        QList<QColor> woodColors = {
            QColor(75, 45, 35),   // 簪首 - 紫檀
            QColor(130, 80, 50),  // 簪身 - 花梨
            QColor(160, 130, 80)  // 簪尾 - 黄杨
        };
        pp.setBrush(woodColors[i]);
        pp.setPen(QPen(woodColors[i].darker(130), 2));
        pp.drawPath(ringPath);

        // 绘制簪子纹样
        drawRingPattern(pp, i, m_rings[i].outerR);

        pp.end();
        m_rings[i].pixmap = pix;
    }
}

void MortisePuzzleGame::drawRingPattern(QPainter& p, int ringIdx, int r)
{
    p.save();
    p.setPen(Qt::NoPen);

    // 根据环绘制不同纹样
    if (ringIdx == 2) {
        // 外环 - 梅花五瓣（簪尾装饰）
        p.setBrush(QColor(200, 60, 60));
        for (int petal = 0; petal < 5; ++petal) {
            double ang = (72.0 * petal - 90.0) * M_PI / 180.0;
            int px = int((r - 22) * std::cos(ang));
            int py = int((r - 22) * std::sin(ang));
            p.drawEllipse(px - 7, py - 7, 14, 14);
        }
        p.setBrush(QColor(255, 220, 80));
        p.drawEllipse(-5, -5, 10, 10);

        // 外圈云雷纹装饰带
        p.setPen(QPen(QColor(180, 160, 100, 120), 2));
        p.setBrush(Qt::NoBrush);
        for (int seg = 0; seg < 12; ++seg) {
            double a1 = (seg * 30.0) * M_PI / 180.0;
            double a2 = ((seg + 1) * 30.0 - 5.0) * M_PI / 180.0;
            int x1 = int((r - 6) * std::cos(a1));
            int y1 = int((r - 6) * std::sin(a1));
            int x2 = int((r - 6) * std::cos(a2));
            int y2 = int((r - 6) * std::sin(a2));
            p.drawLine(x1, y1, x2, y2);
            int mx = int((r - 12) * std::cos((a1 + a2) / 2));
            int my = int((r - 12) * std::sin((a1 + a2) / 2));
            p.drawEllipse(mx - 2, my - 2, 4, 4);
        }
    }
    else if (ringIdx == 1) {
        // 中环 - 流云纹（簪身主体）
        p.setPen(QPen(QColor(220, 210, 180, 180), 3));
        p.setBrush(Qt::NoBrush);
        for (int cloud = 0; cloud < 6; ++cloud) {
            double baseAng = (cloud * 60.0) * M_PI / 180.0;
            int cx = int((r - 28) * std::cos(baseAng));
            int cy = int((r - 28) * std::sin(baseAng));
            p.drawArc(cx - 10, cy - 6, 20, 12, 0, 180 * 16);
            p.drawArc(cx - 6, cy - 4, 12, 10, 180 * 16, 180 * 16);
        }

        // 镶嵌金丝
        p.setPen(QPen(QColor(210, 190, 120, 150), 2));
        for (int seg = 0; seg < 6; ++seg) {
            double a = (seg * 60.0 + 30.0) * M_PI / 180.0;
            int x1 = int((r - 42) * std::cos(a));
            int y1 = int((r - 42) * std::sin(a));
            int x2 = int((r - 14) * std::cos(a));
            int y2 = int((r - 14) * std::sin(a));
            p.drawLine(x1, y1, x2, y2);
        }
    }
    else {
        // 内环 - 古篆"春"字变形（簪首）
        p.setPen(QPen(QColor(180, 50, 50, 200), 4));
        p.setBrush(Qt::NoBrush);
        int y = -r + 35;
        // 简化的春字篆体线条
        p.drawLine(0, y, 0, y + 28);
        p.drawLine(-10, y + 8, 10, y + 8);
        p.drawLine(-8, y + 18, 8, y + 18);
        p.drawArc(-6, y + 20, 12, 10, 200 * 16, 140 * 16);

        // 内圈玉饰
        p.setBrush(QColor(120, 180, 160, 100));
        p.setPen(Qt::NoPen);
        for (int j = 0; j < 4; ++j) {
            double a = (j * 90.0 + 45.0) * M_PI / 180.0;
            int jx = int((r - 18) * std::cos(a));
            int jy = int((r - 18) * std::sin(a));
            p.drawEllipse(jx - 4, jy - 4, 8, 8);
        }
    }

    p.restore();
}

void MortisePuzzleGame::drawBackground(QPainter& p)
{
    // 古书案/檀木背景
    QRadialGradient bg(QPointF(width() / 2, height() / 2), qMax(width(), height()));
    bg.setColorAt(0, QColor(42, 30, 22));
    bg.setColorAt(0.6, QColor(24, 16, 12));
    bg.setColorAt(1, QColor(12, 8, 6));
    p.fillRect(rect(), bg);

    if (!m_bgPixmap.isNull()) {
        p.setOpacity(0.05);
        p.drawPixmap(rect(), m_bgPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        p.setOpacity(1.0);
    }

    // 书案纹理横线
    p.setPen(QPen(QColor(60, 42, 30, 60), 1));
    for (int y = 0; y < height(); y += 40) {
        p.drawLine(0, y, width(), y);
    }

    // 四角青铜包角
    p.setPen(QPen(QColor(140, 120, 80, 100), 3));
    p.setBrush(Qt::NoBrush);
    int cs = 50;
    p.drawArc(15, 15, cs, cs, 90 * 16, 90 * 16);
    p.drawArc(width() - 15 - cs, 15, cs, cs, 0 * 16, 90 * 16);
    p.drawArc(15, height() - 15 - cs, cs, cs, 180 * 16, 90 * 16);
    p.drawArc(width() - 15 - cs, height() - 15 - cs, cs, cs, 270 * 16, 90 * 16);
}

void MortisePuzzleGame::drawSnow(QPainter& p)
{
    // 改为飘落的桃花/木屑微尘
    p.setPen(Qt::NoPen);
    for (int i = 0; i < m_snowflakes.size(); ++i) {
        int alpha = 45 + int(35 * std::sin(m_frame * 0.03 + i * 0.7));
        QColor dustColor = (i % 3 == 0) ? QColor(255, 200, 200, alpha)
            : QColor(220, 190, 150, alpha);
        p.setBrush(dustColor);
        int sz = 2 + (i % 3);
        p.drawEllipse(int(m_snowflakes[i].x()), int(m_snowflakes[i].y()), sz, sz);
    }
}

int MortisePuzzleGame::hitTestRing(const QPoint& pos) const
{
    QPoint d = pos - m_center;
    double dist = std::sqrt(d.x() * d.x() + d.y() * d.y());
    if (dist < 28) return -1;

    for (int i = 0; i < m_rings.size(); ++i) {
        if (dist >= m_rings[i].innerR && dist <= m_rings[i].outerR)
            return i;
    }
    return -1;
}

void MortisePuzzleGame::checkLocks()
{
    for (auto& r : m_rings) {
        bool wasLocked = r.locked;
        r.locked = (r.angle % 360 == 0);
        if (r.locked && !wasLocked) {
            r.clickRipple = 1.0f;
        }
    }
}

bool MortisePuzzleGame::checkWin() const
{
    for (const auto& r : m_rings) if (!r.locked) return false;
    return true;
}

void MortisePuzzleGame::updateAnimations()
{
    for (int i = 0; i < m_snowflakes.size(); ++i) {
        m_snowflakes[i].setY(m_snowflakes[i].y() + m_snowSpeed[i]);
        m_snowflakes[i].setX(m_snowflakes[i].x() + std::sin(m_frame * 0.012 + i) * 0.35);
        if (m_snowflakes[i].y() > height()) {
            m_snowflakes[i].setY(-5);
            m_snowflakes[i].setX(QRandomGenerator::global()->bounded(width()));
        }
    }

    for (auto& r : m_rings) {
        if (r.clickRipple > 0) {
            r.clickRipple -= 0.035f;
            if (r.clickRipple < 0) r.clickRipple = 0;
        }
    }

    if (!m_running && m_winGlow < 1.0f) {
        m_winGlow += 0.018f;
    }
}

void MortisePuzzleGame::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    drawBackground(p);
    drawSnow(p);

    // 标题
    p.setPen(QColor(230, 215, 185));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 22, QFont::Bold));
    p.drawText(QRect(0, 22, width(), 50), Qt::AlignCenter,
        QString::fromUtf8(u8"旧簪榫卯 · 旋合归位"));

    p.setPen(QColor(180, 165, 145));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 13));
    p.drawText(QRect(0, 68, width(), 28), Qt::AlignCenter,
        QString::fromUtf8(u8"点击圆环顺时针旋转，使榫卯纹路咬合归位"));

    // 绘制三层簪盘
    for (int i = 0; i < m_rings.size(); ++i) {
        p.save();
        p.translate(m_center);
        p.rotate(m_rings[i].angle);
        int px = -m_rings[i].pixmap.width() / 2;
        int py = -m_rings[i].pixmap.height() / 2;
        p.drawPixmap(px, py, m_rings[i].pixmap);
        p.restore();

        // 锁定金光
        if (m_rings[i].locked) {
            QRadialGradient glow(m_center, m_rings[i].outerR + 12);
            glow.setColorAt(0.7, QColor(255, 220, 120, 0));
            glow.setColorAt(0.85, QColor(255, 220, 120, 50));
            glow.setColorAt(1.0, QColor(255, 220, 120, 0));
            p.setBrush(glow);
            p.setPen(Qt::NoPen);
            p.drawEllipse(m_center, m_rings[i].outerR + 12, m_rings[i].outerR + 12);
        }

        // 点击涟漪
        if (m_rings[i].clickRipple > 0.01f) {
            int rippleR = int(m_rings[i].outerR * (0.85f + 0.35f * m_rings[i].clickRipple));
            p.setPen(QPen(QColor(255, 255, 255, int(90 * m_rings[i].clickRipple)), 2));
            p.setBrush(Qt::NoBrush);
            p.drawEllipse(m_center, rippleR, rippleR);
        }
    }

    // 中心枢轴（玉珠）
    QRadialGradient gem(m_center, 28);
    gem.setColorAt(0, QColor(255, 252, 235));
    gem.setColorAt(0.5, QColor(255, 230, 160));
    gem.setColorAt(1, QColor(200, 160, 80));
    p.setBrush(gem);
    p.setPen(QPen(QColor(160, 130, 60), 3));
    p.drawEllipse(m_center, 24, 24);

    // 玉珠高光
    p.setBrush(QColor(255, 255, 255, 180));
    p.setPen(Qt::NoPen);
    p.drawEllipse(m_center.x() - 8, m_center.y() - 8, 10, 8);

    // 胜利效果：榫卯归位，金光浮现，露出春时旧簪
    if (!m_running && m_winGlow > 0.01f) {
        int cx = m_center.x();
        int cy = m_center.y();

        // 整体辉光
        QRadialGradient winGlow(cx, cy, 320);
        winGlow.setColorAt(0, QColor(255, 240, 200, int(55 * m_winGlow)));
        winGlow.setColorAt(0.4, QColor(255, 220, 140, int(30 * m_winGlow)));
        winGlow.setColorAt(1, QColor(255, 200, 100, 0));
        p.setBrush(winGlow);
        p.setPen(Qt::NoPen);
        p.drawEllipse(cx, cy, 320, 320);

        // 簪子虚影从中心升起
        int hairpinH = int(80 * m_winGlow);
        if (hairpinH > 0) {
            p.setPen(QPen(QColor(255, 220, 140, int(200 * m_winGlow)), 3));
            p.setBrush(QColor(255, 240, 200, int(60 * m_winGlow)));
            // 簪杆
            p.drawRoundedRect(cx - 4, cy - hairpinH / 2, 8, hairpinH, 4, 4);
            // 簪首花饰
            p.setBrush(QColor(220, 80, 80, int(180 * m_winGlow)));
            p.drawEllipse(cx, cy - hairpinH / 2 - 8, 16, 16);
            // 流苏
            p.setPen(QPen(QColor(255, 220, 140, int(160 * m_winGlow)), 2));
            p.drawLine(cx, cy - hairpinH / 2 - 8, cx - 6, cy - hairpinH / 2 - 22);
            p.drawLine(cx, cy - hairpinH / 2 - 8, cx + 6, cy - hairpinH / 2 - 22);
        }

        p.setPen(QColor(255, 240, 200, int(255 * m_winGlow)));
        p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 28, QFont::Bold));
        p.drawText(QRect(0, height() / 2 - 55, width(), 45), Qt::AlignCenter,
            QString::fromUtf8(u8"榫卯归位"));

        p.setPen(QColor(255, 230, 170, int(255 * m_winGlow)));
        p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 15));
        p.drawText(QRect(0, height() / 2 + 55, width(), 30), Qt::AlignCenter,
            QString::fromUtf8(u8"获得【春时旧簪】"));
    }

    // 底部提示
    if (m_running) {
        int lockedCount = 0;
        for (const auto& r : m_rings) if (r.locked) ++lockedCount;

        QRect hintR(width() / 2 - 280, height() - 65, 560, 38);
        p.setBrush(QColor(0, 0, 0, 90));
        p.setPen(QPen(QColor(140, 120, 90, 80), 1));
        p.drawRoundedRect(hintR, 19, 19);
        p.setPen(QColor(200, 185, 160));
        p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 11));
        p.drawText(hintR, Qt::AlignCenter,
            QString::fromUtf8(u8"点击圆环旋转    已咬合 %1 / 3    ESC 跳过").arg(lockedCount));
    }
}

void MortisePuzzleGame::mousePressEvent(QMouseEvent* event)
{
    if (!m_running) return;
    int idx = hitTestRing(event->pos());
    if (idx < 0) return;

    auto& r = m_rings[idx];
    if (r.locked) return;

    r.angle = (r.angle + 60) % 360;
    r.clickRipple = 1.0f;

    checkLocks();
    update();

    if (checkWin()) {
        m_running = false;
        releaseKeyboard();
        m_animTimer->stop();
        QTimer::singleShot(1200, this, [this]() {
            emit gameFinished(m_itemName);
            hide();
            });
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