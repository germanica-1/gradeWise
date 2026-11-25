#pragma once
#include <QWidget>
#include "../core/AppData.h"

class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;

class SubjectsPage : public QWidget {
    Q_OBJECT
public:
    explicit SubjectsPage(AppData& data, QWidget* parent = nullptr);
    void refresh();

signals:
    void dataChanged();

private:
    void onSaveClicked();   // adds a new subject, or saves the one being edited
    void onEditClicked();
    void onDeleteClicked();
    void clearForm();
    void showError(const QString& message);
    int selectedRow() const;

    AppData& data;
    int editingIndex = -1; // -1 means the form adds a new subject

    QLineEdit* nameInput;
    QDoubleSpinBox* gradeInput;
    QSpinBox* unitsInput;
    QPushButton* saveButton;
    QPushButton* cancelButton;
    QPushButton* editButton;
    QPushButton* deleteButton;
    QLabel* errorLabel;
    QTableWidget* table;
    QLabel* totalsLabel;
};
