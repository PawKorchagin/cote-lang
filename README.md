in progress

структура проекта:

в bin main.cpp песочница и можно позапускать что хотите

в lib пишем парсер

в tests/sources создаем тестовые файлы .ct

# Перед запуском (из корня)

```
cmake .
```

# Запуск (из корня)

запуск main.cpp:
```
cmake --build . && ./bin/crypt
```

запуск тестов:
```
cmake --build . --target parser_test && ctest -V
```