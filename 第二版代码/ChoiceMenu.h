#ifndef CHOICEMENU_H
#define CHOICEMENU_H

#include <QWidget>
#include <QList>
#include <QString>

class QPushButton;

class ChoiceMenu : public QWidget
{
    Q_OBJECT
public:
    explicit ChoiceMenu(QWidget* parent = nullptr);

    void setChoices(const QList<QString>& choices);
    void clear();
    void showChoices();
    void hideChoices();

signals:
    void choiceSelected(int index);   // 0-based

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QList<QPushButton*> m_buttons;
    void createButton(int index, const QString& text);
};

#endif // CHOICEMENU_H
