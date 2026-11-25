#pragma once
#include <QString>
#include "../core/AppData.h"

QString defaultDataPath(); // e.g. ~/.local/share/GwaCalculator/data.json on Linux

bool saveRecords(const AppData& data, const QString& path);
// Missing file -> defaults, returns true. Corrupt file -> renamed to .bak, defaults, returns false.
bool loadRecords(AppData& data, const QString& path);
