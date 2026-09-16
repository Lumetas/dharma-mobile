# dharmactl - Control Utility

`dharmactl` - утилита для управления через командную строку. Предоставляет API для работы с тегами, окнами и событиями.

## Tag API (работа с тегами/workspace)

### tag.current
Получить номер текущего тега (1-based):
```bash
dharmactl tag.current
```

### tag.list
Получить список всех тегов:
```bash
dharmactl tag.list
```

### tag.list.count
Получить количество тегов:
```bash
dharmactl tag.list.count
```

### tag.moveTo
Переключиться на указанный тег:
```bash
dharmactl tag.moveTo <номер_тега>
```

---

## Window API (работа с окнами)

### window.list
Получить список всех окон (только ID):
```bash
dharmactl window.list
```

### window.list.count
Получить количество окон:
```bash
dharmactl window.list.count
```

### window.list.info
Получить подробную информацию о всех окнах:
```bash
dharmactl window.list.info
```
Вывод: `window_id|class|name|desktop`

### window.onTag.list
Получить список окон на указанном теге:
```bash
dharmactl window.onTag.list <номер_тега>
```
Или `all` для всех тегов:
```bash
dharmactl window.onTag.list all
```

### window.onTag.list.info
Получить подробную информацию об окнах на теге:
```bash
dharmactl window.onTag.list.info <номер_тега>
```

### window.onTag.list.count
Получить количество окон на теге:
```bash
dharmactl window.onTag.list.count <номер_тега>
```

### window.info
Получить полную информацию об окне:
```bash
dharmactl window.info <window_id>
```

### window.info.tag
Получить тег окна:
```bash
dharmactl window.info.tag <window_id>
```

### window.kill
Закрыть окно:
```bash
dharmactl window.kill <window_id>
```

### window.moveToTag
Переместить окно на указанный тег:
```bash
dharmactl window.moveToTag <window_id> <номер_тега>
```

---

## Event API (события)

API событий позволяет регистрировать скрипты, которые будут автоматически выполняться при наступлении определенных событий.

### event.register
Зарегистрировать скрипт для события:
```bash
dharmactl event.register <event_name> <path_to_script>
```

### event.unregister
Удалить регистрацию скрипта:
```bash
dharmactl event.unregister <event_name> <path_to_script>
```

### event.list
Показать все зарегистрированные обработчики событий:
```bash
dharmactl event.list
```

### event.help
Показать справку по Event API:
```bash
dharmactl event.help
```

---

## Доступные события

| Событие | Описание | Аргументы |
|---------|----------|-----------|
| `SwitchTag` | Переключение между тегами | `old_tag new_tag` |
| `ClientTag` | Окно перемещено на тег | `window_id old_tag new_tag` |
| `ClientFocus` | Окно получило фокус | `window_id tag` |
| `ClientUnmanage` | Окно закрыто | `window_id` |

---

## Примеры использования Event API

### Выполнение скрипта при переключении тегов

```bash
#!/bin/bash
# script.sh

EVENT=$1
OLD_TAG=$2
NEW_TAG=$3

```

Регистрация:
```bash
dharmactl event.register SwitchTag /path/to/script.sh
```

### Выполнение скрипта при фокусе окна

```bash
#!/bin/bash
# focus_handler.sh

EVENT=$1
WINDOW_ID=$2
TAG=$3

```

Регистрация:
```bash
dharmactl event.register ClientFocus /path/to/focus_handler.sh
```

### Несколько скриптов на одно событие

Можно зарегистрировать несколько скриптов на одно событие:
```bash
dharmactl event.register SwitchTag /path/to/script1.sh
dharmactl event.register SwitchTag /path/to/script2.sh
```

Все зарегистрированные скрипты будут выполнены асинхронно (с `&`).

### Удаление регистрации

```bash
dharmactl event.unregister SwitchTag /path/to/script.sh
```

---

## Технические детали

- События хранятся в памяти (в рамках сессии)
- При перезагрузке все зарегистрированные события сбрасываются
- Скрипты выполняются асинхронно с использованием `system()`
