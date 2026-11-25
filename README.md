# GradeWise — GWA Calculator & AI Study Advisor

A C++17 / Qt 6 desktop app that computes a student's **General Weighted Average (GWA)**, checks it against a target GWA, and asks **Google Gemini** for study advice based only on the student's actual grades.

> **Academic project:** made as a requirement for the **Computer Programming 1 Laboratory Finals** at **FEU Tech** (FEU Institute of Technology). The code also demonstrates the course's Module 5 (conditionals), Module 6 (loops), and Module 7 (functions and recursion) lessons. See [Programming Concepts Map](#programming-concepts-map).

---

## Table of Contents

- [Overview](#overview)
- [Key Features](#key-features)
- [Technology Stack](#technology-stack)
- [Project Structure](#project-structure)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Gemini API Key Setup](#gemini-api-key-setup)
- [Usage](#usage)
- [Grading Scale Presets](#grading-scale-presets)
- [Architecture & Workflow](#architecture--workflow)
- [Programming Concepts Map](#programming-concepts-map)
- [Testing](#testing)
- [Data Storage & Security](#data-storage--security)
- [Troubleshooting](#troubleshooting)
- [Future Improvements](#future-improvements)
- [Credits](#credits)

---

## Overview

Students often compute their GWA by hand, make arithmetic mistakes, and can't easily tell which subject is pulling their average down the most. A low grade in a 3-unit subject hurts more than the same grade in a 1-unit subject.

GradeWise solves this in three screens:

| Screen | What it does |
|---|---|
| **Dashboard** | Shows the current GWA, target GWA, subject count, total units, a color-coded goal status banner, and the AI feedback panel. |
| **Subjects** | Add, edit, and delete subjects (name, grade, units) in a table. Totals and GWA update on every change. |
| **Settings** | Set the target GWA, pick a school grading scale (or a custom one), and enter the Gemini API key and model. Every field has a short tip explaining it. |

The calculator works **fully offline**. Only the AI feedback needs internet and a free Gemini API key.

## Key Features

- **Live GWA calculation:** `GWA = Σ(grade × units) ÷ Σ(units)`, recalculated after every add, edit, or delete. With no subjects, the GWA shows `—` instead of dividing by zero.
- **Direction-aware goal status:** the dashboard banner shows
  - 🟢 **GOAL ACHIEVED**: the GWA meets or beats the target.
  - 🟡 **GOAL NOT YET ACHIEVED**: the GWA is within the "almost there" margin (default 5% of the scale range, i.e. 0.20 on a 1.00–5.00 scale).
  - 🔴 **GOAL AT RISK**: the GWA is farther than the margin, or it is a failing average.
  
  This works for scales where **lower is better** (1.00–5.00) and where **higher is better** (4.0).
- **Biggest-impact subject:** subjects are ranked by `(distance from the best grade) × units`, so the dashboard names the subject that lowers the GWA the most.
- **Philippine university presets:** STI College, UST, UE, NU, and FEU / FEU Tech, plus a **Custom** scale.
- **Input validation:** empty or duplicate names (case-insensitive), grades outside the scale, and units outside 1–10 are rejected with an inline red message. A blank name automatically becomes `Subject N`.
- **Performance remarks:** each subject is labeled Passed (Excellent / Very Good / Good / Fair) or Failed. Failing and out-of-scale grades are shown in red.
- **Gemini AI feedback:** returns a summary, subjects to prioritize (with reasons), strengths, and study recommendations.
  - The prompt contains only the app's own computed numbers and tells the model not to invent data or recalculate.
  - **Anti-hallucination check:** any priority subject the AI names that is not in the student's list is dropped.
  - The last feedback is saved with its date and marked out of date when grades or settings change.
- **Clear error handling:** invalid API key, unknown model, free-tier limit (HTTP 429), server errors, 30-second timeout, no internet, SSL failure, and empty or blocked answers each show a plain-language message and a **Retry** button.
- **Autosave:** data is saved to a local JSON file after every change. A corrupt file is kept as `data.json.bak`, and the app starts with empty data instead of crashing.
- **Consistent look:** Fusion style with a forced light palette and one stylesheet, so the app looks the same on Linux and Windows, even under a dark system theme.

## Technology Stack

| Area | Technology |
|---|---|
| Language | C++17 |
| GUI | Qt 6 Widgets (layouts written in C++, no `.ui` files) |
| Networking | Qt 6 Network (`QNetworkAccessManager`, asynchronous HTTPS) |
| JSON | Qt's built-in `QJsonDocument` / `QJsonObject` |
| AI | Google Gemini REST API (`generateContent`), default model `gemini-3.5-flash-lite` |
| Build system | CMake ≥ 3.19 |
| Styling | Qt Style Sheet (`resources/style.qss`) compiled in through `resources.qrc` |
| Tests | Plain `assert`-based executable run by CTest |

There are no third-party libraries. Everything comes from the C++ standard library and Qt.

## Project Structure

```
gradeWise/
├── CMakeLists.txt              # Builds the app (gwa_calculator) and the tests (core_tests)
├── README.md
├── .gitignore                  # build/, data.json, *.docx, .claude/, ...
├── resources/
│   ├── resources.qrc           # Embeds the stylesheet into the executable
│   └── style.qss               # All colors and fonts
├── src/
│   ├── main.cpp                # Starts Qt, applies the style, shows the main window
│   ├── core/                   # Plain standard C++ (no Qt): the calculation logic
│   │   ├── Subject.h           # Subject record + weightedGrade()
│   │   ├── GradingScale.h/.cpp # Scale struct, presets, "which grade is better" helpers
│   │   ├── Calculator.h/.cpp   # Totals (recursive + iterative), GWA, goal status, ranking
│   │   ├── SubjectManager.h/.cpp # Add / edit / delete / find subjects
│   │   ├── Validation.h/.cpp   # Input checks and number formatting
│   │   └── AppData.h           # Everything the app saves
│   ├── storage/
│   │   └── Storage.h/.cpp      # saveRecords() / loadRecords() as JSON
│   ├── ai/
│   │   ├── PromptBuilder.h/.cpp # Builds the Gemini prompt from computed data (no Qt)
│   │   └── GeminiClient.h/.cpp  # HTTPS request, response parsing, error messages
│   └── gui/
│       ├── MainWindow.h/.cpp   # Sidebar + page stack; owns AppData; refresh + autosave
│       ├── DashboardPage.h/.cpp # Stat cards, goal banner, embeds the feedback panel
│       ├── FeedbackPage.h/.cpp  # AI feedback panel shown on the Dashboard
│       ├── SubjectsPage.h/.cpp  # Subject form and table
│       └── SettingsPage.h/.cpp  # Goal, grading scale, and Gemini settings
└── tests/
    └── core_tests.cpp          # Tests for the core logic and the prompt
```

## Prerequisites

| Requirement | Version | Notes |
|---|---|---|
| C++ compiler | C++17 support | GCC 9+ on Linux, or MinGW 64-bit (bundled with Qt) on Windows |
| CMake | 3.19 or newer | |
| Qt | 6.3 or newer | Needs the **Widgets** and **Network** modules. Developed with Qt 6.4.2. |
| Gemini API key | — | Optional. Only needed for AI feedback. Free from [Google AI Studio](https://aistudio.google.com/apikey). |

## Installation

### 1. Clone the repository

```bash
git clone https://github.com/germanica-1/gradeWise.git
cd gradeWise
```

### 2a. Linux (Ubuntu / Debian)

```bash
# Install the compiler, CMake, and Qt 6 (Widgets + Network are in qt6-base-dev)
sudo apt install -y build-essential cmake qt6-base-dev

# Configure and build
cmake -S . -B build
cmake --build build -j

# Run the tests (optional)
ctest --test-dir build

# Start the app
./build/gwa_calculator
```

### 2b. Windows

1. Install Qt with the [Qt Online Installer](https://www.qt.io/download-qt-installer). Select:
   - **Qt 6.x → MinGW 64-bit**
   - Under *Developer and Designer Tools*: **Qt Creator**, **CMake**, **Ninja**, and the matching **MinGW** toolchain
2. Open Qt Creator → **File → Open File or Project…** → select `CMakeLists.txt`.
3. Choose the **Desktop Qt 6.x MinGW 64-bit** kit, then click **Build** and **Run**.

To run the `.exe` on a computer without Qt, open the **Qt 6.x (MinGW) command prompt** from the Start menu and run:

```bat
windeployqt path\to\gwa_calculator.exe
```

This copies the Qt DLLs and plugins next to the executable, including the **`tls`** plugin. Without that plugin, every Gemini request fails.

## Gemini API Key Setup

The API key is **never stored in the source code**. The app looks for it in this order:

1. The **`GEMINI_API_KEY` environment variable** (takes priority)
2. The key typed in **Settings → Gemini AI → API key** (saved in the local data file)

**Option A: environment variable**

```bash
# Linux (add to ~/.bashrc to keep it)
export GEMINI_API_KEY="your-key"
./build/gwa_calculator
```

```bat
:: Windows (then reopen Qt Creator or the terminal)
setx GEMINI_API_KEY "your-key"
```

**Option B: in the app.** Paste the key in **Settings**, click **Test Connection**, then **Save Settings**.

**Test Connection** reads the model's info, which checks the key and the model name without using generation quota. When the environment variable is set, Settings says that it overrides the field.

## Usage

1. **Settings:** choose your school preset and set your **Target GWA** (default 1.75), then click **Save Settings**.
2. **Subjects:** enter each subject's name, grade, and units, then click **Add Subject** (or press Enter).
   - Select a row and click **Edit Selected** (or double-click the row) to change it.
   - Click **Delete Selected** to remove it. A confirmation dialog appears first.
3. **Dashboard:** read your GWA and goal status, then click **Generate Feedback** for AI advice.

### Worked example (STI scale, target 1.75)

| Subject | Grade | Units | Weighted |
|---|---|---|---|
| Math | 1.50 | 3 | 4.50 |
| Programming | 1.25 | 3 | 3.75 |
| PE | 2.00 | 2 | 4.00 |
| English | 2.50 | 3 | 7.50 |
| **Total** | | **11** | **19.75** |

`GWA = 19.75 ÷ 11 = 1.80`

On this scale lower is better, so the gap is `1.80 − 1.75 = 0.05`. The margin is `5% × (5.00 − 1.00) = 0.20`, and the gap is within it, so the status is **GOAL NOT YET ACHIEVED**.

English is the biggest-impact subject. Its 2.50 is the farthest from 1.00, across 3 units. A plain "lowest grade" sort would ignore the units.

## Grading Scale Presets

| Preset | Best | Passing | Worst | Direction | Source |
|---|---|---|---|---|---|
| STI College | 1.00 | 3.00 | 5.00 | Lower is better | Consistent across published STI grading guides |
| UST | 1.00 | 3.00 | 5.00 | Lower is better | UST Student Handbook (2018) |
| UE | 1.00 | 3.00 | 5.00 | Lower is better | UE Student Manual (2023) |
| NU | 4.0 | 1.0 | 0.0 | Higher is better | Third-party guides only. Confirm with the registrar. |
| FEU / FEU Tech | 4.0 | 1.0 (D) | 0.0 (F) | Higher is better | FEU Student Handbook (2024). FEU Tech assumed to match. |
| Custom | any | any | any | Set by the best and worst grades | — |

In Settings, **Highest grade** means the *best* grade and **Lowest grade** means the *worst*. The app works out the direction from them: if the best is the smaller number, lower is better. If you edit a preset's numbers, the preset switches to **Custom**.

## Architecture & Workflow

```
┌──────────────────────── GUI (Qt Widgets) ─────────────────────────┐
│ MainWindow: sidebar + page stack, owns the single AppData         │
│   DashboardPage (+ FeedbackPage)   SubjectsPage   SettingsPage    │
└──────┬──────────────────────┬───────────────────────┬─────────────┘
       │ calls                │ calls                 │ async signals
┌──────▼───────────┐  ┌───────▼────────┐  ┌───────────▼───────────┐
│ core/ (plain C++)│  │ storage/       │  │ ai/                   │
│ Calculator,      │  │ JSON save/load │  │ PromptBuilder (C++)   │
│ GradingScale,    │  │ (QSaveFile)    │  │ GeminiClient (HTTPS)  │
│ Validation,      │  └────────────────┘  └───────────────────────┘
│ SubjectManager   │
└──────────────────┘
```

- **Single source of truth:** `MainWindow` owns one `AppData`. A page edits it, then emits `dataChanged`. `MainWindow` marks old AI feedback as out of date, refreshes every page, and saves to disk.
- **Separated logic:** `src/core` and `PromptBuilder` use only standard C++, so they can be tested and explained without a GUI. Only `GeminiClient` talks to the network, and it never calculates anything. It receives a finished prompt.
- **One place for scale direction:** `isBetterOrEqual()`, `isPassing()`, and `goodness()` are the only functions that know whether lower or higher is better. Everything else uses them, so both scale types behave correctly.
- **Non-blocking AI request:** `QNetworkAccessManager` is asynchronous, so the window stays responsive while Gemini answers. There are no threads.

### AI feedback flow

1. Click **Generate Feedback**. The button is disabled while the request runs.
2. `buildPrompt()` writes the scale, target, GWA, total units, goal status, and every subject ranked by impact. It adds instructions to use only this data and to answer in a fixed JSON shape (`summary`, `priority_subjects`, `strengths`, `study_tips`).
3. `GeminiClient` sends `POST …/v1beta/models/{model}:generateContent`, with the key in the `x-goog-api-key` header (never in the URL), `temperature 0.4`, JSON output, and a 30 s timeout.
4. The reply's text parts are joined, code fences are stripped, and the JSON is parsed. If the answer is not JSON, the raw text is shown as the summary.
5. Unknown subject names are removed, then the result is shown and saved with a timestamp.

## Programming Concepts Map

| Module | Concept | Where it is used |
|---|---|---|
| 5 | `if` | `calculateGWA` (zero-units guard), `resolveApiKey` (env var set?), `deleteSubject` (index check) |
| 5 | `if-else` | `isBetterOrEqual` (scale direction), `goodness`, `SubjectsPage::onSaveClicked` (add vs. edit) |
| 5 | `else-if` ladder | `performanceCategory`, `validateUnits`, `validateScale`, `GeminiClient::describeError` (HTTP status), `FeedbackPage::refresh` |
| 5 | Nested conditionals | `evaluateGoal` (achieved? → failing? → within margin?), `validateGrade`, `loadRecords` |
| 5 | `switch` | `presetScale`, `goalStatusText`, `statusColor`, `describeError` (network error type) |
| 6 | `for` | `sumWeightedIterative`, `sumUnitsIterative`, `rankByImpact`, building the prompt and table, anti-hallucination name check |
| 6 | `while` | `findSubject`, `trim`, `rankByImpact` (insertion sort), `loadRecords`, joining Gemini response parts |
| 6 | `do-while` | `nextDefaultName` (tries `Subject N` until the name is unused) |
| 7 | Functions | Every module; each function has one job |
| 7 | Recursion | `sumWeightedRecursive`, `sumUnitsRecursive` (used by `calculateGWA`) |

**Recursion vs. iteration:** `sum(i) = weighted(subject[i]) + sum(i + 1)`, with the base case `i == size → 0`.

- The recursive version reads like the Σ formula, so it is easy to check against the math.
- The `for`-loop version does the same work without one stack frame per subject, so it is the better choice for very large lists.
- With the 5–15 subjects of a school term, both are instant. The app uses the recursive version, and the tests check that both always give the same result.

## Testing

```bash
cmake --build build -j
ctest --test-dir build --output-on-failure
```

`tests/core_tests.cpp` covers:

- totals and GWA (the worked example and a higher-is-better example)
- recursive vs. iterative sums
- goal status on both scale directions
- performance categories and impact ranking
- add, edit, delete, and duplicate checks
- scale validation, the university presets, and the generated prompt

## Data Storage & Security

All data is saved in a single JSON file:

| OS | Path |
|---|---|
| Linux | `~/.local/share/GwaCalculator/data.json` |
| Windows | `%APPDATA%\GwaCalculator\data.json` |

- Writes go through `QSaveFile`, which writes to a temporary file first, so a crash never leaves a half-written file.
- If you save the API key in Settings, it is stored in this file in plain text. **Do not share or commit this file.** Use the `GEMINI_API_KEY` environment variable if you would rather not store the key on disk.
- The key is sent only to Google in a request header. It is masked in the Settings field and never appears in error messages.
- `.gitignore` excludes `build/`, `data.json`, `data.json.bak`, `*.docx`, `.claude/`, and `CLAUDE.md`.

## Troubleshooting

**`Could not find a package configuration file provided by "Qt6"` when running CMake**
Qt 6 is not installed or CMake can't find it. On Linux, install `qt6-base-dev`. Otherwise, point CMake at your Qt install:
```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/gcc_64
```

**Unknown CMake command `qt_standard_project_setup`**
Your Qt is older than 6.3. Install Qt 6.3 or newer.

**"Model not found. Check the model name in Settings."**
Google renames and retires models often. Pick a current model from the [Gemini models list](https://ai.google.dev/gemini-api/docs/models), enter it in Settings, then click **Test Connection**.

**"Your API key is not valid."**
Copy the key again from Google AI Studio. If `GEMINI_API_KEY` is set, it overrides the Settings field, so check that variable too.

**"Free-tier limit reached."**
You hit the free-tier rate limit (HTTP 429). Wait a while and retry. The calculator keeps working.

**"Secure connection failed (SSL)" or every request fails on another Windows PC**
Run `windeployqt` so the `tls` plugin is copied next to the `.exe`. Also check that the PC's date and time are correct.

**"No internet connection."**
The AI feedback needs internet. Everything else works offline.

**"Your saved data could not be read" on startup**
The data file was corrupt. It was renamed to `data.json.bak` in the data folder, and the app started with empty data. You can inspect or repair the backup.

**Subjects shown in red with "Outside scale!" after changing the preset**
Their grades don't fit the new scale (for example, 1.50 on a 0–4 scale where higher is better). Edit those subjects, or switch back to the previous preset.

**The app looks different from the screenshots under a dark theme**
It shouldn't: the app forces Fusion and a light palette. If colors still look wrong, make sure the build includes `resources/resources.qrc`. `CMAKE_AUTORCC` must be `ON`, as it is in `CMakeLists.txt`.

## Future Improvements

- "What-if" calculator: the grade needed in one subject to reach the target.
- Multiple semesters or terms with GWA history.
- Export the subject table to CSV or PDF.
- Store the API key in the OS keychain instead of the data file.
- Confirm the NU and FEU Tech presets with official registrar documents.
- Optional dark theme.
- A Windows installer.

## Credits

- Developed as a requirement for the **Computer Programming 1 Laboratory Finals** at **FEU Tech**.
- Built with [Qt 6](https://www.qt.io/) and the [Google Gemini API](https://ai.google.dev/).
- Grading scale information from the UST Student Handbook (2018), the UE Student Manual (2023), the FEU Student Handbook (2024), and published STI and NU grading guides.
