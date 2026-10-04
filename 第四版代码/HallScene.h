#ifndef HALLSCENE_H
#define HALLSCENE_H

#include "SceneBase.h"
#include "GameData.h"

class QPushButton;
class QLabel;

class HallScene : public SceneBase
{
    Q_OBJECT
public:
    explicit HallScene(QWidget* parent = nullptr);

    void onEnter() override;
    void onExit() override;

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUI();
    void checkFinalChapter();
    void onLevelClicked(LevelID level);

    QLabel* m_narrator = nullptr;
    QLabel* m_skinLabel = nullptr;
    QList<QPushButton*> m_levelButtons;
    QPushButton* m_finalBtn = nullptr;
    QPushButton* m_backBtn = nullptr;
    QPushButton* m_wardrobeBtn = nullptr;
    QPushButton* m_saveBtn = nullptr;   // ¡û ÐÂÔö
};

#endif // HALLSCENE_H

