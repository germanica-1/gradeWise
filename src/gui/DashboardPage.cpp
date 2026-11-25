#include "DashboardPage.h"
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <cmath>
#include "../core/Calculator.h"
#include "../core/Validation.h"
#include "FeedbackPage.h"

static QString statusColor(GoalStatus status) {
    switch (status) {
    case GoalStatus::Achieved: return "#2E9E5B";
    case GoalStatus::NotYet:   return "#E0A100";
    case GoalStatus::AtRisk:   return "#D64545";
    default:                   return "#7A869A";
    }
}

DashboardPage::DashboardPage(AppData& appData, QWidget* parent) : QWidget(parent), data(appData) {
    QLabel* title = new QLabel("Dashboard");
    title->setObjectName("pageTitle");

    QGridLayout* cards = new QGridLayout;
    cards->setSpacing(16);
    cards->addWidget(makeCard("Current GWA", gwaValue), 0, 0);
    cards->addWidget(makeCard("Target GWA", targetValue), 0, 1);
    cards->addWidget(makeCard("Subjects", subjectsValue), 0, 2);
    cards->addWidget(makeCard("Total Units", unitsValue), 0, 3);

    statusBanner = new QWidget;
    statusBanner->setObjectName("statusBanner");
    statusBanner->setAttribute(Qt::WA_StyledBackground);
    statusTitle = new QLabel;
    statusTitle->setObjectName("statusTitle");
    statusDetail = new QLabel;
    statusDetail->setObjectName("statusDetail");
    statusDetail->setWordWrap(true);
    QVBoxLayout* bannerLayout = new QVBoxLayout(statusBanner);
    bannerLayout->setContentsMargins(24, 20, 24, 20);
    bannerLayout->addWidget(statusTitle);
    bannerLayout->addWidget(statusDetail);

    addFirstButton = new QPushButton("Add your first subject  →");
    addFirstButton->setObjectName("primaryButton");
    addFirstButton->setCursor(Qt::PointingHandCursor);
    connect(addFirstButton, &QPushButton::clicked, this, [this]() { emit goToPage(1); });

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(32, 28, 32, 28);
    layout->setSpacing(20);
    layout->addWidget(title);
    layout->addLayout(cards);
    layout->addWidget(statusBanner);
    layout->addWidget(addFirstButton, 0, Qt::AlignLeft);

    feedback = new FeedbackPage(data);
    layout->addWidget(feedback, 1);
    connect(feedback, &FeedbackPage::feedbackSaved, this, &DashboardPage::feedbackSaved);
}

QWidget* DashboardPage::makeCard(const QString& title, QLabel*& valueLabel) {
    QWidget* card = new QWidget;
    card->setObjectName("card");
    card->setAttribute(Qt::WA_StyledBackground);
    QLabel* titleLabel = new QLabel(title);
    titleLabel->setObjectName("cardTitle");
    valueLabel = new QLabel;
    valueLabel->setObjectName("cardValue");
    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);
    return card;
}

void DashboardPage::refresh() {
    int units = getTotalUnits(data.subjects);
    double gwa = calculateGWA(data.subjects);
    GoalStatus status = evaluateGoal(gwa, units, data.target, data.scale);

    gwaValue->setText(units == 0 ? "—" : QString::fromStdString(formatGrade(gwa)));
    targetValue->setText(QString::fromStdString(formatGrade(data.target)));
    subjectsValue->setText(QString::number(data.subjects.size()));
    unitsValue->setText(QString::number(units));

    statusTitle->setText(QString::fromStdString(goalStatusText(status)));
    statusBanner->setStyleSheet("#statusBanner { background: " + statusColor(status) + "; }");
    addFirstButton->setVisible(data.subjects.empty());

    QString target = QString::fromStdString(formatGrade(data.target));
    QString gap = QString::fromStdString(formatGrade(std::fabs(gwa - data.target)));
    QString detail;
    if (status == GoalStatus::NoData) {
        detail = "Add your subjects to see how close you are to your target GWA of " + target + ".";
    } else if (status == GoalStatus::Achieved) {
        detail = "Your GWA meets your target of " + target + ". Keep it up!";
    } else {
        detail = "You are " + gap + " away from your target of " + target + ".";
        std::vector<int> ranking = rankByImpact(data.subjects, data.scale);
        detail += " Biggest impact: " + QString::fromStdString(data.subjects[ranking[0]].name) + ".";
    }
    statusDetail->setText(detail);
    feedback->refresh();
}
