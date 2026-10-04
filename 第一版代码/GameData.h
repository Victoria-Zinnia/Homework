#ifndef GAMEDATA_H
#define GAMEDATA_H

#include <QObject>
#include <QMap>
#include <QString>

// 皮肤ID枚举
enum SkinID {
    SKIN_DEFAULT = 0,       // 陆令晞·本元
    SKIN_SPRING_NORMAL,     // 陆昭芃·春栖
    SKIN_SPRING_RARE,       // 陆昭芃·无根
    SKIN_SUMMER_NORMAL,     // 陆安泫·观澜
    SKIN_SUMMER_RARE,       // 陆安泫·羁澜
    SKIN_AUTUMN_NORMAL,     // 陆祐禾·秋衡
    SKIN_AUTUMN_RARE,       // 陆祐禾·空蘅
    SKIN_WINTER_NORMAL,     // 陆融霏·归晞
    SKIN_WINTER_RARE,       // 陆融霏·寒绾
    SKIN_COUNT
};

// 关卡ID
enum LevelID {
    LEVEL_NONE = -1,
    LEVEL_SPRING = 0,
    LEVEL_SUMMER,
    LEVEL_AUTUMN,
    LEVEL_WINTER,
    LEVEL_COUNT
};

// 结局类型
enum EndingType {
    ENDING_NONE = 0,
    ENDING_A_PERFECT,       // A完美救赎
    ENDING_B_NEUTRAL,       // B中立遗憾
    ENDING_C_CORRUPT,       // C崩坏失败
    ENDING_D_BONUS          // D彩蛋回忆
};

// 终章结局
enum FinalEnding {
    FINAL_NONE = 0,
    FINAL_BROKEN,           // 轮回破碎
    FINAL_DETAINED,         // 轮回羁留
    FINAL_RECONCILED,       // 四时和解
    FINAL_TRUE              // 真结局·残书补全
};

// 皮肤信息
struct SkinInfo {
    SkinID id;
    QString name;           // 显示名称
    QString imageFile;      // 立绘文件名
    bool unlocked;          // 是否解锁
    LevelID relatedLevel;   // 关联关卡
    bool isRare;            // 是否典藏
};

class GameData : public QObject
{
    Q_OBJECT
public:
    explicit GameData(QObject* parent = nullptr);
    static GameData* instance();

    // 皮肤相关
    void setCurrentSkin(SkinID id);
    SkinID currentSkin() const;
    SkinInfo getSkinInfo(SkinID id) const;
    bool isSkinUnlocked(SkinID id) const;
    void unlockSkin(SkinID id);

    // 皮肤类型判断
    bool isSpringSkin() const;
    bool isSummerSkin() const;
    bool isAutumnSkin() const;
    bool isWinterSkin() const;
    bool isDefaultSkin() const;
    bool isRareSkin() const;

    // 关卡结局
    void setLevelEnding(LevelID level, EndingType ending);
    EndingType getLevelEnding(LevelID level) const;
    bool isLevelCompleted(LevelID level) const;

    // D彩蛋标记（持久化）
    void setDFlag(LevelID level, bool value);
    bool getDFlag(LevelID level) const;

    // 超级隐藏标记（本局临时，不存档）
    void setHideFlag(LevelID level, bool value);
    bool getHideFlag(LevelID level) const;

    // D前置标记（本局关卡内临时，不存档）
    void setDPreFlag(LevelID level, bool value);
    bool getDPreFlag(LevelID level) const;

    // 全局标记
    bool anyCorrupt() const;
    void setAnyCorrupt(bool value);
    bool allAEnding() const;
    bool allDFlag() const;
    bool allLevelsCompleted() const;

    // 番外解锁
    bool isExtraUnlocked() const;
    void setExtraUnlocked(bool value);

    // 新周目
    int getNewGamePlus() const;
    void setNewGamePlus(int count);

    // 重置
    void resetCurrentRun();
    void fullReset();

    // 存档路径：save/savegame.ini（相对工作目录）
    void saveToFile();
    void loadFromFile();
    QString saveFilePath() const;

signals:
    void skinUnlocked(SkinID id);

private:
    static GameData* m_instance;

    SkinID m_currentSkin;
    QMap<SkinID, SkinInfo> m_skins;
    QMap<LevelID, EndingType> m_levelEndings;
    QMap<LevelID, bool> m_dFlags;
    QMap<LevelID, bool> m_hideFlags;    // 不存档
    QMap<LevelID, bool> m_dPreFlags;    // 不存档
    bool m_anyCorrupt;
    bool m_extraUnlocked;
    int m_newGamePlus;
};

#endif // GAMEDATA_H
