#include "SubjectsPage.h"
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>
#include "../core/Calculator.h"
#include "../core/SubjectManager.h"
#include "../core/Validation.h"

static QString fmt(double value) { return QString::fromStdString(formatGrade(value)); }

SubjectsPage::SubjectsPage(AppData& appData, QWidget* parent) : QWidget(parent), data(appData) {
    QLabel* title = new QLabel("Subjects");
    title->setObjectName("pageTitle");

    nameInput = new QLineEdit;
    gradeInput = new QDoubleSpinBox;
    gradeInput->setDecimals(2);
    gradeInput->setSingleStep(0.25);
    unitsInput = new QSpinBox;
    unitsInput->setRange(1, 10);
    unitsInput->setValue(3);
    saveButton = new QPushButton("Add Subject");
    saveButton->setObjectName("primaryButton");
    cancelButton = new QPushButton("Cancel");
    cancelButton->hide();

    QHBoxLayout* form = new QHBoxLayout;
    form->addWidget(new QLabel("Name"));
    form->addWidget(nameInput, 3);
    form->addWidget(new QLabel("Grade"));
    form->addWidget(gradeInput, 1);
    form->addWidget(new QLabel("Units"));
    form->addWidget(unitsInput, 1);
    form->addWidget(saveButton);
    form->addWidget(cancelButton);

    QWidget* formCard = new QWidget;
    formCard->setObjectName("card");
    formCard->setAttribute(Qt::WA_StyledBackground);
    QVBoxLayout* formCardLayout = new QVBoxLayout(formCard);
    formCardLayout->setContentsMargins(20, 16, 20, 16);
    formCardLayout->addLayout(form);
    errorLabel = new QLabel;
    errorLabel->setObjectName("errorText");
    errorLabel->hide();
    formCardLayout->addWidget(errorLabel);

    table = new QTableWidget(0, 5);
    table->setHorizontalHeaderLabels({"Subject", "Grade", "Units", "Weighted", "Remark"});
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    table->verticalHeader()->hide();
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);

    editButton = new QPushButton("Edit Selected");
    deleteButton = new QPushButton("Delete Selected");
    deleteButton->setObjectName("dangerButton");
    totalsLabel = new QLabel;
    totalsLabel->setObjectName("totalsText");
    QHBoxLayout* footer = new QHBoxLayout;
    footer->addWidget(editButton);
    footer->addWidget(deleteButton);
    footer->addStretch();
    footer->addWidget(totalsLabel);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(32, 28, 32, 28);
    layout->setSpacing(16);
    layout->addWidget(title);
    layout->addWidget(formCard);
    layout->addWidget(table, 1);
    layout->addLayout(footer);

    connect(saveButton, &QPushButton::clicked, this, &SubjectsPage::onSaveClicked);
    connect(nameInput, &QLineEdit::returnPressed, this, &SubjectsPage::onSaveClicked);
    connect(cancelButton, &QPushButton::clicked, this, &SubjectsPage::clearForm);
    connect(editButton, &QPushButton::clicked, this, &SubjectsPage::onEditClicked);
    connect(deleteButton, &QPushButton::clicked, this, &SubjectsPage::onDeleteClicked);
    connect(table, &QTableWidget::cellDoubleClicked, this, &SubjectsPage::onEditClicked);
    connect(table, &QTableWidget::itemSelectionChanged, this, [this]() {
        bool hasSelection = selectedRow() != -1;
        editButton->setEnabled(hasSelection);
        deleteButton->setEnabled(hasSelection);
    });
}

int SubjectsPage::selectedRow() const {
    QList<QTableWidgetItem*> items = table->selectedItems();
    return items.isEmpty() ? -1 : items.first()->row();
}

void SubjectsPage::refresh() {
    const GradingScale& scale = data.scale;
    gradeInput->setRange(scale.minGrade, scale.maxGrade);
    nameInput->setPlaceholderText(QString::fromStdString(nextDefaultName(data.subjects)) + " (leave blank to use)");

    table->setRowCount(static_cast<int>(data.subjects.size()));
    for (int row = 0; row < static_cast<int>(data.subjects.size()); row++) {
        const Subject& s = data.subjects[row];
        std::string unused;
        bool inRange = validateGrade(s.grade, scale, unused);

        QString remark;
        if (!inRange) {
            remark = "Outside scale!";
        } else if (isPassing(s.grade, scale)) {
            remark = "Passed (" + QString::fromStdString(performanceCategory(s.grade, scale)) + ")";
        } else {
            remark = "Failed";
        }

        QStringList cells = {QString::fromStdString(s.name), fmt(s.grade), QString::number(s.units),
                             fmt(weightedGrade(s)), remark};
        for (int col = 0; col < cells.size(); col++) {
            QTableWidgetItem* item = new QTableWidgetItem(cells[col]);
            if (col > 0) {
                item->setTextAlignment(Qt::AlignCenter);
            }
            if (!inRange || (col == 4 && !isPassing(s.grade, scale))) {
                item->setForeground(QColor("#D64545"));
            }
            table->setItem(row, col, item);
        }
    }

    int units = getTotalUnits(data.subjects);
    QString gwaText = units == 0 ? "—" : fmt(calculateGWA(data.subjects));
    totalsLabel->setText("Total units: " + QString::number(units) +
                         "    Total weighted: " + fmt(getTotalWeighted(data.subjects)) +
                         "    GWA: " + gwaText);

    bool hasSelection = selectedRow() != -1;
    editButton->setEnabled(hasSelection);
    deleteButton->setEnabled(hasSelection);
}

void SubjectsPage::onSaveClicked() {
    Subject subject;
    subject.name = nameInput->text().toStdString();
    subject.grade = gradeInput->value();
    subject.units = unitsInput->value();
    if (trim(subject.name).empty() && editingIndex == -1) {
        subject.name = nextDefaultName(data.subjects);
    }

    std::string error;
    bool ok;
    if (editingIndex == -1) {
        ok = addSubject(data.subjects, subject, data.scale, error);
    } else {
        ok = editSubject(data.subjects, editingIndex, subject, data.scale, error);
    }

    if (!ok) {
        showError(QString::fromStdString(error));
        return;
    }
    clearForm();
    emit dataChanged();
}

void SubjectsPage::onEditClicked() {
    int row = selectedRow();
    if (row == -1) {
        return;
    }
    const Subject& s = data.subjects[row];
    editingIndex = row;
    nameInput->setText(QString::fromStdString(s.name));
    gradeInput->setValue(s.grade);
    unitsInput->setValue(s.units);
    saveButton->setText("Save Changes");
    cancelButton->show();
    errorLabel->hide();
    nameInput->setFocus();
}

void SubjectsPage::onDeleteClicked() {
    int row = selectedRow();
    if (row == -1) {
        return;
    }
    QString name = QString::fromStdString(data.subjects[row].name);
    if (QMessageBox::question(this, "Delete subject", "Delete \"" + name + "\"?") != QMessageBox::Yes) {
        return;
    }
    deleteSubject(data.subjects, row);
    clearForm(); // the edited row index may no longer be valid
    emit dataChanged();
}

void SubjectsPage::clearForm() {
    editingIndex = -1;
    nameInput->clear();
    unitsInput->setValue(3);
    saveButton->setText("Add Subject");
    cancelButton->hide();
    errorLabel->hide();
}

void SubjectsPage::showError(const QString& message) {
    errorLabel->setText(message);
    errorLabel->show();
}
