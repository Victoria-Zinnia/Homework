#ifndef TITLESCENE_H
#define TITLESCENE_H

#include "SceneBase.h"
#include <QList>

class QPushButton;
class QLabel;
class QWidget;
class QPropertyAnimation;  // ← 新增

class TitleScene : public SceneBase
{
    Q_OBJECT
public:
    explicit TitleScene(QWidget* parent = nullptr);

    void onEnter() override;
    void onExit() override;

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    enum State { BOOT, TITLE };
    State m_state;

    void setupBoot();
    void setupTitle();
    void stopBootAnimations();  // ← 新增

    QWidget* m_bootOverlay = nullptr;
    QLabel* m_hintLabel = nullptr;
    QList<QPushButton*> m_titleButtons;
    QPropertyAnimation* m_bootAnim = nullptr;  // ← 新增
    QPropertyAnimation* m_hintAnim = nullptr;  // ← 新增
};

#endif // TITLESCENE_H
