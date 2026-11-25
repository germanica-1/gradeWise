#pragma once
#include <QWidget>
#include "../ai/GeminiClient.h"
#include "../core/AppData.h"

class QComboBox;
class QDoubleSpinBox;
class QFormLayout;
class QLabel;
class QLineEdit;
class QPushButton;

class SettingsPage : public QWidget {
    Q_OBJECT
public:
    explicit SettingsPage(AppData& data, QWidget* parent = nullptr);
    void refresh(); // copy saved values into the form

signals:
    void dataChanged();

private:
    void onPresetChanged(int index);
    void onSaveClicked();
    void onTestClicked();
    void showMessage(const QString& text, const QString& kind); // kind: successText / errorText / warningText / mutedText
    void fillScale(const GradingScale& scale);
    GradingScale scaleFromForm() const;
    void addRow(QFormLayout* form, const QString& label, QWidget* field, const QString& tip);

    AppData& data;
    GeminiClient client;
    bool filling = false; // true while refresh() fills the form, so preset changes don't overwrite it

    QDoubleSpinBox* targetInput;
    QComboBox* presetInput;
    QDoubleSpinBox* bestInput;
    QDoubleSpinBox* worstInput;
    QDoubleSpinBox* passingInput;
    QDoubleSpinBox* marginInput;
    QLabel* directionLabel;
    QLineEdit* apiKeyInput;
    QLineEdit* modelInput;
    QLabel* keySourceLabel;
    QPushButton* testButton;
    QLabel* messageLabel;
};
