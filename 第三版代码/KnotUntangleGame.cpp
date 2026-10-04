#include "KnotUntangleGame.h"
#include "ResourceManager.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <algorithm>
#include <cmath>

KnotUntangleGame::KnotUntangleGame(QWidget* parent) : QWidget(parent),
m_running(false), m_dragIdx(-1), m_frame(0), m_errorShake(0), m_errorFlash(0.0f)
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

void KnotUntangleGame::startGame(const QString& itemName, const QString& bgName)
{
    m_itemName = itemName;
    m_running = true;
    m_dragIdx = -1;
    m_frame = 0;
    m_errorShake = 0;
    m_errorFlash = 0.0f;

    if (!bgName.isEmpty())
        m_bgPixmap = ResourceManager::instance()->getBackground(bgName);

    resize(parentWidget() ? parentWidget()->size() : QSize(1280, 720));
    move(0, 0);
    initPuzzle();

    show(); raise(); grabKeyboard();
    m_animTimer->start(40);
}

void KnotUntangleGame::initPuzzle()
{
    m_ropes.clear();
    m_slots.clear();

    int w = width();
    int h = height();

    // 6 种高区分度颜色
    QList<QColor> colors = {
        QColor(200, 50, 50),   // 朱红
        QColor(230, 180, 40),  // 明黄
        QColor(40, 160, 80),   // 翠绿
        QColor(50, 90, 190),   // 宝蓝
        QColor(160, 60, 170),  // 紫罗兰
        QColor(230, 100, 30)   // 橙红
    };
    QList<QColor> jadeColors = {
        QColor(255, 180, 180),
        QColor(255, 240, 180),
        QColor(180, 255, 200),
        QColor(180, 210, 255),
        QColor(230, 180, 255),
        QColor(255, 200, 160)
    };
    QStringList names = {
        QString::fromUtf8(u8"朱缨"),
        QString::fromUtf8(u8"金缕"),
        QString::fromUtf8(u8"碧绦"),
        QString::fromUtf8(u8"蓝络"),
        QString::fromUtf8(u8"紫纡"),
        QString::fromUtf8(u8"赤绳")
    };

    int slotW = 80, slotH = 80, gap = 30;
    int totalW = 6 * slotW + 5 * gap;
    int startX = (w - totalW) / 2;
    int slotY = h - 160;

    for (int i = 0; i < 6; ++i) {
        Slot s;
        s.rect = QRect(startX + i * (slotW + gap), slotY, slotW, slotH);
        s.color = colors[i];
        s.jadeColor = jadeColors[i];
        s.label = names[i];
        s.filled = false;
        m_slots.append(s);
    }

    int topY = 140;
    int midY1 = h / 2 - 70;
    int midY2 = h / 2 + 30;

    for (int i = 0; i < 6; ++i) {
        Rope r;
        r.color = colors[i];
        r.jadeColor = jadeColors[i];
        r.name = names[i];
        r.targetSlot = i;
        r.extracted = false;
        r.layer = i + 1;
        r.hoverGlow = 0.0f;

        int sx = w / 2 - 300 + QRandomGenerator::global()->bounded(600);
        r.points << QPoint(sx, topY);

        int mx1 = w / 2 - 350 + QRandomGenerator::global()->bounded(700);
        int my1 = topY + (midY1 - topY) / 2 + QRandomGenerator::global()->bounded(-60, 60);
        r.points << QPoint(mx1, my1);

        int mx2 = w / 2 - 350 + QRandomGenerator::global()->bounded(700);
        int my2 = midY1 + QRandomGenerator::global()->bounded(-50, 50);
        r.points << QPoint(mx2, my2);

        int mx3 = w / 2 - 300 + QRandomGenerator::global()->bounded(600);
        int my3 = midY2 + QRandomGenerator::global()->bounded(-40, 40);
        r.points << QPoint(mx3, my3);

        int wrongOffset = ((i + 3) % 6);
        if (wrongOffset == 0) wrongOffset = 1;
        int restX = m_slots[(i + wrongOffset) % 6].rect.center().x()
            + QRandomGenerator::global()->bounded(-60, 60);
        int restY = slotY - 120 - QRandomGenerator::global()->bounded(80);
        r.restPos = QPoint(restX, restY);
        r.headPos = r.restPos;

        m_ropes.append(r);
    }

    QList<int> layers;
    for (int i = 1; i <= 6; ++i) layers.append(i);
    for (int i = 5; i > 0; --i) {
        int j = QRandomGenerator::global()->bounded(i + 1);
        qSwap(layers[i], layers[j]);
    }
    for (int i = 0; i < 6; ++i) {
        m_ropes[i].layer = layers[i];
    }
}

bool KnotUntangleGame::isTopmost(int index) const
{
    if (index < 0 || index >= m_ropes.size()) return false;
    if (m_ropes[index].extracted) return false;
    int curLayer = m_ropes[index].layer;
    for (int i = 0; i < m_ropes.size(); ++i) {
        if (i != index && !m_ropes[i].extracted && m_ropes[i].layer > curLayer)
            return false;
    }
    return true;
}

int KnotUntangleGame::hitTestHead(const QPoint& pos) const
{
    QList<int> order;
    for (int i = 0; i < m_ropes.size(); ++i)
        if (!m_ropes[i].extracted) order.append(i);
    std::sort(order.begin(), order.end(), [this](int a, int b) {
        return m_ropes[a].layer > m_ropes[b].layer;
        });
    for (int idx : order) {
        QPoint d = pos - m_ropes[idx].headPos;
        if (d.x() * d.x() + d.y() * d.y() < 28 * 28) return idx;
    }
    return -1;
}

int KnotUntangleGame::hitTestSlot(const QPoint& pos) const
{
    for (int i = 0; i < m_slots.size(); ++i) {
        if (m_slots[i].rect.contains(pos)) return i;
    }
    return -1;
}

void KnotUntangleGame::updateAnimations()
{
    for (int i = 0; i < m_ropes.size(); ++i) {
        auto& r = m_ropes[i];
        if (r.extracted) {
            QPoint target = m_slots[r.targetSlot].rect.center();
            r.headPos = QPoint(
                r.headPos.x() + (target.x() - r.headPos.x()) * 0.12,
                r.headPos.y() + (target.y() - r.headPos.y()) * 0.12
            );
        }
        else if (m_dragIdx < 0) {
            int dx = r.restPos.x() - r.headPos.x();
            int dy = r.restPos.y() - r.headPos.y();
            if (qAbs(dx) > 2 || qAbs(dy) > 2) {
                r.headPos = QPoint(r.headPos.x() + dx * 0.12, r.headPos.y() + dy * 0.12);
            }
            else {
                r.headPos = r.restPos;
            }
        }
        if (isTopmost(i)) {
            r.hoverGlow = 0.6f + 0.4f * std::sin(m_frame * 0.08f + r.headPos.x() * 0.05f);
        }
        else {
            r.hoverGlow = qMax(0.0f, r.hoverGlow - 0.05f);
        }
    }
    if (m_errorShake > 0) m_errorShake--;
    if (m_errorFlash > 0.0f) m_errorFlash -= 0.03f;
}

void KnotUntangleGame::checkWin()
{
    bool all = true;
    for (const auto& r : m_ropes) {
        if (!r.extracted) { all = false; break; }
    }
    if (all) {
        m_running = false;
        releaseKeyboard();
        m_animTimer->stop();
        emit gameFinished(m_itemName);
        hide();
    }
}

void KnotUntangleGame::drawJadeHead(QPainter& p, const QPoint& head,
    const QColor& jadeColor, const QColor& ropeColor, bool glow, bool dragging)
{
    p.save();
    if (glow || dragging) {
        int breath = int(6 + 4 * std::sin(m_frame * 0.12));
        QRadialGradient grad(head, 40 + breath);
        grad.setColorAt(0, QColor(jadeColor.red(), jadeColor.green(), jadeColor.blue(), 100));
        grad.setColorAt(0.5, QColor(jadeColor.red(), jadeColor.green(), jadeColor.blue(), 40));
        grad.setColorAt(1, QColor(jadeColor.red(), jadeColor.green(), jadeColor.blue(), 0));
        p.setBrush(grad);
        p.setPen(Qt::NoPen);
        p.drawEllipse(head, 35 + breath, 35 + breath);
    }
    p.setBrush(QColor(45, 30, 15));
    p.setPen(QPen(QColor(218, 165, 32), 3));
    p.drawEllipse(head, 22, 22);

    QRadialGradient jade(head - QPoint(3, 3), 20);
    jade.setColorAt(0, jadeColor.lighter(130));
    jade.setColorAt(0.6, jadeColor);
    jade.setColorAt(1, jadeColor.darker(130));
    p.setBrush(jade);
    p.setPen(QPen(QColor(180, 140, 80), 1));
    p.drawEllipse(head, 19, 19);

    p.setBrush(QColor(255, 255, 255, 180));
    p.setPen(Qt::NoPen);
    p.drawEllipse(head - QPoint(6, 6), 7, 5);

    p.setBrush(ropeColor);
    p.setPen(Qt::NoPen);
    p.drawEllipse(head, 7, 7);
    p.restore();
}

void KnotUntangleGame::drawTassel(QPainter& p, const QPoint& head, const QColor& color, float sway)
{
    p.save();
    int tx = head.x();
    int ty = head.y() + 22;
    int len = 35;
    int offset = int(sway * 8);

    p.setPen(QPen(color.darker(120), 2));
    p.drawLine(tx, ty, tx + offset, ty + len);
    p.setPen(QPen(color.darker(100), 1.5));
    p.drawLine(tx - 4, ty, tx + offset - 3, ty + len - 5);
    p.drawLine(tx + 4, ty, tx + offset + 3, ty + len - 5);

    p.setBrush(color.darker(110));
    p.setPen(Qt::NoPen);
    p.drawEllipse(tx + offset, ty + len, 5, 8);
    p.restore();
}

void KnotUntangleGame::drawRope(QPainter& p, int ropeIdx)
{
    const Rope& rope = m_ropes[ropeIdx];
    if (rope.extracted) {
        QPoint target = m_slots[rope.targetSlot].rect.center();
        int dist = qAbs(rope.headPos.x() - target.x()) + qAbs(rope.headPos.y() - target.y());
        if (dist < 12) return;
    }

    p.save();
    bool topmost = isTopmost(ropeIdx);
    float alpha = (topmost || rope.extracted) ? 1.0f : 0.45f;
    p.setOpacity(alpha);

    QPainterPath path;
    path.moveTo(rope.points.first());
    for (int i = 1; i < rope.points.size(); ++i) {
        QPoint prev = rope.points[i - 1];
        QPoint curr = rope.points[i];
        QPoint ctrl((prev.x() + curr.x()) / 2, (prev.y() + curr.y()) / 2 + 25);
        path.quadTo(ctrl, curr);
    }
    QPoint last = rope.points.last();
    QPoint ctrl((last.x() + rope.headPos.x()) / 2, (last.y() + rope.headPos.y()) / 2 + 25);
    path.quadTo(ctrl, rope.headPos);

    p.setPen(QPen(QColor(20, 12, 8), 14, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawPath(path.translated(4, 4));

    QColor base = rope.color;
    p.setPen(QPen(base.darker(130), 11, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawPath(path);

    p.setPen(QPen(base, 7, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawPath(path.translated(-1, -1));

    p.setPen(QPen(base.lighter(150), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawPath(path.translated(-2, -2));

    p.restore();
}

void KnotUntangleGame::drawSlot(QPainter& p, const Slot& slot)
{
    p.save();
    p.setBrush(QColor(55, 38, 22));
    p.setPen(QPen(QColor(100, 72, 48), 3));
    p.drawRoundedRect(slot.rect, 14, 14);

    QRadialGradient grad(slot.rect.center(), slot.rect.width() * 0.6);
    grad.setColorAt(0, QColor(35, 22, 12));
    grad.setColorAt(1, QColor(50, 35, 20));
    p.setBrush(grad);
    p.setPen(QPen(QColor(80, 55, 35), 2));
    p.drawRoundedRect(slot.rect.adjusted(6, 6, -6, -6), 10, 10);

    if (!slot.filled) {
        p.setBrush(QColor(slot.jadeColor.red(), slot.jadeColor.green(), slot.jadeColor.blue(), 60));
        p.setPen(QPen(slot.color.darker(120), 2, Qt::DashLine));
        p.drawEllipse(slot.rect.center(), 18, 18);
        p.setPen(QColor(200, 190, 170, 180));
        p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 9, QFont::Bold));
        p.drawText(slot.rect, Qt::AlignCenter, slot.label);
    }
    else {
        p.setBrush(slot.color);
        p.setPen(QPen(slot.color.darker(120), 2));
        p.drawEllipse(slot.rect.center(), 24, 24);
        p.setPen(QPen(QColor(255, 220, 100), 3));
        int cx = slot.rect.center().x();
        int cy = slot.rect.center().y();
        p.drawLine(cx - 8, cy, cx - 2, cy + 8);
        p.drawLine(cx - 2, cy + 8, cx + 10, cy - 6);
        QRadialGradient glow(slot.rect.center(), 40);
        glow.setColorAt(0, QColor(slot.jadeColor.red(), slot.jadeColor.green(), slot.jadeColor.blue(), 80));
        glow.setColorAt(1, QColor(slot.jadeColor.red(), slot.jadeColor.green(), slot.jadeColor.blue(), 0));
        p.setBrush(glow);
        p.setPen(Qt::NoPen);
        p.drawEllipse(slot.rect.center(), 40, 40);
    }
    p.restore();
}

void KnotUntangleGame::drawBackground(QPainter& p)
{
    QRadialGradient bg(QPointF(width() / 2, height() / 2), qMax(width(), height()));
    bg.setColorAt(0, QColor(55, 35, 30));
    bg.setColorAt(0.6, QColor(32, 20, 18));
    bg.setColorAt(1, QColor(16, 8, 8));
    p.fillRect(rect(), bg);

    if (!m_bgPixmap.isNull()) {
        p.setOpacity(0.06);
        p.drawPixmap(rect(), m_bgPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        p.setOpacity(1.0);
    }

    p.setPen(QPen(QColor(180, 140, 60, 40), 2));
    p.setBrush(Qt::NoBrush);
    for (int corner = 0; corner < 4; ++corner) {
        int cx = (corner % 2 == 0) ? 60 : width() - 60;
        int cy = (corner < 2) ? 60 : height() - 60;
        for (int i = 0; i < 3; ++i) {
            int r = 30 + i * 15;
            p.drawEllipse(cx, cy, r, r);
        }
    }
    p.fillRect(0, 0, width(), 6, QColor(100, 70, 40));
    p.fillRect(0, height() - 6, width(), 6, QColor(100, 70, 40));
}

void KnotUntangleGame::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    if (m_errorFlash > 0.0f) {
        p.fillRect(rect(), QColor(180, 40, 40, int(60 * m_errorFlash)));
    }

    drawBackground(p);

    p.setPen(Qt::NoPen);
    for (int i = 0; i < 16; ++i) {
        float t = ((m_frame + i * 37) % 400) / 400.0f;
        int x = int(width() * 0.05f + ((i * 89) % int(width() * 0.9f)));
        int y = int(height() * 0.05f + t * height() * 0.85f);
        int a = int(20 + 25 * std::sin((m_frame + i * 13) * 0.06f));
        p.setBrush(QColor(255, 200, 100, a));
        p.drawEllipse(x, y, 3, 3);
    }

    QRect titleR(width() / 2 - 360, 22, 720, 55);
    p.setPen(QColor(0, 0, 0, 150));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 24, QFont::Bold));
    p.drawText(titleR.adjusted(2, 2, 2, 2), Qt::AlignCenter,
        QString::fromUtf8(u8"千丝结缕 · 理顺纠缠的绳结"));
    p.setPen(QColor(235, 215, 185));
    p.drawText(titleR, Qt::AlignCenter,
        QString::fromUtf8(u8"千丝结缕 · 理顺纠缠的绳结"));

    p.setPen(QColor(210, 190, 165));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 14));
    p.drawText(QRect(0, 78, width(), 30), Qt::AlignCenter,
        QString::fromUtf8(u8"拖拽绳头放入对应颜色的木匣，只能解开最上层的绳结"));

    if (m_errorShake > 0) {
        p.save();
        int shakeX = (m_errorShake % 4 < 2) ? 4 : -4;
        p.translate(shakeX, 0);
        p.setPen(QColor(235, 90, 90));
        p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 13, QFont::Bold));
        p.drawText(QRect(0, height() - 180, width(), 30), Qt::AlignCenter,
            QString::fromUtf8(u8"请先解开压在上面的绳结"));
        p.restore();
    }

    for (const auto& s : m_slots) drawSlot(p, s);

    QList<int> order;
    for (int i = 0; i < m_ropes.size(); ++i)
        if (!m_ropes[i].extracted) order.append(i);
    std::sort(order.begin(), order.end(), [this](int a, int b) {
        return m_ropes[a].layer < m_ropes[b].layer;
        });

    for (int idx : order) {
        drawRope(p, idx);
    }

    for (int idx : order) {
        bool topmost = isTopmost(idx);
        bool dragging = (idx == m_dragIdx);
        drawJadeHead(p, m_ropes[idx].headPos, m_ropes[idx].jadeColor,
            m_ropes[idx].color, topmost, dragging);
        float sway = std::sin(m_frame * 0.05f + idx) * 0.5f;
        if (dragging) sway = std::sin(m_frame * 0.2f) * 0.8f;
        drawTassel(p, m_ropes[idx].headPos, m_ropes[idx].color, sway);
    }

    for (int i = 0; i < m_ropes.size(); ++i) {
        if (m_ropes[i].extracted) {
            QPoint target = m_slots[m_ropes[i].targetSlot].rect.center();
            int dist = qAbs(m_ropes[i].headPos.x() - target.x()) + qAbs(m_ropes[i].headPos.y() - target.y());
            if (dist >= 12) {
                drawRope(p, i);
                drawJadeHead(p, m_ropes[i].headPos, m_ropes[i].jadeColor,
                    m_ropes[i].color, false, false);
            }
        }
    }

    QRect hintR(width() / 2 - 300, height() - 65, 600, 38);
    p.setBrush(QColor(0, 0, 0, 110));
    p.setPen(QPen(QColor(140, 115, 75, 90), 1));
    p.drawRoundedRect(hintR, 19, 19);
    p.setPen(QColor(200, 185, 160));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 11));
    p.drawText(hintR, Qt::AlignCenter,
        QString::fromUtf8(u8"拖拽绳头放入对应木匣    金色光晕 = 可解开    ESC 跳过"));
}

void KnotUntangleGame::mousePressEvent(QMouseEvent* event)
{
    if (!m_running) return;
    int idx = hitTestHead(event->pos());
    if (idx < 0) return;
    if (!isTopmost(idx)) {
        m_errorShake = 24;
        m_errorFlash = 1.0f;
        update();
        return;
    }
    m_dragIdx = idx;
    m_dragOffset = event->pos() - m_ropes[idx].headPos;
}

void KnotUntangleGame::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_running || m_dragIdx < 0) return;
    m_ropes[m_dragIdx].headPos = event->pos() - m_dragOffset;
    update();
}

void KnotUntangleGame::mouseReleaseEvent(QMouseEvent* event)
{
    Q_UNUSED(event)
        if (!m_running || m_dragIdx < 0) return;
    int slotIdx = hitTestSlot(event->pos());
    int ropeIdx = m_dragIdx;
    m_dragIdx = -1;

    if (slotIdx >= 0 && slotIdx == m_ropes[ropeIdx].targetSlot) {
        m_ropes[ropeIdx].extracted = true;
        m_slots[slotIdx].filled = true;
    }
    else {
        m_errorShake = 16;
        m_errorFlash = 1.0f;
    }
    update();
    checkWin();
}

void KnotUntangleGame::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        m_running = false;
        releaseKeyboard();
        m_animTimer->stop();
        emit gameSkipped(m_itemName);
        hide();
    }
}

void KnotUntangleGame::showEvent(QShowEvent*)
{
    if (parentWidget()) {
        resize(parentWidget()->size());
        move(0, 0);
    }
}