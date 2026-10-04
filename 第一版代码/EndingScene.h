#ifndef ENDINGSCENE_H
#define ENDINGSCENE_H

#include "SceneBase.h"
#include "GameData.h"

class EndingScene : public SceneBase
{
    Q_OBJECT
public:
    explicit EndingScene(QWidget* parent = nullptr);
    void setFinalEnding(FinalEnding ending);

    void onEnter() override;
    void onExit() override;

private:
    void runBroken();
    void runDetained();
    void runReconciled();
    void runTrueEnding();

    FinalEnding m_finalEnding;
    int m_index;
};

#endif // ENDINGSCENE_H
