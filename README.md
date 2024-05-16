Консольный чат на C++
============

- [Фичи](#фичи)
- [Сборка проекта](#сборка-проекта)

![два клиента](https://raw.githubusercontent.com/EvgeniyLupanov1024/EvgeniyLupanov1024/main/projects_media/ConsoleChatOnCPP/console_chat_cpp_01.gif)

## Фичи

- Разделение на `Строку ввода` и `Окно сообщений`
- Сохранение имён участников чата
- Реализованно мультиплексирование через **epoll**
- Реализованно IPC между клиентом и терминалом для отображения сообщений посредством **FIFO**
- Логирование из нескольких потоков исполнения
- Уведомление клиентов о подключении и отключении пользователей в чате, а так же передача списка участников чата новому участнику

## Сборка проекта

Запустить `compile_all.sh` в корне проекта (под linux).

<img src="https://raw.githubusercontent.com/EvgeniyLupanov1024/EvgeniyLupanov1024/main/projects_media/ConsoleChatOnCPP/console_chat_cpp_wrungel.jpg" align="middle" alt="Врунгель" width="60%">
