#include "SettingsPage.h"
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QStyle>
#include <QVBoxLayout>
#include <algorithm>
#include "../core/Validation.h"

static QDoubleSpinBox* makeGradeBox() {
    QDoubleSpinBox* box = new QDoubleSpinBox;
    box->setDecimals(2);
    box->setRange(-1000, 1000); // real limits are checked by validateScale/validateTarget on save
    return box;
}

static QWidget* makeSection(const QString& title, QFormLayout*& form) {
    QWidget* card = new QWidget;
    card->setObjectName("card");
    card->setAttribute(Qt::WA_StyledBackground);
    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 16, 20, 16);
    QLabel* label = new QLabel(title);
    label->setObjectName("sectionTitle");
    layout->addWidget(label);
    form = new QFormLayout;
    form->setVerticalSpacing(14);
    layout->addLayout(form);
    return card;
}

// A form row with a short grey tip under the input.
void SettingsPage::addRow(QFormLayout* form, const QString& label, QWidget* field, const QString& tip) {
    QWidget* box = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(box);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);
    layout->addWidget(field);
    if (!tip.isEmpty()) {
        QLabel* tipLabel = new QLabel(tip);
        tipLabel->setObjectName("tipText");
        tipLabel->setWordWrap(true);
        layout->addWidget(tipLabel);
    }
    form->addRow(label, box);
}

SettingsPage::SettingsPage(AppData& appData, QWidget* parent) : QWidget(parent), data(appData) {
    QLabel* title = new QLabel("Settings");
    title->setObjectName("pageTitle");

    QFormLayout* goalForm;
    QWidget* goalCard = makeSection("GWA Goal", goalForm);
    targetInput = makeGradeBox();
    addRow(goalForm, "Target GWA", targetInput,
           "The GWA you are aiming for. On a 1.00–5.00 scale a smaller number is better, "
           "so a target of 1.75 means \"1.75 or better\".");

    QFormLayout* scaleForm;
    QWidget* scaleCard = makeSection("Grading Scale", scaleForm);
    presetInput = new QComboBox;
    presetInput->addItems({"STI College (1.00 – 5.00)", "University of Santo Tomas – UST (1.00 – 5.00)",
                           "University of the East – UE (1.00 – 5.00)", "National University – NU (4.0 – 0.0)",
                           "FEU / FEU Tech (4.0 – 0.0)", "Custom"});
    bestInput = makeGradeBox();
    worstInput = makeGradeBox();
    passingInput = makeGradeBox();
    marginInput = new QDoubleSpinBox;
    marginInput->setRange(0, 50);
    marginInput->setSuffix(" % of scale");
    directionLabel = new QLabel;
    directionLabel->setObjectName("directionText");

    addRow(scaleForm, "School preset", presetInput,
           "Pick your school to fill in its grading scale. Choose Custom if your school is not listed "
           "or uses a different scale, then edit the numbers below.");
    addRow(scaleForm, "Highest grade", bestInput,
           "The best grade you can get (1.00 at STI, UST, UE; 4.0 at NU and FEU).");
    addRow(scaleForm, "Lowest grade", worstInput,
           "The worst grade on the scale, usually the failing grade (5.00, or 0.0 on a 4.0 scale).");
    addRow(scaleForm, "", directionLabel, "");
    addRow(scaleForm, "Passing grade", passingInput,
           "The minimum grade to pass a subject (3.00 on 1.00–5.00 scales, 1.0 on 4.0 scales). "
           "Anything worse is marked Failed.");
    addRow(scaleForm, "\"Almost there\" margin", marginInput,
           "How close to your target still counts as \"almost there\". 5% of a 1.00–5.00 scale is 0.20: "
           "within 0.20 of your target shows NOT YET ACHIEVED instead of AT RISK.");

    QFormLayout* aiForm;
    QWidget* aiCard = makeSection("Gemini AI", aiForm);
    apiKeyInput = new QLineEdit;
    apiKeyInput->setEchoMode(QLineEdit::Password);
    apiKeyInput->setPlaceholderText("Paste your key from Google AI Studio");
    modelInput = new QLineEdit;
    keySourceLabel = new QLabel;
    keySourceLabel->setObjectName("tipText");
    keySourceLabel->setWordWrap(true);
    testButton = new QPushButton("Test Connection");
    addRow(aiForm, "API key", apiKeyInput,
           "A free key from aistudio.google.com/apikey. Only needed for AI feedback; "
           "the calculator works without it.");
    aiForm->addRow("", keySourceLabel);
    addRow(aiForm, "Model", modelInput,
           "Which Gemini model writes your feedback. Keep the default unless you see a \"Model not found\" error.");
    addRow(aiForm, "", testButton, "Checks that your key and model work, without generating feedback.");

    QPushButton* saveButton = new QPushButton("Save Settings");
    saveButton->setObjectName("primaryButton");
    messageLabel = new QLabel;
    messageLabel->setWordWrap(true);
    QHBoxLayout* bottom = new QHBoxLayout;
    bottom->addWidget(saveButton);
    bottom->addWidget(messageLabel, 1);

    QWidget* content = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(content);
    layout->setContentsMargins(32, 28, 32, 28);
    layout->setSpacing(16);
    layout->addWidget(title);
    layout->addWidget(goalCard);
    layout->addWidget(scaleCard);
    layout->addWidget(aiCard);
    layout->addLayout(bottom);
    layout->addStretch();

    QScrollArea* scroll = new QScrollArea;
    scroll->setWidget(content);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);

    auto updateDirection = [this]() {
        if (bestInput->value() < worstInput->value()) {
            directionLabel->setText("↓ Smaller numbers are better on this scale.");
        } else if (bestInput->value() > worstInput->value()) {
            directionLabel->setText("↑ Bigger numbers are better on this scale.");
        } else {
            directionLabel->setText("Highest and lowest grade cannot be the same.");
        }
    };
    connect(bestInput, &QDoubleSpinBox::valueChanged, this, updateDirection);
    connect(worstInput, &QDoubleSpinBox::valueChanged, this, updateDirection);
    connect(presetInput, &QComboBox::currentIndexChanged, this, &SettingsPage::onPresetChanged);
    connect(saveButton, &QPushButton::clicked, this, &SettingsPage::onSaveClicked);
    connect(testButton, &QPushButton::clicked, this, &SettingsPage::onTestClicked);
    connect(&client, &GeminiClient::connectionOk, this, [this]() {
        testButton->setEnabled(true);
        showMessage("Connected! Your key and model work.", "successText");
    });
    connect(&client, &GeminiClient::requestFailed, this, [this](const QString& message) {
        testButton->setEnabled(true);
        showMessage(message, "errorText");
    });
}

void SettingsPage::fillScale(const GradingScale& scale) {
    bestInput->setValue(bestGrade(scale));
    worstInput->setValue(worstGrade(scale));
    passingInput->setValue(scale.passingGrade);
    marginInput->setValue(scale.nearMarginPercent);
}

void SettingsPage::refresh() {
    filling = true;
    targetInput->setValue(data.target);
    presetInput->setCurrentIndex(data.presetIndex);
    fillScale(data.scale);
    apiKeyInput->setText(QString::fromStdString(data.apiKey));
    modelInput->setText(QString::fromStdString(data.model));
    filling = false;

    if (!qEnvironmentVariable("GEMINI_API_KEY").trimmed().isEmpty()) {
        keySourceLabel->setText("Using the GEMINI_API_KEY environment variable (it overrides the field above).");
    } else {
        keySourceLabel->setText("The key is saved only on this computer. Never share your data file.");
    }
}

void SettingsPage::onPresetChanged(int index) {
    if (filling || index == PRESET_CUSTOM) {
        return; // Custom: leave the fields for the user to edit
    }
    fillScale(presetScale(index));
    showMessage("Set a target that fits this scale, then click Save Settings.", "mutedText");
}

// The direction comes from which number is the best grade, so it can never disagree with the labels.
GradingScale SettingsPage::scaleFromForm() const {
    GradingScale s;
    double best = bestInput->value();
    double worst = worstInput->value();
    s.minGrade = std::min(best, worst);
    s.maxGrade = std::max(best, worst);
    s.direction = (best < worst) ? Direction::LowerIsBetter : Direction::HigherIsBetter;
    s.passingGrade = passingInput->value();
    s.nearMarginPercent = marginInput->value();
    return s;
}

void SettingsPage::onSaveClicked() {
    GradingScale scale = scaleFromForm();
    double target = targetInput->value();
    std::string error;

    if (!validateScale(scale, error) || !validateTarget(target, scale, error)) {
        if (scale.minGrade == scale.maxGrade) {
            error = "Highest and lowest grade cannot be the same.";
        }
        showMessage(QString::fromStdString(error), "errorText");
        return;
    }
    QString model = modelInput->text().trimmed();
    if (model.isEmpty()) {
        showMessage("Model name cannot be empty.", "errorText");
        return;
    }

    // Editing a preset's numbers turns it into a custom scale.
    int preset = presetInput->currentIndex();
    if (preset != PRESET_CUSTOM && !sameScale(scale, presetScale(preset))) {
        preset = PRESET_CUSTOM;
    }
    scale.name = presetScale(preset).name;

    data.scale = scale;
    data.presetIndex = preset;
    data.target = target;
    data.model = model.toStdString();
    data.apiKey = apiKeyInput->text().trimmed().toStdString();

    int outOfRange = 0;
    for (const Subject& s : data.subjects) {
        if (!validateGrade(s.grade, scale, error)) {
            outOfRange++;
        }
    }

    emit dataChanged(); // MainWindow saves and calls refresh()
    if (outOfRange > 0) {
        showMessage("Saved. " + QString::number(outOfRange) +
                    " subject(s) have grades outside the new scale. They are shown in red on the Subjects page.",
                    "warningText");
    } else {
        showMessage("Settings saved.", "successText");
    }
}

void SettingsPage::onTestClicked() {
    QString key = apiKeyInput->text().trimmed();
    if (key.isEmpty()) {
        key = resolveApiKey(data);
    }
    if (key.isEmpty()) {
        showMessage("Enter an API key first.", "errorText");
        return;
    }
    testButton->setEnabled(false);
    showMessage("Testing connection…", "mutedText");
    client.testConnection(key, modelInput->text().trimmed());
}

void SettingsPage::showMessage(const QString& text, const QString& kind) {
    messageLabel->setObjectName(kind);
    messageLabel->style()->polish(messageLabel);
    messageLabel->setText(text);
}
