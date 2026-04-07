# LizardW2Hack

Проект для модификации Unity il2cpp игр.
В частности для Humans Are Not That Against LizardWoman 2

## Как это работает

DLL файл предназначен для инъекции в процесс игры с помощью инструментов вроде Process Hacker 2. Использует MinHook для перехвата адресов программы, рассчитывает оффсеты с помощью паттернов в памяти, затем ожидает события. При наступлении события MinHook получает параметры, останавливает выполнение и запускает новую версию с измененными параметрами (переопределение).

## Сборка из командной строки

### Зависимости

- Visual Studio (для MSBuild)
- Git
- vcpkg

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

### Сборка

Сборка C++ DLL:
   ```
   "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" Lizard.sln /p:Configuration=Release /p:Platform=x64
   ```

   *Примечание: Если Visual Studio установлена в другом месте, найдите путь к MSBuild.exe или используйте Developer Command Prompt for Visual Studio, где MSBuild доступен по умолчанию.*

### Использование

Инъектируйте полученную DLL в процесс игры.
