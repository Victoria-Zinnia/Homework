#ifndef MORTISEPUZZLEGAME_H
#define MORTISEPUZZLEGAME_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QPixmap>
#include <QTimer>

class MortisePuzzleGame : public QWidget
{
    Q_OBJECT
public:
    explicit MortisePuzzleGame(QWidget* parent = nullptr);
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
    void drawBackground(QPainter& p);
    void drawHints(QPainter& p);
    void drawGrid(QPainter& p);
    void drawCell(QPainter& p, int r, int c, const QRect& rect);
    QRect cellRect(int row, int col) const;
    int cellAt(const QPoint& pos, int& outRow, int& outCol) const;
    void toggleCell(int row, int col);
    bool checkWin() const;
    void updateAnimations();

    static const int SIZE = 5;
    int m_grid[SIZE][SIZE];      // 0=empty, 1=filled, 2=marked-empty(X)
    bool m_solution[SIZE][SIZE];

    QList<QList<int>> m_rowHints;
    QList<QList<int>> m_colHints;

    QString m_itemName;
    bool m_running;
    QPixmap m_bgPixmap;
    int m_frame;
    QTimer* m_animTimer;
    float m_winGlow;
    QPoint m_boardOffset;
    int m_cellSize;
    int m_hintSpace;
};

#endif