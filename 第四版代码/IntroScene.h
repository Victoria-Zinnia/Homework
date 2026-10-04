#ifndef INTROSCENE_H
#define INTROSCENE_H

#include "SceneBase.h"
#include <QList>

class QLabel;
class QPushButton;
class QPropertyAnimation;
class QGraphicsOpacityEffect;

class IntroScene : public SceneBase
{
    Q_OBJECT
public:
    explicit IntroScene(QWidget* parent = nullptr);

    void onEnter() override;
    void onExit() override;

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    enum State { FadingIn, Waiting, FadingOut, Finished };
    State m_state;

    void setupPage(int index);
    void startFadeIn();
    void startFadeOut();
    void advancePage();
    void finishIntro();

    struct Page {
        QString title;
        QString subtitle;
        QString body;
        QString quote;
    };
    QList<Page> m_pages;
    int m_currentPage;

    QWidget* m_pageWidget;
    QLabel* m_titleLabel;
    QLabel* m_subtitleLabel;
    QLabel* m_bodyLabel;
    QLabel* m_quoteLabel;
    QPushButton* m_skipBtn;

    QPropertyAnimation* m_fadeAnim;
    QGraphicsOpacityEffect* m_opacityEffect;

    static const int FADE_DURATION = 1200;
    static const int HOLD_DURATION = 6000;
};

#endif // INTROSCENE_H
