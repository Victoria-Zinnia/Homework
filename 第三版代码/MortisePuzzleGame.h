#ifndef MORTISEPUZZLEGAME_H
#define MORTISEPUZZLEGAME_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QPixmap>
#include <QTimer>

struct Ring {
    QPixmap pixmap;
    int angle;
    int targetAngle;
    bool locked;
    int innerR;
    int outerR;
    float clickRipple;
};

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
    void generateRingPixmaps();
    void drawRingPattern(QPainter& p, int ringIdx, int r);
    void drawBackground(QPainter& p);
    void drawSnow(QPainter& p);
    int hitTestRing(const QPoint& pos) const;
    void checkLocks();
    bool checkWin() const;
    void updateAnimations();

    QList<Ring> m_rings;
    QString m_itemName;
    bool m_running;
    QPoint m_center;
    QPixmap m_bgPixmap;
    int m_frame;
    QTimer* m_animTimer;
    float m_winGlow;
    QList<QPointF> m_snowflakes;
    QList<float> m_snowSpeed;
};

#endif