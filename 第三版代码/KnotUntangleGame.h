#ifndef KNOTUNTANGLEGAME_H
#define KNOTUNTANGLEGAME_H

#include <QWidget>
#include <QList>
#include <QPoint>
#include <QRect>
#include <QString>
#include <QPixmap>
#include <QColor>
#include <QTimer>

struct Rope {
    QList<QPoint> points;   // 贝塞尔控制点
    QPoint headPos;         // 绳头实时位置
    QPoint restPos;         // 绳头静止位置
    QColor color;           // 绳子主题色
    QColor jadeColor;       // 玉佩颜色
    int layer;              // 层级，越大越在上
    bool extracted;         // 是否已归位
    int targetSlot;         // 目标槽位索引
    QString name;           // 绳名
    float hoverGlow;        // 悬停光晕强度
};

struct Slot {
    QRect rect;
    QColor color;
    QColor jadeColor;
    QString label;
    bool filled;
};

class KnotUntangleGame : public QWidget
{
    Q_OBJECT
public:
    explicit KnotUntangleGame(QWidget* parent = nullptr);
    void startGame(const QString& itemName, const QString& bgName = QString());

signals:
    void gameFinished(const QString& itemName);
    void gameSkipped(const QString& itemName);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void showEvent(QShowEvent*) override;

private:
    void initPuzzle();
    void drawRope(QPainter& p, int ropeIdx);
    void drawJadeHead(QPainter& p, const QPoint& head, const QColor& jadeColor, const QColor& ropeColor, bool glow, bool dragging);
    void drawSlot(QPainter& p, const Slot& slot);
    void drawBackground(QPainter& p);
    void drawTassel(QPainter& p, const QPoint& head, const QColor& color, float sway);
    int hitTestHead(const QPoint& pos) const;
    int hitTestSlot(const QPoint& pos) const;
    bool isTopmost(int index) const;
    void updateAnimations();
    void checkWin();

    QList<Rope> m_ropes;
    QList<Slot> m_slots;
    QString m_itemName;
    bool m_running;
    int m_dragIdx;
    QPoint m_dragOffset;
    QPixmap m_bgPixmap;
    QTimer* m_animTimer;
    int m_frame;
    int m_errorShake;
    float m_errorFlash;     // 错误时屏幕泛红强度
};

#endif