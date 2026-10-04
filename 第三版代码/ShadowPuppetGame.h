#ifndef SHADOWPUPPETGAME_H
#define SHADOWPUPPETGAME_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QPixmap>
#include <QTimer>

class ShadowPuppetGame : public QWidget
{
    Q_OBJECT
public:
    explicit ShadowPuppetGame(QWidget* parent = nullptr);
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
    void nextRound();
    void playerClick(int idx);
    void drawBackground(QPainter& p);
    void drawLockPlate(QPainter& p);
    void drawJade(QPainter& p, int idx, int cx, int cy, int size);
    void drawPattern(QPainter& p, int idx, int cx, int cy, int size);
    void drawProgress(QPainter& p);
    void drawSnow(QPainter& p);
    int hitTestJade(const QPoint& pos) const;
    void updateAnimations();

    static const int JADE_COUNT = 9;
    QString m_itemName;
    bool m_running;
    int m_round;
    int m_sequenceLen;
    QList<int> m_sequence;
    int m_playerStep;
    int m_showTimer;
    bool m_inputEnabled;
    int m_errorShake;
    float m_jadeGlow[JADE_COUNT];
    float m_clickRipple[JADE_COUNT];
    float m_lockOpen;
    QPixmap m_bgPixmap;
    int m_frame;
    QTimer* m_animTimer;
    float m_winGlow;
    int m_cellSize;
    QPoint m_boardCenter;
    QList<QPointF> m_snowflakes;
    QList<float> m_snowSpeed;
};

#endif
