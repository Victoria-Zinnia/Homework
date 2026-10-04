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
    void drawJade(QPainter& p, int idx);
    void drawProgress(QPainter& p);
    void drawSnow(QPainter& p);
    QPoint jadeCenter(int idx) const;
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
    int m_gap;
    QPoint m_boardOffset;
    QList<QPointF> m_snowflakes;
    QList<float> m_snowSpeed;
    QStringList m_jadeNames;
};

#endif
