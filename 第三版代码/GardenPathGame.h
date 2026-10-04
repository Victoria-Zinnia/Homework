#ifndef GARDENPATHGAME_H
#define GARDENPATHGAME_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QPixmap>
#include <QPoint>
#include <QRect>

struct PipeCell {
    bool open[4];    // 上 右 下 左
    bool fixed;      // 起点/终点/障碍 不可旋转
    bool flooded;    // 是否已通水
    int type;        // 0直 1弯 2三通 3十字 4障碍 5起点 6终点
    int rotation;    // 当前旋转 0~3
};

class GardenPathGame : public QWidget
{
    Q_OBJECT
public:
    explicit GardenPathGame(QWidget* parent = nullptr);
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
    void generatePuzzle();
    void rotateCell(int row, int col);
    void updateWaterFlow();
    void checkWin();
    QRect cellRect(int row, int col) const;
    void drawCell(QPainter& p, int row, int col, const QRect& rect);
    void drawChannel(QPainter& p, const PipeCell& cell, const QRect& rect, bool water);
    void drawBackground(QPainter& p);

    QList<QList<PipeCell>> m_grid;
    QPoint m_startPos;
    QPoint m_endPos;
    QString m_itemName;
    bool m_running;
    bool m_waterFlowing;
    int m_cellSize;
    QPoint m_boardOffset;
    int m_rows, m_cols;
    QPixmap m_bgPixmap;
    int m_frame;
};

#endif
