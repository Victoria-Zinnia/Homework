#include "MiniGameSelect.h"
#include "ResourceManager.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>

MiniGameSelect::MiniGameSelect(QWidget* parent) : QWidget(parent),
m_page(Page_Select), m_selectedIndex(-1), m_hoverCard(-1), m_hoverStart(false)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    hide();
}

void MiniGameSelect::startSelect(const QString& seasonTitle, const QList<MiniGameLevel>& levels)
{
    m_seasonTitle = seasonTitle;
    m_levels = levels;
    m_page = Page_Select;
    m_selectedIndex = -1;
    m_hoverCard = -1;
    m_hoverStart = false;

    resize(parentWidget() ? parentWidget()->size() : QSize(1280, 720));
    move(0, 0);
    show(); raise(); grabKeyboard();
}

void MiniGameSelect::setCompletedItems(const QSet<QString>& items)
{
    m_completedItems = items;
}

void MiniGameSelect::setBackgroundName(const QString& name)
{
    m_bgName = name;
}

QRect MiniGameSelect::cardRect(int index) const
{
    int count = m_levels.size();
    int cardW = 240;
    int cardH = 340;
    int spacing = 50;

    int totalW = count * cardW + (count - 1) * spacing;
    int startX = (width() - totalW) / 2;
    int y = height() / 2 - cardH / 2;

    return QRect(startX + index * (cardW + spacing), y, cardW, cardH);
}

int MiniGameSelect::cardAt(const QPoint& pos) const
{
    for (int i = 0; i < m_levels.size(); ++i) {
        if (cardRect(i).contains(pos)) return i;
    }
    return -1;
}

QRect MiniGameSelect::startBtnRect() const
{
    return QRect(width() / 2 - 120, height() - 160, 240, 56);
}

void MiniGameSelect::drawSelectPage(QPainter& p)
{
    // 深色背景
    p.fillRect(rect(), QColor(18, 12, 16));

    // 季节大标题
    p.setPen(QColor(255, 235, 200));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 32, QFont::Bold));
    p.drawText(QRect(0, 50, width(), 70), Qt::AlignCenter, m_seasonTitle);

    p.setPen(QColor(200, 180, 160));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 14));
    p.drawText(QRect(0, 130, width(), 30), Qt::AlignCenter,
        QString::fromUtf8(u8"选择关卡，获取对应道具"));

    // 关卡卡片
    for (int i = 0; i < m_levels.size(); ++i) {
        QRect card = cardRect(i);
        bool hovered = (i == m_hoverCard);
        bool completed = m_completedItems.contains(m_levels[i].itemName);

        // 阴影
        p.fillRect(card.adjusted(6, 6, 6, 6), QColor(0, 0, 0, 100));

        QColor bgColor = completed ? QColor(45, 40, 48) : (hovered ? QColor(70, 55, 65) : QColor(48, 36, 44));
        p.fillRect(card, bgColor);
        p.setPen(QPen(hovered && !completed ? QColor(255, 220, 150) : QColor(140, 110, 90), hovered && !completed ? 3 : 2));
        p.drawRect(card);

        // 道具贴图（封面）
        QPixmap itemPix = ResourceManager::instance()->getItem(m_levels[i].itemName);
        if (!itemPix.isNull()) {
            QPixmap scaled = itemPix.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            int imgX = card.center().x() - scaled.width() / 2;
            p.drawPixmap(imgX, card.top() + 30, scaled);
        }

        // 关卡名
        p.setPen(QColor(255, 240, 220));
        p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 17, QFont::Bold));
        p.drawText(QRect(card.left(), card.top() + 200, card.width(), 40),
            Qt::AlignCenter, m_levels[i].displayName);

        // 奖励道具
        p.setPen(QColor(220, 200, 170));
        p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 13));
        p.drawText(QRect(card.left(), card.top() + 245, card.width(), 30),
            Qt::AlignCenter, QString::fromUtf8(u8"奖励：") + m_levels[i].itemName);

        // 已完成遮罩
        if (completed) {
            p.fillRect(card, QColor(20, 20, 20, 160));
            p.setPen(QColor(180, 180, 180));
            p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 18, QFont::Bold));
            p.drawText(card, Qt::AlignCenter, QString::fromUtf8(u8"✓ 已完成"));
        }
    }

    // 底部提示
    p.setPen(QColor(160, 140, 120));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 12));
    p.drawText(QRect(0, height() - 70, width(), 30), Qt::AlignCenter,
        QString::fromUtf8(u8"点击卡片查看规则    ESC 返回"));
}

void MiniGameSelect::drawRulePage(QPainter& p)
{
    // ========== 修改：使用季节道具背景图 ==========
    QPixmap bgPix = ResourceManager::instance()->getBackground(m_bgName);
    if (!bgPix.isNull()) {
        p.drawPixmap(rect(), bgPix.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }
    else {
        p.fillRect(rect(), QColor(25, 18, 22));
    }

    // 暗色遮罩保证文字可读
    p.fillRect(rect(), QColor(12, 8, 10, 210));

    const MiniGameLevel& lvl = m_levels[m_selectedIndex];

    // 标题
    p.setPen(QColor(255, 235, 200));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 38, QFont::Bold));
    p.drawText(QRect(0, 90, width(), 70), Qt::AlignCenter, lvl.displayName);

    // 奖励道具
    p.setPen(QColor(255, 220, 150));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 18));
    p.drawText(QRect(0, 175, width(), 40), Qt::AlignCenter,
        QString::fromUtf8(u8"通关奖励：【") + lvl.itemName + QString::fromUtf8(u8"】"));

    // 规则文字
    p.setPen(QColor(245, 235, 215));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 15));
    QRect textRect(180, 240, width() - 360, height() - 460);
    p.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, lvl.ruleText);

    // 开始按钮
    QRect btn = startBtnRect();
    QColor btnColor = m_hoverStart ? QColor(235, 195, 100) : QColor(195, 155, 70);
    p.fillRect(btn, btnColor);
    p.setPen(QPen(QColor(255, 230, 170), 2));
    p.drawRect(btn);
    p.setPen(QColor(40, 30, 15));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 18, QFont::Bold));
    p.drawText(btn, Qt::AlignCenter, QString::fromUtf8(u8"开始挑战"));

    // 底部提示
    p.setPen(QColor(210, 195, 175));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 12));
    p.drawText(QRect(0, height() - 70, width(), 30), Qt::AlignCenter,
        QString::fromUtf8(u8"ESC 返回关卡选择"));
}

void MiniGameSelect::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    if (m_page == Page_Select) {
        drawSelectPage(p);
    }
    else {
        drawRulePage(p);
    }
}

void MiniGameSelect::mousePressEvent(QMouseEvent* event)
{
    if (m_page == Page_Select) {
        int idx = cardAt(event->pos());
        if (idx >= 0 && !m_completedItems.contains(m_levels[idx].itemName)) {
            m_selectedIndex = idx;
            m_page = Page_Rule;
            m_hoverStart = false;
            update();
        }
    }
    else if (m_page == Page_Rule) {
        if (startBtnRect().contains(event->pos())) {
            hide();
            releaseKeyboard();
            emit levelStarted(m_levels[m_selectedIndex].itemName);
        }
    }
}

void MiniGameSelect::mouseMoveEvent(QMouseEvent* event)
{
    if (m_page == Page_Select) {
        int prev = m_hoverCard;
        m_hoverCard = cardAt(event->pos());
        if (prev != m_hoverCard) update();
    }
    else if (m_page == Page_Rule) {
        bool prev = m_hoverStart;
        m_hoverStart = startBtnRect().contains(event->pos());
        if (prev != m_hoverStart) update();
    }
}

void MiniGameSelect::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        if (m_page == Page_Rule) {
            m_page = Page_Select;
            m_hoverStart = false;
            update();
        }
        else {
            hide();
            releaseKeyboard();
            emit selectCancelled();
        }
    }
}

void MiniGameSelect::showEvent(QShowEvent*)
{
    if (parentWidget()) {
        resize(parentWidget()->size());
        move(0, 0);
    }
}