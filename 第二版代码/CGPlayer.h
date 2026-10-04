#ifndef CGPLAYER_H
#define CGPLAYER_H

#include <QWidget>
#include <QPixmap>
#include <QPropertyAnimation> 
#include <functional>

class CGPlayer : public QWidget
{
    Q_OBJECT
public:
    explicit CGPlayer(QWidget* parent = nullptr);

    void playCG(const QString& cgName);
    void setOnFinished(std::function<void()> callback);

signals:
    void cgFinished();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    QPixmap m_cgPixmap;
    bool m_showing;
    std::function<void()> m_callback;
    QPropertyAnimation* m_fadeAnim = nullptr;
};

#endif // CGPLAYER_H
