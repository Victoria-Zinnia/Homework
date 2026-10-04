#ifndef LEVELSCENE_H
#define LEVELSCENE_H

#include "ItemMiniGame.h"
#include "SceneBase.h"
#include "GameData.h"
#include "DialogBox.h"
#include "ChoiceMenu.h"
#include "ItemPopup.h"
#include "CGPlayer.h"
#include <QSet>

class LevelScene : public SceneBase
{
    Q_OBJECT
public:
    explicit LevelScene(QWidget* parent = nullptr);
    void setLevel(LevelID level);

    void onEnter() override;
    void onExit() override;

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onDialogClicked();
    void onChoiceMade(int index);
    void onCGFinished();
    void onMiniGameFinished(const QStringList& items);
    void onMiniGameSkipped(const QStringList& items);
    void startItemMiniGame(const QStringList& items, int nextIndex, const QString& bgName = QString());

private:
    void advanceScript();
    void showLine(const QString& speaker, const QString& text, const QString& voice);
    void showNarration(const QString& text, const QString& voice);
    void showOption(const QStringList& options);
    void obtainItem(const QString& itemName);
    void showCG(const QString& cgName);
    void backToHall();
    void triggerEnding(EndingType type);
    void showPlayer();
    void updateDialogPosition();

    void runSpring();
    void runSummer();
    void runAutumn();
    void runWinter();

    LevelID m_level;
    int m_index;
    bool m_inChoice;
    bool m_inCG;

    bool m_aPre, m_bPre, m_dPre;
    bool m_hide;
    EndingType m_pendingEnding;
    QString m_npcState;

    QSet<QString> m_items;

    SkinID m_playerSkinId;   // ← 新增：记录当前关卡使用的皮肤

    DialogBox* m_dialog;
    ItemMiniGame* m_itemGame;
    bool m_inMiniGame;
    int m_postMiniGameIndex;
    ChoiceMenu* m_choice;
    ItemPopup* m_popup;
    CGPlayer* m_cg;
};

#endif // LEVELSCENE_H
