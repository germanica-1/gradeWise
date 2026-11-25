#include "MainWindow.h"
#include <QHBoxLayout>
#include <QListWidget>
#include <QMessageBox>
#include <QStackedWidget>
#include "../storage/Storage.h"
#include "DashboardPage.h"
#include "SettingsPage.h"
#include "SubjectsPage.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("GradeWise — GWA Calculator & AI Study Advisor");
    resize(1100, 780);

    dataPath = defaultDataPath();
    if (!loadRecords(data, dataPath)) {
        QMessageBox::warning(this, "Data file problem",
                             "Your saved data could not be read. It was kept as data.json.bak "
                             "and the app started with empty data.");
    }

    sidebar = new QListWidget;
    sidebar->setObjectName("sidebar");
    sidebar->addItems({"Dashboard", "Subjects", "Settings"});
    sidebar->setFixedWidth(200);

    dashboardPage = new DashboardPage(data); // also contains the AI feedback panel
    subjectsPage = new SubjectsPage(data);
    settingsPage = new SettingsPage(data);

    pages = new QStackedWidget;
    pages->addWidget(dashboardPage);
    pages->addWidget(subjectsPage);
    pages->addWidget(settingsPage);

    QWidget* central = new QWidget;
    QHBoxLayout* layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(sidebar);
    layout->addWidget(pages, 1);
    setCentralWidget(central);

    connect(sidebar, &QListWidget::currentRowChanged, pages, &QStackedWidget::setCurrentIndex);
    connect(dashboardPage, &DashboardPage::goToPage, sidebar, [this](int row) { sidebar->setCurrentRow(row); });
    connect(subjectsPage, &SubjectsPage::dataChanged, this, &MainWindow::onDataChanged);
    connect(settingsPage, &SettingsPage::dataChanged, this, &MainWindow::onDataChanged);
    connect(dashboardPage, &DashboardPage::feedbackSaved, this, &MainWindow::save);

    sidebar->setCurrentRow(0);
    refreshAll();
}

void MainWindow::refreshAll() {
    dashboardPage->refresh();
    subjectsPage->refresh();
    settingsPage->refresh();
}

void MainWindow::onDataChanged() {
    if (!data.lastFeedback.empty()) {
        data.feedbackStale = true;
    }
    refreshAll();
    save();
}

void MainWindow::save() {
    if (!saveRecords(data, dataPath)) {
        QMessageBox::warning(this, "Save failed", "Could not save your data to:\n" + dataPath);
    }
}
