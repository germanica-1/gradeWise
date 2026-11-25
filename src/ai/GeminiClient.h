#pragma once
#include <QList>
#include <QNetworkAccessManager>
#include <QObject>
#include <QPair>
#include <QStringList>
#include "../core/AppData.h"

class QNetworkReply;

struct Feedback {
    QString summary;
    QList<QPair<QString, QString>> priorities; // subject name, reason
    QStringList strengths;
    QStringList tips;
};

// Environment variable GEMINI_API_KEY first, otherwise the key saved in Settings.
QString resolveApiKey(const AppData& data);

// Turns Gemini's JSON text into a Feedback. Priority subjects not in `knownNames` are dropped.
Feedback parseFeedbackText(const QString& text, const QStringList& knownNames);

class GeminiClient : public QObject {
    Q_OBJECT
public:
    explicit GeminiClient(QObject* parent = nullptr);

    void generateAIFeedback(const QString& prompt, const QString& apiKey, const QString& model);
    void testConnection(const QString& apiKey, const QString& model);

signals:
    void feedbackReady(const QString& jsonText);
    void connectionOk();
    void requestFailed(const QString& message);

private:
    QNetworkRequest makeRequest(const QString& url, const QString& apiKey) const;
    QString describeError(QNetworkReply* reply, const QByteArray& body) const;

    QNetworkAccessManager network;
};
