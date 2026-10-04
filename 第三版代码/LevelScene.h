#ifndef LEVELSCENE_H
#define LEVELSCENE_H

#include "ItemMiniGame.h"
#include "SceneBase.h"
#include "GameData.h"
#include "DialogBox.h"
#include "ChoiceMenu.h"
#include "ItemPopup.h"
#include "CGPlayer.h"
#include "KnotUntangleGame.h"
#include "MiniGameSelect.h"
#include <QSet>

class MirrorPetalGame;
class MortisePuzzleGame;
class ScrollRestoreGame;
class ShadowPuppetGame;
class PorcelainRestoreGame;
class GardenPathGame;

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
    void onKnotGameFinished(const QString& item);
    void onKnotGameSkipped(const QString& item);

    void onMirrorGameFinished(const QString& item);
    void onMirrorGameSkipped(const QString& item);
    void onMortiseGameFinished(const QString& item);
    void onMortiseGameSkipped(const QString& item);
    void onScrollGameFinished(const QString& item);
    void onScrollGameSkipped(const QString& item);
    void onShadowGameFinished(const QString& item);
    void onShadowGameSkipped(const QString& item);
    void onPorcelainGameFinished(const QString& item);
    void onPorcelainGameSkipped(const QString& item);
    void onGardenGameFinished(const QString& item);
    void onGardenGameSkipped(const QString& item);

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

    void showLevelSelect();           // 新增：显示季节总关卡选择页
    void handleMiniGameDone(const QString& itemName);  // 新增：统一处理小游戏完成

    LevelID m_level;
    int m_index;
    bool m_inChoice;
    bool m_inCG;
    bool m_aPre, m_bPre, m_dPre;
    bool m_hide;
    EndingType m_pendingEnding;
    QString m_npcState;
    QSet<QString> m_items;
    SkinID m_playerSkinId;

    DialogBox* m_dialog;
    ItemMiniGame* m_itemGame;
    bool m_inMiniGame;
    int m_postMiniGameIndex;
    ChoiceMenu* m_choice;
    ItemPopup* m_popup;
    CGPlayer* m_cg;
    KnotUntangleGame* m_knotGame;
    MiniGameSelect* m_gameSelect = nullptr;
    QString m_pendingItem;

    MirrorPetalGame* m_mirrorGame;
    MortisePuzzleGame* m_mortiseGame;
    ScrollRestoreGame* m_scrollGame;
    ShadowPuppetGame* m_shadowGame;
    PorcelainRestoreGame* m_porcelainGame;
    GardenPathGame* m_gardenGame;
    bool m_inAnyMiniGame;
};

#endif // LEVELSCENE_H

