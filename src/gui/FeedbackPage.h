#pragma once
#include <QWidget>
#include "../ai/GeminiClient.h"
#include "../core/AppData.h"

class QLabel;
class QPushButton;
class QTextBrowser;

// The AI feedback panel shown at the bottom of the Dashboard.
class FeedbackPage : public QWidget {
    Q_OBJECT
public:
    explicit FeedbackPage(AppData& data, QWidget* parent = nullptr);
    void refresh();

signals:
    void feedbackSaved();

private:
    void onGenerateClicked();
    void onFeedbackReady(const QString& jsonText);
    void onFailed(const QString& message);
    void showFeedback();
    void setLoading(bool loading);

    AppData& data;
    GeminiClient client;
    bool loading = false;

    QPushButton* generateButton;
    QLabel* modelLabel;
    QLabel* stateLabel;
    QTextBrowser* output;
};
