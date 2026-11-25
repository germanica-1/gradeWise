#include "FeedbackPage.h"
#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QTextBrowser>
#include <QVBoxLayout>
#include "../ai/PromptBuilder.h"

FeedbackPage::FeedbackPage(AppData& appData, QWidget* parent) : QWidget(parent), data(appData) {
    QLabel* title = new QLabel("AI Academic Feedback");
    title->setObjectName("sectionTitle");

    generateButton = new QPushButton("Generate Feedback");
    generateButton->setObjectName("primaryButton");
    generateButton->setCursor(Qt::PointingHandCursor);
    modelLabel = new QLabel;
    modelLabel->setObjectName("mutedText");
    QHBoxLayout* top = new QHBoxLayout;
    top->addWidget(generateButton);
    top->addWidget(modelLabel);
    top->addStretch();

    stateLabel = new QLabel;
    stateLabel->setWordWrap(true);

    output = new QTextBrowser;
    output->setObjectName("feedbackOutput");
    output->setOpenExternalLinks(false);

    QVBoxLayout* layout = new QVBoxLayout(this); // no outer margins: this panel sits inside the Dashboard
    layout->setContentsMargins(0, 8, 0, 0);
    layout->setSpacing(10);
    layout->addWidget(title);
    layout->addLayout(top);
    layout->addWidget(stateLabel);
    layout->addWidget(output, 1);

    connect(generateButton, &QPushButton::clicked, this, &FeedbackPage::onGenerateClicked);
    connect(&client, &GeminiClient::feedbackReady, this, &FeedbackPage::onFeedbackReady);
    connect(&client, &GeminiClient::requestFailed, this, &FeedbackPage::onFailed);
}

void FeedbackPage::refresh() {
    modelLabel->setText("Model: " + QString::fromStdString(data.model));
    if (loading) {
        return; // keep the "Analyzing…" message until the reply arrives
    }

    bool hasKey = !resolveApiKey(data).isEmpty();
    bool hasSubjects = !data.subjects.empty();
    generateButton->setEnabled(hasKey && hasSubjects);
    generateButton->setText("Generate Feedback"); // onFailed() switches it to "Retry" after this

    stateLabel->setObjectName("mutedText");
    if (!hasKey) {
        stateLabel->setText("AI feedback is off: add a Gemini API key in Settings (or set GEMINI_API_KEY). "
                            "Everything else works without it.");
    } else if (!hasSubjects) {
        stateLabel->setText("Add at least one subject first.");
    } else if (data.lastFeedback.empty()) {
        stateLabel->setText("Click Generate Feedback to get advice based on your actual grades.");
    } else if (data.feedbackStale) {
        stateLabel->setObjectName("warningText");
        stateLabel->setText("Generated on " + QString::fromStdString(data.lastFeedbackDate) +
                            ". Your grades or settings changed since then, so generate again for updated advice.");
    } else {
        stateLabel->setText("Generated on " + QString::fromStdString(data.lastFeedbackDate) + ".");
    }
    stateLabel->style()->polish(stateLabel); // re-apply the stylesheet after changing objectName
    showFeedback();
}

void FeedbackPage::setLoading(bool isLoading) {
    loading = isLoading;
    generateButton->setEnabled(!isLoading);
    if (isLoading) {
        stateLabel->setObjectName("mutedText");
        stateLabel->style()->polish(stateLabel);
        stateLabel->setText("Analyzing your grades… (this can take a few seconds)");
    }
}

void FeedbackPage::onGenerateClicked() {
    QString key = resolveApiKey(data);
    if (key.isEmpty() || data.subjects.empty() || loading) {
        return;
    }
    setLoading(true);
    client.generateAIFeedback(QString::fromStdString(buildPrompt(data)), key, QString::fromStdString(data.model));
}

void FeedbackPage::onFeedbackReady(const QString& jsonText) {
    data.lastFeedback = jsonText.toStdString();
    data.lastFeedbackDate = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm").toStdString();
    data.feedbackStale = false;
    setLoading(false);
    generateButton->setText("Generate Feedback");
    emit feedbackSaved();
    refresh();
}

void FeedbackPage::onFailed(const QString& message) {
    setLoading(false);
    refresh();
    generateButton->setText("Retry");
    stateLabel->setObjectName("errorText");
    stateLabel->style()->polish(stateLabel);
    stateLabel->setText(message);
}

static QString htmlList(const QStringList& items) {
    QString html = "<ul>";
    for (int i = 0; i < items.size(); i++) {
        html += "<li>" + items[i].toHtmlEscaped() + "</li>";
    }
    return html + "</ul>";
}

void FeedbackPage::showFeedback() {
    if (data.lastFeedback.empty()) {
        output->clear();
        return;
    }

    QStringList names;
    for (const Subject& s : data.subjects) {
        names.append(QString::fromStdString(s.name));
    }
    Feedback fb = parseFeedbackText(QString::fromStdString(data.lastFeedback), names);

    QString html = "<h3>Summary</h3><p>" + fb.summary.toHtmlEscaped() + "</p>";
    if (!fb.priorities.isEmpty()) {
        html += "<h3>Subjects to Prioritize</h3><ol>";
        for (int i = 0; i < fb.priorities.size(); i++) {
            html += "<li><b>" + fb.priorities[i].first.toHtmlEscaped() + "</b> — " +
                    fb.priorities[i].second.toHtmlEscaped() + "</li>";
        }
        html += "</ol>";
    }
    if (!fb.strengths.isEmpty()) {
        html += "<h3>Strengths</h3>" + htmlList(fb.strengths);
    }
    if (!fb.tips.isEmpty()) {
        html += "<h3>Study Recommendations</h3>" + htmlList(fb.tips);
    }
    html += "<p style='color:#7A869A'><i>AI-generated advice. Grades and GWA are computed by this app, not by the AI.</i></p>";
    output->setHtml(html);
}
