#pragma once
#include <QMainWindow>
#include "../core/AppData.h"

class QListWidget;
class QStackedWidget;
class DashboardPage;
class SubjectsPage;
class SettingsPage;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void refreshAll();   // redraw every page from `data`
    void onDataChanged(); // grades or settings changed: refresh, mark AI feedback old, save
    void save();

    AppData data;        // the single source of truth
    QString dataPath;

    QListWidget* sidebar;
    QStackedWidget* pages;
    DashboardPage* dashboardPage;
    SubjectsPage* subjectsPage;
    SettingsPage* settingsPage;
};
