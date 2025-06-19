#pragma once

#include <QMenu>

class PopupMenu : public QMenu {
    Q_OBJECT
public:
    /**
     * @brief
     * @param calling_widget This is preserved when someday i want the popup more fancy
     * @param parent
     */
    explicit PopupMenu(QWidget* calling_widget, QWidget* parent = nullptr);
    void showEvent(QShowEvent* event) override;

private:
    QWidget* calling_widget;
};
