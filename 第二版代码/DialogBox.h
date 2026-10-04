#ifndef DIALOGBOX_H
#define DIALOGBOX_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QString>
#include <QPixmap>

class DialogBox : public QWidget
{
    Q_OBJECT
public:
    enum SpeakerSide { SideCenter, SideLeft, SideRight };

    explicit DialogBox(QWidget* parent = nullptr);

    void setSpeaker(const QString& name);
    void setText(const QString& text);
    void setVoice(const QString& voiceFile);
    void startTyping();
    void skipTyping();
    void clear();

    void setSpeakerSide(SpeakerSide side);
    SpeakerSide speakerSide() const { return m_side; }
    void setBackgroundImage(const QPixmap& pix);

    bool isTyping() const;
    bool isFinished() const;

signals:
    void textFinished();
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onTypingTimer();

private:
    QLabel* m_nameLabel;
    QLabel* m_textLabel;
    QString m_fullText;
    QString m_voiceFile;
    int m_currentIndex;
    QTimer* m_typingTimer;
    bool m_typing;
    bool m_finished;
    static const int TYPING_INTERVAL = 35;

    QPixmap m_bgPixmap;
    SpeakerSide m_side;

private:
    void updateLayout();   // ¡û ÐÂÔö
};

#endif // DIALOGBOX_H
