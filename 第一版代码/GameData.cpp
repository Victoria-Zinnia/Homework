#include "GameData.h"
#include <QSettings>
#include <QDir>
#include <QDebug>

GameData* GameData::m_instance = nullptr;

GameData* GameData::instance()
{
    if (!m_instance)
        m_instance = new GameData();
    return m_instance;
}

GameData::GameData(QObject* parent) : QObject(parent)
{
    m_currentSkin = SKIN_DEFAULT;
    m_anyCorrupt = false;
    m_extraUnlocked = false;
    m_newGamePlus = 0;

    // 初始化皮肤数据
    m_skins[SKIN_DEFAULT] = { SKIN_DEFAULT, "陆令晞·本元", "主角_陆令晞本元.png", true, LEVEL_NONE, false };
    m_skins[SKIN_SPRING_NORMAL] = { SKIN_SPRING_NORMAL, "陆昭芃·春栖", "陆昭芃_春栖.png", false, LEVEL_SPRING, false };
    m_skins[SKIN_SPRING_RARE] = { SKIN_SPRING_RARE, "陆昭芃·无根", "陆昭芃_无根.png", false, LEVEL_SPRING, true };
    m_skins[SKIN_SUMMER_NORMAL] = { SKIN_SUMMER_NORMAL, "陆安泫·观澜", "陆安泫_观澜.png", false, LEVEL_SUMMER, false };
    m_skins[SKIN_SUMMER_RARE] = { SKIN_SUMMER_RARE, "陆安泫·羁澜", "陆安泫_羁澜.png", false, LEVEL_SUMMER, true };
    m_skins[SKIN_AUTUMN_NORMAL] = { SKIN_AUTUMN_NORMAL, "陆祐禾·秋衡", "陆祐禾_秋衡.png", false, LEVEL_AUTUMN, false };
    m_skins[SKIN_AUTUMN_RARE] = { SKIN_AUTUMN_RARE, "陆祐禾·空蘅", "陆祐禾_空蘅.png", false, LEVEL_AUTUMN, true };
    m_skins[SKIN_WINTER_NORMAL] = { SKIN_WINTER_NORMAL, "陆融霏·归晞", "陆融霏_归晞.png", false, LEVEL_WINTER, false };
    m_skins[SKIN_WINTER_RARE] = { SKIN_WINTER_RARE, "陆融霏·寒绾", "陆融霏_寒绾.png", false, LEVEL_WINTER, true };

    for (int i = 0; i < LEVEL_COUNT; ++i) {
        LevelID lid = static_cast<LevelID>(i);
        m_levelEndings[lid] = ENDING_NONE;
        m_dFlags[lid] = false;
        m_hideFlags[lid] = false;
        m_dPreFlags[lid] = false;
    }
}

void GameData::setCurrentSkin(SkinID id)
{
    if (m_skins.contains(id) && m_skins[id].unlocked)
        m_currentSkin = id;
}

SkinID GameData::currentSkin() const { return m_currentSkin; }

SkinInfo GameData::getSkinInfo(SkinID id) const
{
    if (m_skins.contains(id)) return m_skins[id];
    return SkinInfo();
}

bool GameData::isSkinUnlocked(SkinID id) const
{
    if (m_skins.contains(id)) return m_skins[id].unlocked;
    return false;
}

void GameData::unlockSkin(SkinID id)
{
    if (m_skins.contains(id)) {
        m_skins[id].unlocked = true;
        emit skinUnlocked(id);
    }
}

bool GameData::isSpringSkin() const {
    return m_currentSkin == SKIN_SPRING_NORMAL || m_currentSkin == SKIN_SPRING_RARE;
}
bool GameData::isSummerSkin() const {
    return m_currentSkin == SKIN_SUMMER_NORMAL || m_currentSkin == SKIN_SUMMER_RARE;
}
bool GameData::isAutumnSkin() const {
    return m_currentSkin == SKIN_AUTUMN_NORMAL || m_currentSkin == SKIN_AUTUMN_RARE;
}
bool GameData::isWinterSkin() const {
    return m_currentSkin == SKIN_WINTER_NORMAL || m_currentSkin == SKIN_WINTER_RARE;
}
bool GameData::isDefaultSkin() const { return m_currentSkin == SKIN_DEFAULT; }
bool GameData::isRareSkin() const {
    return m_currentSkin == SKIN_SPRING_RARE || m_currentSkin == SKIN_SUMMER_RARE
        || m_currentSkin == SKIN_AUTUMN_RARE || m_currentSkin == SKIN_WINTER_RARE;
}

void GameData::setLevelEnding(LevelID level, EndingType ending) {
    m_levelEndings[level] = ending;
}
EndingType GameData::getLevelEnding(LevelID level) const {
    return m_levelEndings.value(level, ENDING_NONE);
}
bool GameData::isLevelCompleted(LevelID level) const {
    return m_levelEndings[level] != ENDING_NONE;
}

void GameData::setDFlag(LevelID level, bool value) { m_dFlags[level] = value; }
bool GameData::getDFlag(LevelID level) const { return m_dFlags.value(level, false); }

void GameData::setHideFlag(LevelID level, bool value) { m_hideFlags[level] = value; }
bool GameData::getHideFlag(LevelID level) const { return m_hideFlags.value(level, false); }

void GameData::setDPreFlag(LevelID level, bool value) { m_dPreFlags[level] = value; }
bool GameData::getDPreFlag(LevelID level) const { return m_dPreFlags.value(level, false); }

bool GameData::anyCorrupt() const { return m_anyCorrupt; }
void GameData::setAnyCorrupt(bool value) { m_anyCorrupt = value; }

bool GameData::allAEnding() const {
    for (int i = 0; i < LEVEL_COUNT; ++i) {
        if (m_levelEndings[static_cast<LevelID>(i)] != ENDING_A_PERFECT)
            return false;
    }
    return true;
}

bool GameData::allDFlag() const {
    for (int i = 0; i < LEVEL_COUNT; ++i) {
        if (!m_dFlags[static_cast<LevelID>(i)])
            return false;
    }
    return true;
}

bool GameData::allLevelsCompleted() const {
    for (int i = 0; i < LEVEL_COUNT; ++i) {
        if (m_levelEndings[static_cast<LevelID>(i)] == ENDING_NONE)
            return false;
    }
    return true;
}

bool GameData::isExtraUnlocked() const { return m_extraUnlocked; }
void GameData::setExtraUnlocked(bool value) { m_extraUnlocked = value; }

int GameData::getNewGamePlus() const { return m_newGamePlus; }
void GameData::setNewGamePlus(int count) { m_newGamePlus = count; }

void GameData::resetCurrentRun()
{
    m_currentSkin = SKIN_DEFAULT;
    for (int i = 0; i < LEVEL_COUNT; ++i) {
        LevelID lid = static_cast<LevelID>(i);
        m_levelEndings[lid] = ENDING_NONE;
        m_hideFlags[lid] = false;
        m_dPreFlags[lid] = false;
    }
    m_anyCorrupt = false;
}

void GameData::fullReset()
{
    resetCurrentRun();
    m_extraUnlocked = false;
    m_newGamePlus = 0;
    for (auto it = m_skins.begin(); it != m_skins.end(); ++it) {
        if (it.key() != SKIN_DEFAULT)
            it.value().unlocked = false;
    }
    for (int i = 0; i < LEVEL_COUNT; ++i) {
        m_dFlags[static_cast<LevelID>(i)] = false;
    }
}

QString GameData::saveFilePath() const
{
    QDir dir("save");
    if (!dir.exists()) dir.mkpath(".");
    return "save/savegame.ini";
}

void GameData::saveToFile()
{
    QSettings settings(saveFilePath(), QSettings::IniFormat);
    settings.setValue("currentSkin", m_currentSkin);
    settings.setValue("anyCorrupt", m_anyCorrupt);
    settings.setValue("extraUnlocked", m_extraUnlocked);
    settings.setValue("newGamePlus", m_newGamePlus);

    for (int i = 0; i < LEVEL_COUNT; ++i) {
        LevelID lid = static_cast<LevelID>(i);
        settings.setValue(QString("levelEnding_%1").arg(i), m_levelEndings[lid]);
        settings.setValue(QString("dFlag_%1").arg(i), m_dFlags[lid]);
    }

    for (auto it = m_skins.begin(); it != m_skins.end(); ++it) {
        settings.setValue(QString("skinUnlocked_%1").arg(it.key()), it.value().unlocked);
    }
}

void GameData::loadFromFile()
{
    QString path = saveFilePath();
    if (!QFile::exists(path)) return;

    QSettings settings(path, QSettings::IniFormat);
    m_currentSkin = static_cast<SkinID>(settings.value("currentSkin", 0).toInt());
    m_anyCorrupt = settings.value("anyCorrupt", false).toBool();
    m_extraUnlocked = settings.value("extraUnlocked", false).toBool();
    m_newGamePlus = settings.value("newGamePlus", 0).toInt();

    for (int i = 0; i < LEVEL_COUNT; ++i) {
        LevelID lid = static_cast<LevelID>(i);
        m_levelEndings[lid] = static_cast<EndingType>(settings.value(QString("levelEnding_%1").arg(i), 0).toInt());
        m_dFlags[lid] = settings.value(QString("dFlag_%1").arg(i), false).toBool();
    }

    for (auto it = m_skins.begin(); it != m_skins.end(); ++it) {
        it.value().unlocked = settings.value(QString("skinUnlocked_%1").arg(it.key()), it.key() == SKIN_DEFAULT).toBool();
    }
}