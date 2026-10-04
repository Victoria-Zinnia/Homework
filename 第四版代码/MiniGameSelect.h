#ifndef MINIGAMESELECT_H
#define MINIGAMESELECT_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QSet>

struct MiniGameLevel {
    QString itemName;    // 道具文件名（assets/images/items/ 下的名字，不带 .png）
    QString displayName; // 关卡显示名
    QString ruleText;    // 规则说明
};

class MiniGameSelect : public QWidget
{
    Q_OBJECT
public:
    explicit MiniGameSelect(QWidget* parent = nullptr);
    void startSelect(const QString& seasonTitle, const QList<MiniGameLevel>& levels);
    void setCompletedItems(const QSet<QString>& items);
    void setBackgroundName(const QString& name);  // 新增：设置季节背景图名

signals:
    void levelStarted(const QString& itemName); // 玩家点击"开始挑战"
    void selectCancelled();                     // ESC 返回

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void showEvent(QShowEvent*) override;

private:
    void drawSelectPage(QPainter& p);
    void drawRulePage(QPainter& p);
    QRect cardRect(int index) const;
    int cardAt(const QPoint& pos) const;
    QRect startBtnRect() const;

    enum Page { Page_Select, Page_Rule };
    Page m_page;
    QString m_seasonTitle;
    QList<MiniGameLevel> m_levels;
    int m_selectedIndex;
    int m_hoverCard;
    bool m_hoverStart;
    QSet<QString> m_completedItems;
    QString m_bgName;  // 新增：背景图名称
};

#endif