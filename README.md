# LizardW2Hack

Проект для модификации Unity IL2CPP игр. Из коробки ориентирован на Humans Are Not That Against LizardWoman 2, но **список хуков задаётся текстовым конфигом** и может быть переписан под другую игру.

## Оглавление

- [Описание](#описание)
- [Конфигурация хуков `LizardHooks.txt`](#конфигурация-хуков-lizardhookstxt)
- [Консоль, меню, логотип](#консоль-меню-логотип)
- [Как это работает](#как-это-работает)
- [Сборка](#сборка)
- [Использование](#использование)
- [Пример `doorstop_config.ini`](#пример-doorstop_configini)
- [Благодарности](#благодарности)

## Описание

Нативная DLL (`Lizard.dll`) загружается в процесс игры через **Unity Doorstop** и managed-bootstrap. Перехват методов делается через **MinHook**; оффсеты методов находятся **резолвером Il2Cpp** (метаданные `global-metadata.dat` и `GameAssembly.dll` рядом с игрой).

Поведение каждого хука (какой метод, что логировать, что возвращать, включён ли при старте) задаётся в **`LizardHooks.txt`** в формате, похожем на объявление функции на C++.

## Конфигурация хуков `LizardHooks.txt`

### Где лежит файл

- По умолчанию: **`LizardHooks.txt` в каталоге исполняемого файла игры** (рядом с `.exe`, `GetModuleHandle(NULL)`), а не рядом с `Lizard.dll`.
- Переопределение пути: переменная окружения **`LIZARD_HOOKS_FILE`** — полный путь к `.txt` (wide path, как обычный путь в Windows).

Файл читается как **UTF-8** (поддерживается BOM). Строки `// ...` считаются комментариями и вырезаются перед разбором.

### Лимиты

- До **32** хуков (слотов). Порядок в файле = порядок слотов.

### Формат блока хука

Каждый хук — блок вида **`ИмяКакВIl2Cpp$$(параметры) { тело }`**:

- Слева от **`$$`** — полное имя класса в стиле Il2Cpp: `Класс` или `Пространство.Класс`.
- Справа от **`$$`** — имя метода.
- В скобках — список параметров для **выбора сигнатуры детура** (см. ниже). Достаточно перечислить «как в примере»: обычно это указатели `DWORD*` под видом `__this`, `method`, аргументы сцен и т.д.
- В теле задаются имя для меню, стартовое включение, число параметров для резолвера, лог и возвращаемое значение.

**Сигнатура детура** выводится по **числу `DWORD*`** в списке параметров в скобках:

| `DWORD*` в скобках | Сценарий |
|--------------------|----------|
| 1 | статический метод (`BoolMethod`) |
| 2 | instance + `method` (`BoolThisMethod`) |
| 3 | три указателя (часто сцены) (`BoolThisScenes`) |
| 4 | четыре указателя (`BoolThis4`) |
| 5 | пять указателей (`BoolThis5`) |

Поле **`param N`** — число параметров **метода Il2Cpp** для `FindMethodOffsetByClassAndMethod` (как в исходнике игры, без учёта «this» в reflection API так, как ожидает резолвер в проекте). Его нужно подобрать под конкретный метод (часто совпадает с количеством «логических» аргументов в игре).

### Поля внутри `{ ... }`

| Поле | Пример | Назначение |
|------|--------|------------|
| `name "..."` | `name "Infinite coins";` | Подпись в консольном меню (UTF-8). Если нет — в меню показывается полное имя `Class$$Method`. |
| `enabled` / `enabled_on_start` / `start_enabled` | `enabled true;` | Включён ли хук **при старте** (`true` / `false`). |
| `param` или `param_count` | `param 2;` | Явное число параметров метода для резолвера (перекрывает значение по умолчанию от сигнатуры). |
| `printf("...");` | `printf("hook\\n");` | Строка при срабатывании, когда хук **включён** в меню. |
| `return true;` / `return false;` | `return true;` | Возвращаемое значение при вызове детура, если хук **включён**. |

Если хук **выключен** в меню, вызывается **оригинальная** функция игры (без подмены `return` и без `printf` из конфига).

### Пример с пояснениями

Ниже один блок «как шаблон» — что означает каждая часть.

```text
// Полное имя метода Il2Cpp: Namespace.Class$$MethodName
// В скобках — список параметров (для выбора типа детура по числу DWORD*)
Player$$isSceneOpened(DWORD* __this, DWORD* novelId, DWORD* scenario, DWORD* method) {
    name "Open any scene";   // Текст в меню
    enabled true;            // При старте игры хук включён
    param 2;                 // Il2Cpp: два параметра у метода (например novelId + scenario)
    printf("scene hook\n");  // Лог при срабатывании, если хук включён
    return true;             // Если хук включён — вернуть true из метода
}
```

Готовый файл с несколькими хуками под LizardWoman 2 см. **`LizardHooks.txt.example`** в корне репозитория — скопируйте его как **`LizardHooks.txt`** рядом с `.exe` игры.

## Консоль, меню, логотип

После инициализации выводится **баннер** (логотип UTF-8) и при наличии хуков — **консольное меню**:

- **↑ / ↓** — выбор пункта;
- **Enter** — переключить хук **ON/OFF** (в меню: зелёный — вкл., красный — выкл.);
- выбранная строка помечается **`*`**;
- **Esc** — выход из цикла меню (игра и хуки продолжают работать).

Логотип брайля выводится через **`WriteConsoleW`**; консоль пытается переключить шрифт на вариант с глифами Braille (Cascadia Mono и др.). Если брайль отображается квадратиками, а при копировании текста в буфер всё корректно — смените шрифт окна консоли вручную или используйте **Windows Terminal**.

Сборка исходников с Unicode-литералами: в проекте включается **`/utf-8`** (см. `Lizard.vcxproj`), иначе строки в `u8` могут собираться неверно на системах с не-UTF-8 кодовой страницей по умолчанию.

## Как это работает

1. **Doorstop** подгружает managed bootstrap, тот загружает **`Lizard.dll`**.
2. В отдельном потоке выделяется консоль, читается **`LizardHooks.txt`**, для каждого слота резолвится RVA/оффсет метода через `global-metadata` и **`GameAssembly.dll`**.
3. Ставятся **MinHook**-детуры; при **включённом** пункте меню выполняется логика из конфига; при **выключенном** — вызывается оригинал.

## Сборка

Вся сборка — скриптом **`build.ps1`**: native DLL, Doorstop bootstrap, копирование в **`.\Release`**, подстановка `coreclr_path` / `corlib_dir` в `doorstop_config.ini`.

### Зависимости

- Visual Studio (MSBuild)
- Git
- vcpkg с пакетом **minhook** (статическая линковка x64)

### Установка зависимостей

1. Установите vcpkg:

   ```text
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg integrate install
   ```

2. Установите minhook:

   ```text
   .\vcpkg install minhook:x64-windows-static
   ```

### Запуск сборки

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1
```

С параметрами:

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Configuration Release -Platform x64
```

Явные пути CoreCLR (если не нужен авто-поиск .NET 6):

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 `
  -CoreClrPath "C:\Program Files\dotnet\shared\Microsoft.NETCore.App\6.0.xx\coreclr.dll" `
  -CorlibDir "C:\Program Files\dotnet\shared\Microsoft.NETCore.App\6.0.xx"
```

После сборки в **`.\Release`**:

- `Lizard.dll`
- `Doorstop.dll`
- `doorstop_config.ini`
- `winhttp.dll` (из `res\winhttp.dll`)

Секция **`[Il2Cpp]`** в `.\Release\doorstop_config.ini` заполняется скриптом (`coreclr_path`, `corlib_dir`).

Если MSBuild не в PATH, используйте **Developer PowerShell for Visual Studio** или укажите полный путь к `MSBuild.exe`.

## Использование

1. Скопируйте в **корень игры** (рядом с исполняемым файлом) содержимое **`.\Release`**:

   - `winhttp.dll`
   - `doorstop_config.ini`
   - `Doorstop.dll`
   - `Lizard.dll`

2. Положите **`LizardHooks.txt`** рядом с **`.exe` игры`** (или задайте **`LIZARD_HOOKS_FILE`** на полный путь к файлу).

3. Запустите игру. При необходимости отредактируйте хуки и перезапустите игру (горячая перезагрузка конфига в рантайме в текущей версии не описана).

## Пример `doorstop_config.ini`

Минимальный ориентир (секция `[Il2Cpp]` после сборки заполняется скриптом):

```ini
[General]
enabled=true
target_assembly=Doorstop.dll
redirect_output_log=false
boot_config_override=
ignore_disable_switch=false

[UnityMono]
dll_search_path_override=
debug_enabled=false
debug_address=127.0.0.1:10000
debug_suspend=false

[Il2Cpp]
coreclr_path=
corlib_dir=
```

## Благодарности

Этот проект использует идеи и открытые инструменты:

- [Il2CppDumper](https://github.com/Perfare/Il2CppDumper)
- [web-global-metadata-parser](https://github.com/t-wy/web-global-metadata-parser)
- [frida-il2cpp-bridge](https://github.com/vfsfitvnm/frida-il2cpp-bridge)
- [UnityDoorstop](https://github.com/NeighTools/UnityDoorstop)
