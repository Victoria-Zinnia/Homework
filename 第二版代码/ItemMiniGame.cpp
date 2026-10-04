#include "ItemMiniGame.h"
#include "ResourceManager.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QRandomGenerator>

ItemMiniGame::ItemMiniGame(QWidget* parent) : QWidget(parent),
m_gridSize(3), m_selectedIdx(-1), m_moves(0), m_running(false)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setFocusPolicy(Qt::StrongFocus);
    hide();
}

void ItemMiniGame::startGame(const QStringList& itemNames, const QString& bgName)
{
    m_allItems = itemNames;
    m_collected.clear();
    m_selectedIdx = -1;
    m_moves = 0;
    m_running = true;

    QPixmap source;
    if (!bgName.isEmpty()) {
        source = ResourceManager::instance()->getBackground(bgName);
    }
    if (source.isNull() && !itemNames.isEmpty()) {
        source = ResourceManager::instance()->getItem(itemNames.first());
    }

    m_gridSize = 3;

    if (!source.isNull()) {
        initPuzzle(source);
    }

    resize(parentWidget() ? parentWidget()->size() : QSize(1280, 720));
    move(0, 0);
    show();
    raise();
    grabKeyboard();
}

void ItemMiniGame::initPuzzle(const QPixmap& source)
{
    m_pieces.clear();

    int avail = qMin(parentWidget() ? parentWidget()->width() - 120 : 1100,
        parentWidget() ? parentWidget()->height() - 220 : 550);
    m_puzzleAreaSize = qMin(540, avail);
    m_puzzleAreaSize = (m_puzzleAreaSize / m_gridSize) * m_gridSize;

    int s = qMin(source.width(), source.height());
    QPixmap square = source.copy((source.width() - s) / 2, (source.height() - s) / 2, s, s);
    m_fullImage = square.scaled(m_puzzleAreaSize, m_puzzleAreaSize,
        Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    int cellW = m_puzzleAreaSize / m_gridSize;
    int cellH = m_puzzleAreaSize / m_gridSize;

    for (int row = 0; row < m_gridSize; ++row) {
        for (int col = 0; col < m_gridSize; ++col) {
            int id = row * m_gridSize + col;
            SwapPiece piece;
            piece.id = id;
            piece.currentPos = id;
            piece.locked = false;
            piece.pixmap = m_fullImage.copy(col * cellW, row * cellH, cellW, cellH);
            m_pieces.append(piece);
        }
    }
    shufflePuzzle();
}

void ItemMiniGame::shufflePuzzle()
{
    // Fisher-Yates 彻底洗牌
    QList<int> perm;
    for (int i = 0; i < m_pieces.size(); ++i) perm.append(i);

    bool isSolved;
    do {
        for (int i = perm.size() - 1; i > 0; --i) {
            int j = QRandomGenerator::global()->bounded(i + 1);
            qSwap(perm[i], perm[j]);
        }
        // 检查是否所有块都在正确位置（概率极低，但保险起见）
        isSolved = true;
        for (int i = 0; i < perm.size(); ++i) {
            if (perm[i] != i) { isSolved = false; break; }
        }
    } while (isSolved);

    // 确保没有任何一块在正确位置（错位排列）
    bool hasFixedPoint;
    do {
        hasFixedPoint = false;
        for (int i = 0; i < m_pieces.size(); ++i) {
            if (perm[i] == i) {
                // 找到另一个不在正确位置的交换
                int swapWith = -1;
                for (int j = 0; j < perm.size(); ++j) {
                    if (j != i && perm[j] != j) {
                        swapWith = j;
                        break;
                    }
                }
                if (swapWith == -1) {
                    // 只剩这一块在正确位置，随便找一块交换
                    swapWith = (i + 1) % perm.size();
                }
                qSwap(perm[i], perm[swapWith]);
                hasFixedPoint = true;
                break; // 重新检查
            }
        }
    } while (hasFixedPoint);

    for (int i = 0; i < m_pieces.size(); ++i) {
        m_pieces[i].currentPos = perm[i];
        m_pieces[i].locked = false;
    }
    m_moves = 0;
}

void ItemMiniGame::trySwap(int idx)
{
    if (idx < 0 || idx >= m_pieces.size()) return;
    if (m_pieces[idx].locked) return;

    if (m_selectedIdx == -1) {
        m_selectedIdx = idx;
        update();
        return;
    }

    if (m_selectedIdx == idx) {
        m_selectedIdx = -1;
        update();
        return;
    }

    if (m_pieces[m_selectedIdx].locked) {
        m_selectedIdx = idx;
        update();
        return;
    }

    qSwap(m_pieces[m_selectedIdx].currentPos, m_pieces[idx].currentPos);
    m_moves++;
    m_selectedIdx = -1;

    checkLocks();
    update();

    if (checkWin()) {
        m_running = false;
        releaseKeyboard();
        m_collected = m_allItems;
        emit gameFinished(m_collected);
        hide();
    }
}

void ItemMiniGame::checkLocks()
{
    for (auto& p : m_pieces) {
        if (p.currentPos == p.id) {
            p.locked = true;
        }
    }
}

bool ItemMiniGame::checkWin() const
{
    for (const auto& p : m_pieces) {
        if (!p.locked) return false;
    }
    return true;
}

QRect ItemMiniGame::pieceRect(int gridPos) const
{
    int row = gridPos / m_gridSize;
    int col = gridPos % m_gridSize;
    int cellW = m_puzzleAreaSize / m_gridSize;
    int cellH = m_puzzleAreaSize / m_gridSize;
    int x = m_puzzleOffset.x() + col * cellW;
    int y = m_puzzleOffset.y() + row * cellH;
    return QRect(x, y, cellW, cellH);
}

int ItemMiniGame::pieceAt(const QPoint& pos) const
{
    int cellW = m_puzzleAreaSize / m_gridSize;
    int cellH = m_puzzleAreaSize / m_gridSize;
    int relX = pos.x() - m_puzzleOffset.x();
    int relY = pos.y() - m_puzzleOffset.y();
    int col = relX / cellW;
    int row = relY / cellH;

    if (col < 0 || col >= m_gridSize || row < 0 || row >= m_gridSize) return -1;
    int targetPos = row * m_gridSize + col;

    for (int i = 0; i < m_pieces.size(); ++i) {
        if (m_pieces[i].currentPos == targetPos) return i;
    }
    return -1;
}

void ItemMiniGame::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    p.fillRect(rect(), QColor(10, 10, 20, 240));

    if (m_pieces.isEmpty()) return;

    m_puzzleOffset = QPoint((width() - m_puzzleAreaSize) / 2,
        (height() - m_puzzleAreaSize) / 2 + 10);
    QRect boardRect(m_puzzleOffset.x(), m_puzzleOffset.y(),
        m_puzzleAreaSize, m_puzzleAreaSize);

    // 半透明原图提示
    if (!m_fullImage.isNull()) {
        p.setOpacity(0.18);
        p.drawPixmap(boardRect, m_fullImage);
        p.setOpacity(1.0);
    }

    p.fillRect(boardRect, QColor(25, 25, 40, 200));
    p.setPen(QPen(QColor(70, 70, 110), 2));
    p.drawRect(boardRect);

    // 绘制碎片
    for (int i = 0; i < m_pieces.size(); ++i) {
        const SwapPiece& piece = m_pieces[i];
        QRect r = pieceRect(piece.currentPos);

        // 先画图片（无论是否锁定都画）
        if (!piece.pixmap.isNull()) {
            p.drawPixmap(r, piece.pixmap);
        }
        else {
            // 图片加载失败的兜底：画红叉，绝不画绿底
            p.fillRect(r, QColor(60, 30, 30));
            p.setPen(QColor(255, 80, 80));
            p.drawLine(r.topLeft(), r.bottomRight());
            p.drawLine(r.topRight(), r.bottomLeft());
        }

        if (piece.locked) {
            // 已锁定：只画绿色细边框 + 小勾，绝不画任何填充
            p.setPen(QPen(QColor(100, 230, 150), 2));
            p.drawRect(r);

            // 右下角小勾
            p.setPen(QPen(QColor(100, 230, 150), 2));
            p.drawLine(r.right() - 10, r.bottom() - 6, r.right() - 7, r.bottom() - 3);
            p.drawLine(r.right() - 7, r.bottom() - 3, r.right() - 3, r.bottom() - 9);
        }
        else if (i == m_selectedIdx) {
            // 选中：黄色边框
            p.setPen(QPen(QColor(255, 230, 100), 3));
            p.drawRect(r.adjusted(-2, -2, 2, 2));
        }
        else {
            // 普通：暗色细边框
            p.setPen(QPen(QColor(55, 55, 85), 1));
            p.drawRect(r);
        }
    }

    // 标题
    p.setPen(QColor(224, 208, 176));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 18, QFont::Bold));
    p.drawText(QRect(0, 18, width(), 40), Qt::AlignCenter,
        QString::fromUtf8(u8"流光拾忆 — 重组记忆碎片"));

    int lockedCount = 0;
    for (const auto& p : m_pieces) if (p.locked) lockedCount++;

    p.setPen(QColor(180, 180, 180));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 12));
    p.drawText(QRect(0, 58, width(), 28), Qt::AlignCenter,
        QString::fromUtf8(u8"步数：%1  |  已归位：%2 / %3")
        .arg(m_moves).arg(lockedCount).arg(m_pieces.size()));

    p.setPen(QColor(140, 140, 160));
    p.setFont(QFont(QString::fromUtf8(u8"Microsoft YaHei"), 11));
    QString hint = (m_selectedIdx == -1)
        ? QString::fromUtf8(u8"点击碎片选中，再点击另一块交换位置，ESC 跳过")
        : QString::fromUtf8(u8"已选中碎片，点击另一块进行交换");
    p.drawText(rect().adjusted(0, -35, 0, 0), Qt::AlignBottom | Qt::AlignHCenter, hint);
}

void ItemMiniGame::mousePressEvent(QMouseEvent* event)
{
    if (!m_running) return;
    int idx = pieceAt(event->pos());
    if (idx >= 0) trySwap(idx);
}

void ItemMiniGame::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        m_running = false;
        releaseKeyboard();
        emit gameSkipped(m_allItems);
        hide();
    }
}

void ItemMiniGame::showEvent(QShowEvent*)
{
    if (parentWidget()) {
        resize(parentWidget()->size());
        move(0, 0);
    }
}