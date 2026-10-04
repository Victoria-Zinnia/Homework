#ifndef ENDINGSCENE_H
#define ENDINGSCENE_H

#include "SceneBase.h"
#include "GameData.h"

class DialogBox;
class QPushButton;

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
    void onBackToHallClicked();

private:
    void runBroken();
    void runDetained();
    void runReconciled();
    void runTrueEnding();
    void advanceEnding();
    void showLine(const QString& speaker, const QString& text, const QString& voice);
    void showNarration(const QString& text, const QString& voice);
    void updateDialogPosition();
    void showBackToHallButton();

    FinalEnding m_finalEnding;
    int m_index;
    DialogBox* m_dialog;
    QPushButton* m_backToHallBtn = nullptr;
};

#endif // ENDINGSCENE_H


