#ifndef POPUPMENU_H
#define POPUPMENU_H

#include <QMenu>

class popupMenu : public QMenu {
    Q_OBJECT
public:
    /**
     * @brief
     * @param calling_widget This is preserved when someday i want the popup more fancy
     * @param parent
     */
    explicit popupMenu(QWidget* calling_widget, QWidget* parent = nullptr);
    void showEvent(QShowEvent* event) override;

private:
    QWidget* calling_widget;
};

#endif // POPUPMENU_H
