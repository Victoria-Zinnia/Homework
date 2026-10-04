#ifndef ITEMMINIGAME_H
#define ITEMMINIGAME_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QPixmap>
#include <QPoint>

struct GameItem {
    QString name;
    QPoint pos;
    bool collected;
    QPixmap icon;
    float phase;
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
    void timerEvent(QTimerEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void checkCollisions();
    QPoint randomPos() const;

    QList<GameItem> m_items;
    QStringList m_collected;
    QPoint m_playerPos;
    int m_timerId;
    float m_time;
    bool m_running;
    QPixmap m_playerPixmap;
    QPixmap m_backgroundPixmap;
};

#endif // ITEMMINIGAME_H
