#include "Storage.h"
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>
#include "../core/Validation.h"

QString defaultDataPath() {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/data.json";
}

static QJsonObject scaleToJson(const GradingScale& s) {
    QJsonObject o;
    o["name"] = QString::fromStdString(s.name);
    o["min"] = s.minGrade;
    o["max"] = s.maxGrade;
    o["passing"] = s.passingGrade;
    o["lowerIsBetter"] = (s.direction == Direction::LowerIsBetter);
    o["nearMarginPercent"] = s.nearMarginPercent;
    return o;
}

static GradingScale scaleFromJson(const QJsonObject& o) {
    GradingScale s;
    s.name = o["name"].toString(QString::fromStdString(s.name)).toStdString();
    s.minGrade = o["min"].toDouble(s.minGrade);
    s.maxGrade = o["max"].toDouble(s.maxGrade);
    s.passingGrade = o["passing"].toDouble(s.passingGrade);
    s.direction = o["lowerIsBetter"].toBool(true) ? Direction::LowerIsBetter : Direction::HigherIsBetter;
    s.nearMarginPercent = o["nearMarginPercent"].toDouble(s.nearMarginPercent);
    return s;
}

bool saveRecords(const AppData& data, const QString& path) {
    QJsonArray subjects;
    for (const Subject& s : data.subjects) {
        QJsonObject o;
        o["name"] = QString::fromStdString(s.name);
        o["grade"] = s.grade;
        o["units"] = s.units;
        subjects.append(o);
    }

    QJsonObject root;
    root["subjects"] = subjects;
    root["scale"] = scaleToJson(data.scale);
    root["presetIndex"] = data.presetIndex;
    root["target"] = data.target;
    root["model"] = QString::fromStdString(data.model);
    root["apiKey"] = QString::fromStdString(data.apiKey);
    root["lastFeedback"] = QString::fromStdString(data.lastFeedback);
    root["lastFeedbackDate"] = QString::fromStdString(data.lastFeedbackDate);
    root["feedbackStale"] = data.feedbackStale;

    // QSaveFile writes to a temp file first, so a crash never leaves a half-written data.json.
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    file.write(QJsonDocument(root).toJson());
    return file.commit();
}

bool loadRecords(AppData& data, const QString& path) {
    data = AppData(); // start from defaults
    QFile file(path);
    if (!file.exists()) {
        return true; // first run
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        QFile::remove(path + ".bak");
        QFile::rename(path, path + ".bak"); // keep the broken file so nothing is lost
        return false;
    }

    QJsonObject root = doc.object();
    data.scale = scaleFromJson(root["scale"].toObject());
    data.presetIndex = root["presetIndex"].toInt(PRESET_STI);
    if (data.presetIndex < 0 || data.presetIndex > PRESET_CUSTOM) {
        data.presetIndex = PRESET_CUSTOM;
    }
    // Files from older versions may name a preset that no longer matches the saved numbers.
    if (data.presetIndex != PRESET_CUSTOM && !sameScale(data.scale, presetScale(data.presetIndex))) {
        data.presetIndex = PRESET_CUSTOM;
    }
    data.scale.name = presetScale(data.presetIndex).name;
    data.target = root["target"].toDouble(data.target);
    data.model = root["model"].toString(QString::fromStdString(data.model)).toStdString();
    data.apiKey = root["apiKey"].toString().toStdString();
    data.lastFeedback = root["lastFeedback"].toString().toStdString();
    data.lastFeedbackDate = root["lastFeedbackDate"].toString().toStdString();
    data.feedbackStale = root["feedbackStale"].toBool(false);

    QJsonArray subjects = root["subjects"].toArray();
    int i = 0;
    while (i < subjects.size()) {
        QJsonObject o = subjects[i].toObject();
        Subject s;
        s.name = o["name"].toString().toStdString();
        s.grade = o["grade"].toDouble();
        s.units = o["units"].toInt();
        std::string error;
        if (validateName(s.name, error) && validateUnits(s.units, error)) {
            data.subjects.push_back(s); // out-of-range grades are kept and shown in red
        }
        i++;
    }
    return true;
}
