#ifndef WARDROBESCENE_H
#define WARDROBESCENE_H

#include "SceneBase.h"
#include <QList>

class QPushButton;

class WardrobeScene : public SceneBase
{
    Q_OBJECT
public:
    explicit WardrobeScene(QWidget* parent = nullptr);

    void onEnter() override;
    void onExit() override;

private:
    void setupGrid();
    void onSkinSelected(int skinId);
    void updateSelection();

    int m_selectedSkin;
    QList<QPushButton*> m_skinButtons;
};

#endif // WARDROBESCENE_H
