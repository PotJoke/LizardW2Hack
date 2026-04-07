# LizardW2Hack

Проект для модификации Unity il2cpp игр.
В частности для Humans Are Not That Against LizardWoman 2

## Оглавление

- [Описание](#описание)
- [Как это работает](#как-это-работает)
- [Сборка](#сборка)
  - [Зависимости](#зависимости)
  - [Установка зависимостей](#установка-зависимостей)
  - [Запуск сборки](#запуск-сборки)
- [Использование](#использование)
- [Благодарности](#благодарности)

## Описание

Этот проект представляет собой DLL-модификацию для Unity игры, использующей Il2Cpp. Основная цель - обход различных игровых ограничений путем перехвата и модификации ключевых функций игры.

### Что делает модификация

- **Обход проверки монет**: Функция `GameController$$checkCoins` всегда возвращает `true`, позволяя тратить монеты без проверки баланса.
- **Открытие всех сцен**: Функции `Player$$isSceneOpened`, `Controllers.StoryStateUtilities$$CheckOneOfScenariosUnlocked` и `FStoryModel$$CheckSetOfScenariosUnlocked` всегда возвращают `true`.
- **Завершение уровней**: Функция `YMatchThree.Core.LevelGameplay$$IsLevelComplete` всегда возвращает `true`.
- **Отключение стори-мода**: Функции `GameController$$GetStoryModeEnabled` и `CustomStoryFunctions$$StoryModeEnabled` всегда возвращают `false`.

Модификация использует MinHook для перехвата функций в `GameAssembly.dll`, а также собственный резолвер Il2Cpp для автоматического определения оффсетов методов на основе метаданных игры.

## Как это работает

Проект использует UnityDoorstop для автоматической загрузки модификации в процесс игры при ее запуске. DLL перехватывает адреса функций в GameAssembly.dll с помощью MinHook, рассчитывает оффсеты на основе Il2Cpp метаданных и модифицирует поведение ключевых функций для обхода игровых ограничений.

## Сборка

Вся сборка осуществляется с помощью скрипта `build.ps1`. Скрипт автоматически собирает native DLL и managed bootstrap, устанавливает необходимые зависимости и подготавливает артефакты.

### Зависимости

- Visual Studio (для MSBuild)
- Git
- vcpkg (с установленным minhook)

### Установка зависимостей

1. Установите vcpkg:
   ```
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg integrate install
   ```

2. Установите minhook:
   ```
   .\vcpkg install minhook:x64-windows-static
   ```

### Запуск сборки

Запустите скрипт сборки:
   ```
   powershell -ExecutionPolicy Bypass -File .\build.ps1
   ```

Параметры:
   ```
   powershell -ExecutionPolicy Bypass -File .\build.ps1 -Configuration Debug -Platform x64
   ```

Параметры для явной установки IL2CPP CoreCLR (если не хотите автоопределение):
   ```
   powershell -ExecutionPolicy Bypass -File .\build.ps1 -CoreClrPath "C:\Program Files\dotnet\shared\Microsoft.NETCore.App\6.0.xx\coreclr.dll" -CorlibDir "C:\Program Files\dotnet\shared\Microsoft.NETCore.App\6.0.xx"
   ```

После сборки скрипт автоматически складывает артефакты в `.\Release` (создаст папку при необходимости):

- `Lizard.dll`
- `Doorstop.dll`
- `doorstop_config.ini`
- `winhttp.dll` (из `res\winhttp.dll`)

Секция `[Il2Cpp]` в `.\Release\doorstop_config.ini` заполняется автоматически:

- `coreclr_path`
- `corlib_dir`

   *Примечание: Если Visual Studio установлена в другом месте, найдите путь к MSBuild.exe или используйте Developer Command Prompt for Visual Studio, где MSBuild доступен по умолчанию.*

### Использование

После размещения файлов в папке игры просто запустите игру. Модификация загрузится автоматически через UnityDoorstop и применит изменения.

В корень игры (рядом с `Game.exe`) положите файлы из папки `.\Release`:

- `winhttp.dll` (из релиза UnityDoorstop, x64)
- `doorstop_config.ini`
- `Doorstop.dll`
- `Lizard.dll`

### Пример `doorstop_config.ini`

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

Этот проект был создан с использованием следующих открытых репозиториев:

- [Il2CppDumper](https://github.com/Perfare/Il2CppDumper) - Инструмент для дампа Il2Cpp метаданных
- [web-global-metadata-parser](https://github.com/t-wy/web-global-metadata-parser) - Парсер глобальных метаданных
- [frida-il2cpp-bridge](https://github.com/vfsfitvnm/frida-il2cpp-bridge) - Мост для Frida и Il2Cpp
- [UnityDoorstop](https://github.com/NeighTools/UnityDoorstop) - Инструмент для загрузки DLL в Unity игры
