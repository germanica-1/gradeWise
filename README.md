# GWA Calculator with Gemini AI

A C++17 / Qt 6 desktop app that computes your General Weighted Average (GWA), checks it against your target, and asks Google Gemini for study advice based on your actual grades.

- GWA = Σ(grade × units) ÷ Σ(units), recalculated on every add, edit, or delete.
- School presets plus a Custom scale:

  | Preset | Best | Passing | Worst | Source |
  |---|---|---|---|---|
  | STI College | 1.00 | 3.00 | 5.00 | consistent across published STI guides |
  | UST | 1.00 | 3.00 | 5.00 | UST Student Handbook (2018) |
  | UE | 1.00 | 3.00 | 5.00 | UE Student Manual (2023) |
  | NU | 4.0 | 1.0 | 0.0 | third-party guides only (official site unreachable). Confirm with the registrar |
  | FEU / FEU Tech | 4.0 | 1.0 (D) | 0 (F) | FEU Student Handbook (2024). FEU Tech assumed to match; confirm |
- Goal status: **ACHIEVED**, **NOT YET ACHIEVED** (within the "close" margin), **AT RISK**.
- Works fully offline. Only the AI Feedback page needs internet.

## Build and run

### Linux (Ubuntu 24.04)
```bash
sudo apt install -y build-essential cmake qt6-base-dev
cmake -S . -B build
cmake --build build -j
ctest --test-dir build        # core logic tests
./build/gwa_calculator
```

### Windows
1. Install Qt with the [Qt Online Installer](https://www.qt.io/download-qt-installer). Select **Qt 6.x → MinGW 64-bit**, plus **Qt Creator**, **CMake** and **Ninja** under Developer and Designer Tools.
2. In Qt Creator: **File → Open File or Project → `CMakeLists.txt`**, choose the MinGW kit, then **Build** and **Run**.
3. To run the `.exe` on another PC, open the *Qt 6.x MinGW* command prompt and run:
   `windeployqt path\to\gwa_calculator.exe`
   This copies the Qt DLLs, including the `tls` plugin. Without that plugin, Gemini requests fail.

## Gemini API key
1. Get a free key at [Google AI Studio](https://aistudio.google.com/apikey).
2. Either paste it in **Settings → Gemini AI → API key**, or set an environment variable (this one takes priority):
   - Linux: `export GEMINI_API_KEY="your-key"`
   - Windows: `setx GEMINI_API_KEY "your-key"`, then reopen Qt Creator.
3. Click **Test Connection**.

The key is never in the source code. If you save it in Settings, it is stored in your local data file:
- Linux: `~/.local/share/GwaCalculator/data.json`
- Windows: `%APPDATA%\GwaCalculator\data.json`

**Do not share that file.** The default model is `gemini-3.5-flash-lite` (free tier). Google renames models and changes free-tier limits often, so if you get "Model not found", change the model in Settings using the current list at <https://ai.google.dev/gemini-api/docs/models>.

## Project structure
| Path | Responsibility |
|---|---|
| `src/main.cpp` | Starts Qt, loads the stylesheet, shows the main window |
| `src/core/Subject.h` | Subject record and `weightedGrade()` |
| `src/core/GradingScale.*` | Scale presets and the only "which grade is better" logic |
| `src/core/Calculator.*` | Totals (recursive + iterative), GWA, goal evaluation, categories, impact ranking |
| `src/core/SubjectManager.*` | Add / edit / delete / find subjects |
| `src/core/Validation.*` | Input checks and number formatting |
| `src/core/AppData.h` | Everything the app saves |
| `src/storage/Storage.*` | Save/load JSON (`saveRecords`, `loadRecords`) |
| `src/ai/PromptBuilder.*` | Builds the Gemini prompt from computed data |
| `src/ai/GeminiClient.*` | HTTPS request, response parsing, error messages |
| `src/gui/MainWindow.*` | Sidebar navigation: Dashboard, Subjects, Settings |
| `src/gui/DashboardPage.*` | Stat cards, goal banner, and the AI feedback panel (`FeedbackPage.*`) |
| `src/gui/SubjectsPage.*`, `SettingsPage.*` | Subject table and form; goal, scale, and Gemini settings with tips |
| `resources/style.qss` | All colors and fonts |
| `tests/core_tests.cpp` | Assert-based tests of the core logic and prompt |

The core (`src/core`, `src/ai/PromptBuilder`) is plain standard C++ with no Qt, so it can be tested and explained on its own.

## Where each programming concept is used

| Concept | Where |
|---|---|
| `if` | `calculateGWA` (zero-units guard), `resolveApiKey` (env var set?), `deleteSubject` (index check) |
| `if-else` | `isBetterOrEqual` (scale direction), `goodness`, `SettingsPage::refresh` (key source) |
| `else-if` ladder | `performanceCategory`, `validateUnits`, `validateScale`, `GeminiClient::describeError` (HTTP status), `FeedbackPage::refresh` |
| Nested conditionals | `evaluateGoal` (achieved? → failing? → within margin?), `validateGrade` (out of range → below or above?), `loadRecords` (exists → valid JSON → valid fields) |
| `switch` | `presetScale`, `goalStatusText`, `statusColor` (dashboard), `describeError` (network error type) |
| `for` | `sumWeightedIterative`, `sumUnitsIterative`, building table rows, building the prompt, `parseFeedbackText` (name check) |
| `while` | `findSubject`, `trim`, `rankByImpact` (insertion sort), `loadRecords` (subjects array), joining Gemini response parts |
| `do-while` | `nextDefaultName` (try "Subject N" until unused) |
| Functions | Every file above. Each function has one job. |
| Recursion | `sumWeightedRecursive`, `sumUnitsRecursive` (used by `calculateGWA`) |

### Recursion vs. iteration
`sum(i) = weighted(subject[i]) + sum(i + 1)`, with base case `i == size → 0`. The recursive version reads like the Σ formula, so it is easy to check against the math. The `for` loop version does the same work without using one stack frame per subject. For very large lists the loop is the better choice. With the 5–15 subjects of a school term, both are instant. The app uses the recursive version, and `tests/core_tests.cpp` checks that both always give the same answer.

## Worked example (PH scale, target 1.75)
| Subject | Grade | Units | Weighted |
|---|---|---|---|
| Math | 1.50 | 3 | 4.50 |
| Programming | 1.25 | 3 | 3.75 |
| PE | 2.00 | 2 | 4.00 |
| English | 2.50 | 3 | 7.50 |
| **Total** | | **11** | **19.75** |

GWA = 19.75 ÷ 11 = **1.80**. On this scale lower is better, so the gap is 1.80 − 1.75 = 0.05. The "close" margin is 5% of the 1–5 range, which is 0.20. The gap is within the margin, so the status is **GOAL NOT YET ACHIEVED**. English is the top priority because 2.50 × 3 units costs the most.
