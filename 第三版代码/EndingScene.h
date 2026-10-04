#ifndef ENDINGSCENE_H
#define ENDINGSCENE_H

#include "SceneBase.h"
#include "GameData.h"

class DialogBox;

class EndingScene : public SceneBase
{
    Q_OBJECT
public:
    explicit EndingScene(QWidget* parent = nullptr);
    void setFinalEnding(FinalEnding ending);

    void onEnter() override;
    void onExit() override;

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onDialogClicked();

private:
    void runBroken();
    void runDetained();
    void runReconciled();
    void runTrueEnding();
    void advanceEnding();
    void showLine(const QString& speaker, const QString& text, const QString& voice);
    void showNarration(const QString& text, const QString& voice);
    void updateDialogPosition();

    FinalEnding m_finalEnding;
    int m_index;
    DialogBox* m_dialog;
};

#endif // ENDINGSCENE_H


