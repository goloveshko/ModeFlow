# ModeFlow — Developer & Release Guide

This document defines the standard workflow for making changes, compiling, translating, and publishing releases of the **ModeFlow** utility.

---

## 🛠️ 1. Git Remotes Configuration

To prevent confusion between the local debugging server (Forgejo) and the public production repository (GitHub), the standard `origin` remote has been split into explicit, self-documenting targets.

Verify your remotes configuration by running:
```bash
git remote -v
```

If needed, set them up using the following commands:
```bash
# Register Gitea/Forgejo as the local debug environment
git remote add forgejo http://localhost:3000/goloveshko/ModeFlow.git

# Register official GitHub as the production environment
git remote add github https://github.com/goloveshko/ModeFlow.git
```

---

## 💻 2. Daily Development Cycle

Follow this strict cycle before committing any changes to ensure the codebase remains stable and compilable.

### Step 1: Format Code
Before compiling or committing, run `clang-format` on all modified source files. Thanks to our `.clang-format` categories, this will automatically regroup and sort all your `#include` directives into a clean, unified standard:
```bash
scripts\build.bat --format
```
*Note: Every time you run the build, CMake automatically copies the active configuration's `compile_commands.json` directly to the project root, keeping your LSP (clangd in VS Code/Zed) 100% synchronized and error-free.*

### Step 2: Build & Verify
Always run a test build using the default generator (Ninja is recommended for speed) to check for compile errors or warnings. Due to our shared `ModeFlowCore` Object Library configuration, compiling is now up to 50% faster as files are built strictly once:
```bash
scripts\build.bat --ninja
```

### Step 3: Run Tests
Verify that the full suite of unit tests passes successfully before committing (running the build script with `--ninja` automatically compiles and executes these):
```bash
build\bin\Debug\ModeFlowTests.exe
```

---

## 📝 3. Commit Workflow (Conventional Commits)

Commit your changes once a logical task is complete. Group your commits using the following prefixes:

*   `feat(<scope>):` — A new user-facing feature (e.g., `feat(ui): add inline delete buttons`).
*   `fix(<scope>):` — A bug fix (e.g., `fix(audio): prevent background thread crash`).
*   `refactor(<scope>):` — Code restructuring without altering behavior (e.g., `refactor(core): decouple list widget from main window`).
*   `docs(<scope>):` — Updates to documentation files (e.g., `docs(git): add release guide`).

*Never use "git add ." or "git add -A" as it can accidentally stage local build folders, compile databases, or other untracked developer artifacts. Stage only the specific C++ or QSS source files you edited.*

---

## 🌐 4. Internationalization (i18n) Pipeline

All user-facing strings must be wrapped in `tr()`. If you modify, add, or delete translatable strings:

1.  **Extract new strings** into the translation source file (`ModeFlow_ru_RU.ts`):
    ```bash
    scripts\build_lupdate.bat
    ```
2.  **Open the `.ts` file** using **Qt Linguist** (`i18n/ModeFlow_ru_RU.ts`) and translate the new entries.
3.  **Compile the translated strings** into binary `.qm` files before packaging:
    ```bash
    scripts\build_lrelease.bat
    ```

---

## 📦 5. Release & Deployment Pipeline

Follow these steps to package and publish a new version of ModeFlow.

### Step 1: Version Bump
Update the version numbers in `src/utils/VersionInfo.h` (increment MAJOR for major features, MINOR for features/pre-releases, and PATCH for bug fixes):
```cpp
#define APP_VERSION_MAJOR 0
#define APP_VERSION_MINOR 9  // Set to 9 for Release Candidate
#define APP_VERSION_PATCH 0  // Increment this for bug fixes
```

### Step 2: Write Release Notes
Open `metadata/changelog.md` in the metadata directory. Write your release notes in Markdown under the following structure. Keep the `{VERSION}` placeholder intact; the build script will replace it automatically:
```markdown
# Changelog - ModeFlow v{VERSION}

### 🇬🇧 English
* **UI:** Streamlined sidebar layouts.

---

### 🇷🇺 Русский
* **Интерфейс:** Оптимизированы отступы боковой панели.
```

### Step 3: Package Release
Run the build script with the `--package` flag. This will compile the release binary, compress the artifacts, calculate the SHA-256 hash, and dynamically generate `update.json` inside the metadata folder:
```bash
scripts\build.bat --release --static --ninja --package
```

Verify the generated output:
*   `build\artifacts\ModeFlow-vX.Y.Z-win-x64.zip` — Portable archive.
*   `build\artifacts\ModeFlow-vX.Y.Z-win-x64.zip.sha256` — Lowercase SHA-256 checksum.
*   `metadata\update.json` — Automatically populated update manifest (written to metadata directory).

### Step 4: Publish Release
1.  **Commit and Push metadata:**
    ```bash
    git add metadata/update.json metadata/changelog.md src/utils/VersionInfo.h
    git commit -m "bump: release version X.Y.Z"
    git push <remote_name> main
    ```
2.  **Upload Assets to Gitea/GitHub:**
    *   Open your Git host in a web browser.
    *   Create a new tag/release matching `vX.Y.Z`.
    *   Open `build\artifacts\temp_changelog.md` and copy its pre-rendered contents into the release description box.
    *   Drag and drop the generated `.zip` and `.sha256` files into the assets attachment area.
    *   Click **Publish**.
```
