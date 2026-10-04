#ifndef MIRRORPETALGAME_H
#define MIRRORPETALGAME_H

#include <QWidget>
#include <QPixmap>
#include <QString>
#include <QVector>
#include <QTimer>

struct Lantern {
    bool lit;
    float pulse;
};

class MirrorPetalGame : public QWidget
{
    Q_OBJECT
public:
    explicit MirrorPetalGame(QWidget* parent = nullptr);
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
    void applyToggle(int row, int col);
    void drawLantern(QPainter& p, const QRect& rect, bool lit, float pulse);
    void drawBoard(QPainter& p);
    bool checkWin() const;
    int countLit() const;
    QRect cellRect(int row, int col) const;
    QPoint cellAt(const QPoint& pos) const;

    QVector<QVector<Lantern>> m_grid;
    int m_gridSize;
    int m_cellSize;
    QPoint m_boardOffset;
    QString m_itemName;
    bool m_running;
    QTimer* m_animTimer;
    int m_frame;
};

#endif