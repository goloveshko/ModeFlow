# Changelog - ModeFlow v{VERSION} (Release Candidate)

### 🇬🇧 English
Welcome to the Release Candidate (**v{VERSION}**) of **ModeFlow**! This release brings a massive architectural rewrite, visual polish, and performance optimization to ensure a native Windows 11 Fluent experience and lightweight background execution.

#### ✨ Features & UI/UX Upgrades:
* **Fluent Cards & Spacing:** Replaced legacy group box borders with modern, rounded, semi-transparent Fluent Cards (Mica layering). Enforced spacious minimal sizes for all dialogs to support high-DPI scaling.
* **Custom Themed File Dialogs:** All file pickers are now built-in Qt dialogs fully styled under the active theme (Light/Dark) with native Mica glass, removing unstyled white dialogs and console exceptions.
* **Opt-in Confirmation Prompts:** Added a new "Confirm before applying profiles" settings checkbox, allowing users to toggle confirmation popups for tray and context menu switches, while keeping global hotkeys instant.
* **Uncluttered System Tray:** Renamed the main tray menu item to "Open ModeFlow" (or "ModeFlow" in bold) and shortened the sub-menu name to "Profiles".
* **Active Profile Visual Feedback:** Added a subtle status dot next to the active profile name, ensuring clear feedback of the active system state.

#### ⚙️ Performance & Architecture:
* **Fast Log Viewer:** Integrated high-performance `QSyntaxHighlighter` (lazy loading) and zero-allocation `QStringView` parsing, completely eliminating main-thread HTML freezing.
* **Delayed Launch Abort Protection:** Switched to managed, cancelable timers. Switching away from a profile now instantly aborts any pending delayed application launches, preventing runaway background processes.
* **Smart Hotkey Diffing:** Rebuilt `HotkeyManager` to only update/re-register modified keys, eliminating redundant Win32 hook mutations and log spam.
* **Zero-Flicker Window Restoration:** Fixed Windows 11 DWM Mica "creeping" geometry bugs and startup resizing flickering.
* **DIP & Decoupling:** Extracted profile icons, details forms, and file transactions into isolated, lightweight presenter classes.
* **Modular CMake:** Restructured tests into a separate folder, compiling all common sources into a shared `ModeFlowCore` Object Library (reducing compilation times by 50%).

---

### 🇷🇺 Русский
Добро пожаловать в стабильный предрелиз (**v{VERSION}**) **ModeFlow**! Этот релиз представляет собой масштабный цикл рефакторинга архитектуры, визуальной полировки и оптимизации производительности, обеспечивающий нативный Fluent-дизайн Windows 11 и легкую работу приложения в фоне.

#### ✨ Новые возможности и интерфейс:
* **Fluent-карточки (Windows 11):** Заменили устаревшие рамки групп параметров на закругленные полупрозрачные карточки (Fluent Cards). Задали просторные лимиты по ширине окон для поддержки масштабирования High-DPI.
* **Стилизованные проводники файлов:** Диалоги импорта/экспорта теперь встроены в тему приложения (Light/Dark) с нативным Mica-эффектом, убирая белые системные окна и ошибки COM.
* **Опциональные подтверждения:** Добавлен чекбокс «Запрашивать подтверждение перед переключением» в настройки. Контроль подтверждений централизован, а горячие клавиши по-прежнему работают мгновенно.
* **Лаконичный системный трей:** Главный пункт трей-меню заменен на лаконичное «Открыть ModeFlow» (Open ModeFlow), а заголовок подменю профилей сокращен до простого «Профили» (Profiles).
* **Маркер активности:** Добавлена аккуратная Fluent-точка активности слева от названия текущего профиля, обеспечивая четкий отклик о реальном состоянии системы.

#### ⚙️ Оптимизации и Архитектура:
* **Быстрый лог-вьювер:** Переведен на `QSyntaxHighlighter` с ленивой загрузкой и zero-allocation парсинг `QStringView` (загрузка логов без зависаний).
* **Безопасный отложенный запуск:** Уход с профиля теперь мгновенно отменяет запланированные отложенные запуски программ, предотвращая «беглые» фоновые процессы.
* **Умный дифф хоткеев:** Перерегистрация затрагивает только измененные или новые клавиши, исключая лишнюю нагрузку на Windows API и спам в консоли.
* **Устранение фликкеров окон:** Устранен баг сползания окон DWM Mica при перезапусках, окно восстанавливает размеры мгновенно и без мерцания.
* **Архитектурная чистота:** Вынесли логику иконок, формы параметров и диалогов файлов в изолированные классы-контроллеры.
* **Модульная CMake-сборка:** Перенесли тесты в отдельную папку и упаковали ядро в общую объектную библиотеку `ModeFlowCore` (сокращение времени компиляции в 2 раза).