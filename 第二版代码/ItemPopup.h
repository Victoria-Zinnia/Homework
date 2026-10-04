#ifndef ITEMPOPUP_H
#define ITEMPOPUP_H

#include <QWidget>
#include <QLabel>
#include <QTimer>

class ItemPopup : public QWidget
{
    Q_OBJECT
public:
    explicit ItemPopup(QWidget* parent = nullptr);
    void showItem(const QString& itemName, const QString& desc = "");

signals:
    void popupFinished();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QLabel* m_iconLabel;
    QLabel* m_nameLabel;
    QLabel* m_descLabel;
    QTimer* m_hideTimer;
};

#endif // ITEMPOPUP_H
