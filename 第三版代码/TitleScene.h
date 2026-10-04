#ifndef TITLESCENE_H
#define TITLESCENE_H

#include "SceneBase.h"
#include <QList>

class QPushButton;
class QLabel;
class QWidget;
class QPropertyAnimation;

class TitleScene : public SceneBase
{
    Q_OBJECT
public:
    explicit TitleScene(QWidget* parent = nullptr);

    void setSkipBoot(bool skip);

    void onEnter() override;
    void onExit() override;

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    enum State { BOOT, TITLE };
    State m_state;

    bool m_skipBoot = false;

    void setupBoot();
    void setupTitle();
    void stopBootAnimations();
    void showBootButton();  // ← 新增：显示开机页进入按钮

    QWidget* m_bootOverlay = nullptr;
    QLabel* m_hintLabel = nullptr;
    QList<QPushButton*> m_titleButtons;
    QPropertyAnimation* m_bootAnim = nullptr;
    QPropertyAnimation* m_hintAnim = nullptr;
};

#endif // TITLESCENE_H

