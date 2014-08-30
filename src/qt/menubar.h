#ifndef MENUBAR_H
#define MENUBAR_H

#include <QFrame>

namespace Ui {
    class MenuBar;
}

/** A skinnable menu bar to be shown at the right of the main frame tab bar. */
class MenuBar : public QFrame
{
    Q_OBJECT

public:
    explicit MenuBar(QWidget *parent = 0);
    ~MenuBar();

public slots:

signals:
    void showFileMenu(QPoint);
    void showSettingsMenu(QPoint);
    void showHelpMenu(QPoint);

private slots:
    void on_FileButton_clicked();
    void on_SettingsButton_clicked();
    void on_HelpButton_clicked();

private:
	Ui::MenuBar* ui;
};

#endif
