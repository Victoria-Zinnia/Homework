#ifndef PORCELAINRESTOREGAME_H
#define PORCELAINRESTOREGAME_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QPixmap>
#include <QTimer>

struct SnowCell {
    int type; // 0¿ÕµØ 1Ç½ 2Ñ©ÍÅ 3¿Õ¿Ó 4Âú¿Ó
};

class PorcelainRestoreGame : public QWidget
{
    Q_OBJECT
public:
    explicit PorcelainRestoreGame(QWidget* parent = nullptr);
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
    void drawGrid(QPainter& p);
    void drawSnow(QPainter& p);
    void trySlide(int row, int col);
    bool checkWin() const;
    void updateAnimations();
    QPoint cellAt(const QPoint& pos) const;
    QRect cellRect(int row, int col) const;

    static const int GRID = 8;
    SnowCell m_grid[GRID][GRID];
    QString m_itemName;
    bool m_running;
    QPoint m_selectedCell;
    int m_cellSize;
    QPoint m_boardOffset;
    QPixmap m_bgPixmap;
    int m_frame;
    QTimer* m_animTimer;
    float m_winGlow;
    QList<QPointF> m_snowflakes;
    QList<float> m_snowSpeed;
};

#endif
