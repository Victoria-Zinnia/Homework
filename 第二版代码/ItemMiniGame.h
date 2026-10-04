#ifndef ITEMMINIGAME_H
#define ITEMMINIGAME_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QPixmap>

struct SwapPiece {
    QPixmap pixmap;
    int id;
    int currentPos;
    bool locked;
};

class ItemMiniGame : public QWidget
{
    Q_OBJECT
public:
    explicit ItemMiniGame(QWidget* parent = nullptr);
    void startGame(const QStringList& itemNames, const QString& bgName = QString());

signals:
    void gameFinished(const QStringList& collectedItems);
    void gameSkipped(const QStringList& itemNames);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void initPuzzle(const QPixmap& source);
    void shufflePuzzle();
    void ensureNotSolved();   // 新增：确保初始不是完成状态
    void trySwap(int idx);
    void checkLocks();
    bool checkWin() const;
    QRect pieceRect(int gridPos) const;
    int pieceAt(const QPoint& mousePos) const;

    QList<SwapPiece> m_pieces;
    int m_gridSize;
    int m_selectedIdx;
    int m_moves;
    bool m_running;

    QStringList m_allItems;
    QStringList m_collected;

    QPixmap m_fullImage;
    int m_puzzleAreaSize;
    QPoint m_puzzleOffset;
};

#endif // ITEMMINIGAME_H
