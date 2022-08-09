#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateEdit>
#include <QComboBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    enum TabTypes {
        TabCalories = 0,
        TabSteps = 1,
        TabDistance = 2,
        TabActiveTime = 3
    };

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private Q_SLOTS:
    void on_actionImport_Activities_triggered();
    void reloadAll();

private:
    Ui::MainWindow *ui;
    QDateEdit *mStartDate;
    QDateEdit *mEndDate;
    QComboBox *mDuration;
    bool mBlockReloading = false;
};
#endif // MAINWINDOW_H
