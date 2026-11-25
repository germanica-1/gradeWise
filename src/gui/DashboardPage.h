#pragma once
#include <QWidget>
#include "../core/AppData.h"

class QLabel;
class QPushButton;
class FeedbackPage;

class DashboardPage : public QWidget {
    Q_OBJECT
public:
    explicit DashboardPage(AppData& data, QWidget* parent = nullptr);
    void refresh();

signals:
    void goToPage(int index);
    void feedbackSaved();

private:
    QWidget* makeCard(const QString& title, QLabel*& valueLabel);

    AppData& data;
    FeedbackPage* feedback;
    QLabel* gwaValue;
    QLabel* targetValue;
    QLabel* subjectsValue;
    QLabel* unitsValue;
    QLabel* statusTitle;
    QLabel* statusDetail;
    QWidget* statusBanner;
    QPushButton* addFirstButton;
};
