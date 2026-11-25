#include "GeminiClient.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

static const char* API_BASE = "https://generativelanguage.googleapis.com/v1beta/models/";
static const int TIMEOUT_MS = 30000;

QString resolveApiKey(const AppData& data) {
    QString fromEnv = qEnvironmentVariable("GEMINI_API_KEY").trimmed();
    if (!fromEnv.isEmpty()) {
        return fromEnv;
    }
    return QString::fromStdString(data.apiKey).trimmed();
}

static QStringList toStringList(const QJsonValue& value) {
    QStringList list;
    QJsonArray array = value.toArray();
    for (int i = 0; i < array.size(); i++) {
        QString item = array[i].toString().trimmed();
        if (!item.isEmpty()) {
            list.append(item);
        }
    }
    return list;
}

Feedback parseFeedbackText(const QString& text, const QStringList& knownNames) {
    Feedback fb;
    QString cleaned = text.trimmed();
    // Some models wrap JSON in ```json ... ``` even when asked not to.
    if (cleaned.startsWith("```")) {
        cleaned = cleaned.mid(cleaned.indexOf('\n') + 1);
        if (cleaned.endsWith("```")) {
            cleaned.chop(3);
        }
    }

    QJsonDocument doc = QJsonDocument::fromJson(cleaned.toUtf8());
    if (!doc.isObject()) {
        fb.summary = text; // not JSON: show the plain answer rather than nothing
        return fb;
    }

    QJsonObject root = doc.object();
    fb.summary = root["summary"].toString();
    fb.strengths = toStringList(root["strengths"]);
    fb.tips = toStringList(root["study_tips"]);

    QJsonArray priorities = root["priority_subjects"].toArray();
    for (int i = 0; i < priorities.size(); i++) {
        QJsonObject item = priorities[i].toObject();
        QString name = item["name"].toString().trimmed();
        // Anti-hallucination check: only keep subjects the student actually entered.
        for (int k = 0; k < knownNames.size(); k++) {
            if (knownNames[k].compare(name, Qt::CaseInsensitive) == 0) {
                fb.priorities.append({knownNames[k], item["reason"].toString()});
                break;
            }
        }
    }
    return fb;
}

GeminiClient::GeminiClient(QObject* parent) : QObject(parent) {}

QNetworkRequest GeminiClient::makeRequest(const QString& url, const QString& apiKey) const {
    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("x-goog-api-key", apiKey.toUtf8()); // header, not URL, so the key never shows in logs
    request.setTransferTimeout(TIMEOUT_MS);
    return request;
}

void GeminiClient::generateAIFeedback(const QString& prompt, const QString& apiKey, const QString& model) {
    QJsonObject part{{"text", prompt}};
    QJsonObject content{{"role", "user"}, {"parts", QJsonArray{part}}};
    QJsonObject config{{"temperature", 0.4}, {"responseMimeType", "application/json"}};
    QJsonObject body{{"contents", QJsonArray{content}}, {"generationConfig", config}};

    QString url = API_BASE + QUrl::toPercentEncoding(model) + ":generateContent";
    QNetworkReply* reply = network.post(makeRequest(url, apiKey), QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        QByteArray raw = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            emit requestFailed(describeError(reply, raw));
            return;
        }

        QJsonObject root = QJsonDocument::fromJson(raw).object();
        QJsonArray candidates = root["candidates"].toArray();
        if (candidates.isEmpty()) {
            QString reason = root["promptFeedback"].toObject()["blockReason"].toString();
            emit requestFailed(reason.isEmpty() ? "Gemini returned no answer. Please try again."
                                                : "Gemini blocked the request (" + reason + ").");
            return;
        }

        // The answer can be split across several parts; join them.
        QJsonArray parts = candidates[0].toObject()["content"].toObject()["parts"].toArray();
        QString text;
        int i = 0;
        while (i < parts.size()) {
            text += parts[i].toObject()["text"].toString();
            i++;
        }
        if (text.trimmed().isEmpty()) {
            emit requestFailed("Gemini returned an empty answer. Please try again.");
            return;
        }
        emit feedbackReady(text);
    });
}

void GeminiClient::testConnection(const QString& apiKey, const QString& model) {
    // Reading the model's info checks the key and the model name without using generation quota.
    QString url = API_BASE + QUrl::toPercentEncoding(model);
    QNetworkReply* reply = network.get(makeRequest(url, apiKey));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        QByteArray raw = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            emit requestFailed(describeError(reply, raw));
        } else {
            emit connectionOk();
        }
    });
}

QString GeminiClient::describeError(QNetworkReply* reply, const QByteArray& body) const {
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString apiMessage = QJsonDocument::fromJson(body).object()["error"].toObject()["message"].toString();

    if (status != 0) {
        // The server answered, so the problem is with the request, key, or quota.
        if (status == 400 && body.contains("API_KEY_INVALID")) {
            return "Your API key is not valid. Copy it again from Google AI Studio.";
        } else if (status == 400) {
            return "Gemini rejected the request (check the model name). " + apiMessage;
        } else if (status == 401 || status == 403) {
            return "Your API key is invalid or not allowed to use this model.";
        } else if (status == 404) {
            return "Model not found. Check the model name in Settings.";
        } else if (status == 429) {
            return "Free-tier limit reached. Wait a while and try again.";
        } else if (status >= 500) {
            return "Gemini is busy or having problems right now. Try again later.";
        } else {
            return "Unexpected response from Gemini (HTTP " + QString::number(status) + ").";
        }
    }

    // No HTTP status: the request never reached Gemini.
    switch (reply->error()) {
    case QNetworkReply::HostNotFoundError:
    case QNetworkReply::NetworkSessionFailedError:
    case QNetworkReply::TemporaryNetworkFailureError:
        return "No internet connection. The calculator still works offline.";
    case QNetworkReply::OperationCanceledError:
    case QNetworkReply::TimeoutError:
        return "Gemini took too long to answer (timed out). Try again.";
    case QNetworkReply::SslHandshakeFailedError:
        return "Secure connection failed (SSL). Check your system date or TLS support.";
    default:
        return "Network error: " + reply->errorString();
    }
}
