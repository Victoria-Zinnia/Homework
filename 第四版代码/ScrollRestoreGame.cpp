#include "ScrollRestoreGame.h"
#include "ResourceManager.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QTimer>
#include <cmath>

ScrollRestoreGame::ScrollRestoreGame(QWidget* parent) : QWidget(parent),
m_gridRows(3), m_gridCols(4), m_cardW(140), m_cardH(180), m_spacing(20),
m_running(false), m_firstFlip(-1), m_secondFlip(-1), m_animating(false),
m_steps(0), m_matchedPairs(0), m_totalPairs(6), m_frame(0)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setFocusPolicy(Qt::StrongFocus);
    hide();
}

void ScrollRestoreGame::startGame(const QString& itemName, const QString& bgName)
{
    m_itemName = itemName;
    m_running = true;
    m_firstFlip = -1;
    m_secondFlip = -1;
    m_animating = false;
    m_steps = 0;
    m_matchedPairs = 0;
    m_frame = 0;

    if (!bgName.isEmpty())
        m_bgPixmap = ResourceManager::instance()->getBackground(bgName);

    resize(parentWidget() ? parentWidget()->size() : QSize(1280, 720));
    move(0, 0);
    initPuzzle();

    show(); raise(); grabKeyboard();
}

void ScrollRestoreGame::initPuzzle()
{
    m_cards.clear();
    m_totalPairs = 6;
    m_gridRows = 3;
    m_gridCols = 4;

    QStringList herbNames = {
        QString::fromUtf8(u8"当归"),
        QString::fromUtf8(u8"川芎"),
        QString::fromUtf8(u8"白芍"),
        QString::fromUtf8(u8"黄芪"),
        QString::fromUtf8(u8"甘草"),
        QString::fromUtf8(u8"枸杞")
    };

    // 生成12张牌（6对）
    for (int i = 0; i < m_totalPairs; ++i) {
        for (int j = 0; j < 2; ++j) {
            HerbCard card;
            card.id = i;
            card.faceUp = false;
            card.matched = false;
            card.flipProgress = 0.0f;
            card.name = herbNames[i];
            m_cards.append(card);
        }
    }

    shuffleCards();

    // 计算布局
    int boardW = m_gridCols * m_cardW + (m_gridCols - 1) * m_spacing;
    int boardH = m_gridRows * m_cardH + (m_gridRows - 1) * m_spacing;
    m_boardOffset = QPoint((width() - boardW) / 2, (height() - boardH) / 2 + 10);

    // 分配位置
    for (int i = 0; i < m_cards.size(); ++i) {
        m_cards[i].row = i / m_gridCols;
        m_cards[i].col = i % m_gridCols;
    }
}

void ScrollRestoreGame::shuffleCards()
{
    for (int i = m_cards.size() - 1; i > 0; --i) {
        int j = QRandomGenerator::global()->bounded(i + 1);
        qSwap(m_cards[i], m_cards[j]);
    }
}

QRect ScrollRestoreGame::cardRect(int row, int col) const
{
    int x = m_boardOffset.x() + col * (m_cardW + m_spacing);
    int y = m_boardOffset.y() + row * (m_cardH + m_spacing);
    return QRect(x, y, m_cardW, m_cardH);
}

int ScrollRestoreGame::cardAt(const QPoint& pos) const
{
    for (int i = 0; i < m_cards.size(); ++i) {
        if (m_cards[i].matched) continue;
        QRect r = cardRect(m_cards[i].row, m_cards[i].col);
        if (r.contains(pos)) return i;
    }
    return -1;
}

void ScrollRestoreGame::onCardClicked(int idx)
{
    if (!m_running || m_animating) return;
    if (idx < 0 || idx >= m_cards.size()) return;
    if (m_cards[idx].matched) return;
    if (m_cards[idx].faceUp) return;

    // 翻开
    m_cards[idx].faceUp = true;
    m_cards[idx].flipProgress = 1.0f;

    if (m_firstFlip == -1) {
        m_firstFlip = idx;
        update();
    }
    else if (m_secondFlip == -1 && idx != m_firstFlip) {
        m_secondFlip = idx;
        m_steps++;
        m_animating = true;
        update();

        QTimer::singleShot(800, this, [this]() {
            checkMatch();
            });
    }
}

void ScrollRestoreGame::checkMatch()
{
    if (m_firstFlip < 0 || m_secondFlip < 0) {
        m_animating = false;
        return;
    }

    if (m_cards[m_firstFlip].id == m_cards[m_secondFlip].id) {
        // 配对成功
        m_cards[m_firstFlip].matched = true;
        m_cards[m_secondFlip].matched = true;
        m_matchedPairs++;
    }
    else {
        // 配对失败，翻回去
        m_cards[m_firstFlip].faceUp = false;
        m_cards[m_secondFlip].faceUp = false;
        m_cards[m_firstFlip].flipProgress = 0.0f;
        m_cards[m_secondFlip].flipProgress = 0.0f;
    }

    m_firstFlip = -1;
    m_secondFlip = -1;
    m_animating = false;
    update();
    checkWin();
}

void ScrollRestoreGame::checkWin()
{
    if (m_matchedPairs >= m_totalPairs) {
        m_running = false;
        releaseKeyboard();
        QTimer::singleShot(600, this, [this]() {
            emit gameFinished(m_itemName);
            hide();
            });
    }
}

void ScrollRestoreGame::drawCardBack(QPainter& p, const QRect& rect)
{
    int r = 8;
    // 阴影
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 60));
    p.drawRoundedRect(rect.adjusted(4, 4, 4, 4), r, r);

    // 牌背底色
    QLinearGradient grad(rect.topLeft(), rect.bottomRight());
    grad.setColorAt(0, QColor(55, 45, 35));
    grad.setColorAt(1, QColor(35, 28, 22));
    p.setBrush(grad);
    p.setPen(QPen(QColor(120, 100, 80), 2));
    p.drawRoundedRect(rect, r, r);

    // 装饰边框
    p.setPen(QPen(QColor(90, 75, 55), 1));
    p.drawRoundedRect(rect.adjusted(8, 8, -8, -8), r - 4, r - 4);

    // 药草图标占位
    p.setPen(QColor(120, 105, 85));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 24));
    p.drawText(rect, Qt::AlignCenter, QString::fromUtf8(u8"草"));

    // 四角装饰
    p.setPen(QPen(QColor(140, 120, 90), 2));
    int d = 12;
    p.drawLine(rect.left() + 6, rect.top() + 6, rect.left() + 6 + d, rect.top() + 6);
    p.drawLine(rect.left() + 6, rect.top() + 6, rect.left() + 6, rect.top() + 6 + d);
    p.drawLine(rect.right() - 6, rect.top() + 6, rect.right() - 6 - d, rect.top() + 6);
    p.drawLine(rect.right() - 6, rect.top() + 6, rect.right() - 6, rect.top() + 6 + d);
    p.drawLine(rect.left() + 6, rect.bottom() - 6, rect.left() + 6 + d, rect.bottom() - 6);
    p.drawLine(rect.left() + 6, rect.bottom() - 6, rect.left() + 6, rect.bottom() - 6 - d);
    p.drawLine(rect.right() - 6, rect.bottom() - 6, rect.right() - 6 - d, rect.bottom() - 6);
    p.drawLine(rect.right() - 6, rect.bottom() - 6, rect.right() - 6, rect.bottom() - 6 - d);
}

void ScrollRestoreGame::drawCardFace(QPainter& p, const QRect& rect, const HerbCard& card)
{
    int r = 8;
    // 阴影
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 60));
    p.drawRoundedRect(rect.adjusted(4, 4, 4, 4), r, r);

    // 牌面底色（药草颜色根据ID区分）
    QList<QColor> herbColors = {
        QColor(80, 50, 50),   // 当归 - 暗红
        QColor(50, 60, 80),   // 川芎 - 蓝紫
        QColor(70, 70, 70),   // 白芍 - 灰白
        QColor(80, 70, 40),   // 黄芪 - 土黄
        QColor(60, 80, 50),   // 甘草 - 草绿
        QColor(90, 60, 40)    // 枸杞 - 赭红
    };
    QColor baseColor = herbColors[card.id % herbColors.size()];

    QLinearGradient grad(rect.topLeft(), rect.bottomRight());
    grad.setColorAt(0, baseColor.lighter(130));
    grad.setColorAt(1, baseColor);
    p.setBrush(grad);
    p.setPen(QPen(QColor(180, 160, 120), 2));
    p.drawRoundedRect(rect, r, r);

    // 药草图案（简化几何图形）
    int cx = rect.center().x();
    int cy = rect.center().y();
    p.setPen(Qt::NoPen);

    // 茎
    p.setBrush(QColor(80, 120, 60));
    p.drawRoundedRect(cx - 3, cy - 20, 6, 50, 3, 3);

    // 叶子
    p.setBrush(QColor(100, 160, 80));
    p.drawEllipse(cx - 18, cy - 10, 14, 22);
    p.drawEllipse(cx + 4, cy - 5, 14, 20);
    p.drawEllipse(cx - 12, cy + 15, 12, 18);

    // 花朵/果实
    p.setBrush(QColor(220, 180, 80));
    p.drawEllipse(cx - 5, cy - 28, 10, 10);

    // 药草名
    p.setPen(QColor(255, 245, 220));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 14, QFont::Bold));
    p.drawText(QRect(rect.left(), rect.bottom() - 38, rect.width(), 28),
        Qt::AlignCenter, card.name);

    // 已配对标记
    if (card.matched) {
        p.fillRect(rect, QColor(255, 255, 255, 40));
        p.setPen(QPen(QColor(100, 230, 150), 3));
        p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 20, QFont::Bold));
        p.drawText(rect, Qt::AlignCenter, QString::fromUtf8(u8"✓"));
    }
}

void ScrollRestoreGame::drawCard(QPainter& p, const HerbCard& card)
{
    QRect rect = cardRect(card.row, card.col);

    if (card.matched) {
        // 已配对：半透明正面
        p.setOpacity(0.4);
        drawCardFace(p, rect, card);
        p.setOpacity(1.0);
        return;
    }

    if (card.faceUp) {
        drawCardFace(p, rect, card);
    }
    else {
        drawCardBack(p, rect);
    }
}

void ScrollRestoreGame::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    m_frame++;

    // 背景
    QRadialGradient bg(QPointF(width() / 2, height() / 2), qMax(width(), height()));
    bg.setColorAt(0, QColor(38, 45, 32));
    bg.setColorAt(0.6, QColor(22, 28, 18));
    bg.setColorAt(1, QColor(10, 14, 8));
    p.fillRect(rect(), bg);

    if (!m_bgPixmap.isNull()) {
        p.setOpacity(0.06);
        p.drawPixmap(rect(), m_bgPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        p.setOpacity(1.0);
    }

    // 飘落药草粒子
    p.setPen(Qt::NoPen);
    for (int i = 0; i < 12; ++i) {
        float t = ((m_frame + i * 47) % 400) / 400.0f;
        int x = int(width() * 0.05f + ((i * 89) % int(width() * 0.9f)));
        int y = int(height() * 0.05f + t * height() * 0.85f);
        int alpha = int(15 + 20 * std::sin((m_frame + i * 13) * 0.05f));
        p.setBrush(QColor(120, 160, 100, alpha));
        p.drawEllipse(x, y, 3 + (i % 3), 3 + (i % 3));
    }

    // 标题
    QRect titleR(width() / 2 - 360, 22, 720, 55);
    p.setPen(QColor(0, 0, 0, 150));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 24, QFont::Bold));
    p.drawText(titleR.adjusted(2, 2, 2, 2), Qt::AlignCenter,
        QString::fromUtf8(u8"药草识鉴 · 找回散落的本草记忆"));
    p.setPen(QColor(220, 235, 200));
    p.drawText(titleR, Qt::AlignCenter,
        QString::fromUtf8(u8"药草识鉴 · 找回散落的本草记忆"));

    // 副标题
    p.setPen(QColor(180, 200, 160));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 14));
    p.drawText(QRect(0, 78, width(), 30), Qt::AlignCenter,
        QString::fromUtf8(u8"点击卡牌翻开，找出相同的药草配对"));

    // 进度
    p.setPen(QColor(200, 220, 180));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 12));
    p.drawText(QRect(0, 108, width(), 28), Qt::AlignCenter,
        QString::fromUtf8(u8"已找回 %1 / %2 对    步数：%3")
        .arg(m_matchedPairs).arg(m_totalPairs).arg(m_steps));

    // 绘制卡牌
    for (const auto& card : m_cards) {
        drawCard(p, card);
    }

    // 底部提示
    if (m_running) {
        QRect hintR(width() / 2 - 280, height() - 65, 560, 38);
        p.setBrush(QColor(0, 0, 0, 110));
        p.setPen(QPen(QColor(100, 130, 75, 90), 1));
        p.drawRoundedRect(hintR, 19, 19);
        p.setPen(QColor(190, 210, 170));
        p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 11));
        p.drawText(hintR, Qt::AlignCenter,
            QString::fromUtf8(u8"点击翻开卡牌    连续翻开两张相同的即可配对    ESC 跳过"));
    }

    // 胜利文字
    if (!m_running && m_matchedPairs >= m_totalPairs) {
        p.setPen(QColor(255, 230, 150));
        p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 28, QFont::Bold));
        p.drawText(QRect(0, height() / 2 - 30, width(), 50), Qt::AlignCenter,
            QString::fromUtf8(u8"本草归位"));
    }
}

void ScrollRestoreGame::mousePressEvent(QMouseEvent* event)
{
    if (!m_running || m_animating) return;
    int idx = cardAt(event->pos());
    if (idx >= 0) onCardClicked(idx);
}

void ScrollRestoreGame::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        m_running = false;
        releaseKeyboard();
        emit gameSkipped(m_itemName);
        hide();
    }
}

void ScrollRestoreGame::showEvent(QShowEvent*)
{
    if (parentWidget()) {
        resize(parentWidget()->size());
        move(0, 0);
    }
}