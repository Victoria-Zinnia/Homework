#ifndef SCROLLRESTOREGAME_H
#define SCROLLRESTOREGAME_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QPixmap>

struct HerbCard {
    int id;           // 配对ID (0~7)
    int row, col;     // 网格位置
    bool faceUp;      // 是否翻开
    bool matched;     // 是否已配对
    float flipProgress; // 翻牌动画进度 0=背面, 1=正面
    QString name;     // 药草名
};

class ScrollRestoreGame : public QWidget
{
    Q_OBJECT
public:
    explicit ScrollRestoreGame(QWidget* parent = nullptr);
    void startGame(const QString& itemName, const QString& bgName = QString());

signals:
    void gameFinished(const QString& itemName);
    void gameSkipped(const QString& itemName);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void showEvent(QShowEvent*) override;

private:
    void initPuzzle();
    void shuffleCards();
    void drawCard(QPainter& p, const HerbCard& card);
    void drawCardBack(QPainter& p, const QRect& rect);
    void drawCardFace(QPainter& p, const QRect& rect, const HerbCard& card);
    int cardAt(const QPoint& pos) const;
    QRect cardRect(int row, int col) const;
    void onCardClicked(int idx);
    void checkMatch();
    void checkWin();

    QList<HerbCard> m_cards;
    int m_gridRows;
    int m_gridCols;
    int m_cardW;
    int m_cardH;
    int m_spacing;
    QPoint m_boardOffset;
    QString m_itemName;
    bool m_running;
    int m_firstFlip;      // 第一张翻开的索引，-1=无
    int m_secondFlip;     // 第二张翻开的索引
    bool m_animating;     // 是否在翻牌动画中
    int m_steps;          // 步数
    int m_matchedPairs;   // 已配对数
    int m_totalPairs;     // 总对数
    QPixmap m_bgPixmap;
    int m_frame;
};

#endif