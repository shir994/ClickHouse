### Для чего эта домашка

Цель этой домашки – понять что такое большой незнакомый проект и как с ним работать.
Этим она отличается от домашек, где вам нужно было реализовать относительно небольшие программы полностью самостоятельно.
В этой домашке придётся в первую очередь читать чужой код и разбираться в нём.
Приготовьтесь потратить пару дней на чтение документации, кода и другое первоначальное ознакомление с проектом – его сборку и запуск тестов.

В качестве прокта выбран ClickHouse.
[ClickHouse как сделать самую быструю распределённую аналитическую СУБД — Виктор Тарнавский](https://www.youtube.com/watch?v=Ho4_dQk7dAg)

### Постановка задачи

#### Ввести понятие сессии для HTTP интерфейса ClickHouse

В HTTP интерфейсе должна быть возможность указать параметры session_id, session_timeout, session_check.
session_id - произвольная строка.
session_timeout - время жизни сессии в секундах, после последнего действия.
session_check - если 1 - проверить, что сессия существует, и если нет - кинуть исключение.
Вводится настройка max_session_timeout (=3600) и default_session_timeout (=60).
Если пользователь попросил session_timeout > max_session_timeout, кидается исключение.
В сервере существует отображение user -> session_id -> Context - "хранилище сессий".
Соответствующий сессии Context используется в качестве session_context.
В нём могут создаваться временные таблицы и настройки уровня сессии.
Для каждого пользователя, сессии полностью независимы от сессий другого пользователя.
При каждом использовании, срок устаревания сессии продлевается (максимизируется) на указанный session_timeout.
Внутри сервера создаётся поток, который спит до момента устаревания следующей сессии или до момента обновления множества сессий, и в нужное время, очищает устаревшие сессии.

#### Примеры использования

```
# Сессии позволяют задавать сессионные настройки:

curl "http://localhost:8123/?session_id=test&session_timeout=300" --data "SET max_rows_to_read=1000000000"

curl "http://localhost:8123/?session_id=test&session_timeout=300" --data "SELECT * FROM system.settings WHERE name = 'max_rows_to_read'"

# Сессии позволяют создавать временные таблицы:

curl "http://localhost:8123/?session_id=test&session_timeout=300" --data "CREATE TEMPORARY TABLE t (x String)"

curl "http://localhost:8123/?session_id=test&session_timeout=300" --data "INSERT INTO t VALUES ('Hello'), ('World')"

curl "http://localhost:8123/?session_id=test&session_timeout=300" --data "SELECT * FROM t ORDER BY x"

# Работа с таймаутом сессии:

curl "http://localhost:8123/?session_id=test&session_timeout=5" --data "CREATE TEMPORARY TABLE t (x String)"
curl "http://localhost:8123/?session_id=test&session_timeout=5" --data "INSERT INTO t VALUES ('Hello'), ('World')"

sleep 4

curl "http://localhost:8123/?session_id=test&session_timeout=5" --data "SELECT * FROM t ORDER BY x"

sleep 4

curl "http://localhost:8123/?session_id=test&session_timeout=10" --data "SELECT * FROM t ORDER BY x"

sleep 9

curl "http://localhost:8123/?session_id=test&session_timeout=1" --data "SELECT * FROM t ORDER BY x"
```

### Прежде всего – много чтения

#### Стоит почитать документацию проекта

* Про архитектуру ClickHouse: [doc/developers/architecture.md](doc/developers/architecture.md). Как минимум, [раздел Server](doc/developers/architecture.md#server) – он про ту часть кода, которую нужно менять в домашке.
* Как собрать: [doc/build.md](doc/build.md).
* Как запустить тесты: [doc/developers/tests.md](doc/developers/tests.md).
* Хорошим тоном является соблюдение стиля проекта: [doc/developers/style_ru.md](doc/developers/style_ru.md).
* Очень полезно проиндексировать исходники в IDE, ctags, и т.д. Чтобы можно было быстро понять что делает тот или иной класс или функция.

#### Стоит почитать код файлов, которые предстоит модифицировать

* [dbms/src/Server/Server.cpp](dbms/src/Server/Server.cpp)
* [dbms/src/Server/HTTPHandler.cpp](dbms/src/Server/HTTPHandler.cpp). Обратите внимание на `reserved_param_names`, возможно вам придётся модифицировать этот список.
* [dbms/src/Interpreters/Context.h](dbms/src/Interpreters/Context.h)
* [dbms/src/Interpreters/Context.cpp](dbms/src/Interpreters/Context.cpp). В `Context`, который описывает контекст сессии, есть поле `shared` типа `ContextShared`. В `ContextShared` хранится общая информация про все сессии. Обратите внимание, что для того чтобы работать с `shared` функции `Context` захватывают лок: `auto lock = getLock();`. Ещё из этого файла, как и из Server.cpp, можно понять, как работать с конфигурационными параметрами.
* [dbms/src/Core/ErrorCodes.cpp](dbms/src/Core/ErrorCodes.cpp)

#### Стоит почитать файлы, в которых есть аналогичные требуемым доработкам вещи

Главное – не копируйте бездумно то, что кажется подходящим.
В ClickHouse, как и в любом достаточно объёмном сложном коде, есть ошибки.
Очень важно понимание, что код делает, почему это то что нужно и почему это корректно.
* [dbms/src/Server/TCPHandler.cpp](dbms/src/Server/TCPHandler.cpp). Обратите внимание на инициализацию контекста в TCP-сессиях
```
void TCPHandler::runImpl()
{
    connection_context = *server.global_context;
    connection_context.setSessionContext(connection_context);
    ...
```
* [dbms/src/Server/ConfigReloader.h](dbms/src/Server/ConfigReloader.h)
* [dbms/src/Server/ConfigReloader.h](dbms/src/Server/ConfigReloader.cpp)

### После чтения

#### Нужно научиться собирать

Я ставил себе виртуалку с Ubuntu-17.04.
В этом релизе Boost и Poco тех версий, которые требуются для сборки ClickHouse – 1.62 и 1.6.1.
Без них ClickHouse будет использовать лежащие в нём contrib/libboost и contrib/libpoco.
И будет собирать их.
В итоге, сборка отнимет больше времени и места на диске.
Даже с системными Boost и Poco сборка у меня отняла около 80 минут в пересчёте на однопроцессорную машину.
Но совершенно не обязательно идти тем же путём.

После инсталляции системы я ставил git, cmake, boost, poco и остальные зависимости, которые упомянуты в [doc/build.md](doc/build.md):
```
sudo apt-get install git cmake
sudo apt-get install libboost-all-dev libpoco-dev
sudo apt-get install libicu-dev libreadline-dev libmysqlclient-dev libssl-dev unixodbc-dev
```

Дальше нужно скачать ClickHouse и запустить сборку:
```
git clone https://github.com/sergey-v-galtsev/ClickHouse
cd ClickHouse
mkdir build
cd build
cmake -DUSE_INTERNAL_POCO_LIBRARY=0 -DUSE_INTERNAL_BOOST_LIBRARY=0 ..
make -j2
```

В дальнейшем, тестам понадобится `clickhouse-client`.
Это просто линк на собравшийся бинарник самого сервера `clickhouse`.
Для его создания нужно в той же директории `ClickHouse/build` выполнить команду
```
ln -s clickhouse dbms/src/Server/clickhouse-client
```

#### Нужно научиться запусткать тесты

Тесты проекта стоит запускать чтобы проверить, что ваши изменения ничего не сломали.
Возможно, не все тесты пройдут успешно до ваших изменений.
Главное, чтобы после изменений не ноявились новые падающие тесты.

Сперва нужно запустить сервер.
Проще всего сделать это в дополнительной консоли.
Серверу понадобится файл с конфигурацией, он находится в `ClickHouse/dbms/src/Server/config.xml`.
Можно, например, перейти в директорию, где собрался бинарник сервера – `ClickHouse/build/dbms/src/Server/`.
И запустить его:
```
cd ClickHouse/build/dbms/src/Server
./clickhouse --config-file ../../../../dbms/src/Server/config.xml
```

Для запуска тестов нужно в другой консоли перейти в директорию `ClickHouse/dbms/tests`, указать в `$PATH` путь к `clickhouse-client` и запустить `clickhouse-test` с опцией `--no-shard` – у нас не поднята схема с шардированием сервера.
```
cd ClickHouse/dbms/tests
PATH=$PATH:../../build/dbms/src/Server ./clickhouse-test --no-shard
```

### Написание кода

Для решения задачи понадобится хранилище контекстов сессий – `Context` – с эффективным поиском по имени сессии.
Также это хранилище должно эффективно находить сессии с истёкшим тайм-аутом.
То есть, скорее всего придётся завести два связанных контейнера.
Расположить их можно в `ContextShared`.
Если вы хотите применить свои знания по алгоритмам и стуктурам данных, все нужные операции можно реализовать за O(1).

Подумайте над всеми возможными ситуациями, в которые может попасть сессия.
Прежде всего, сессии разных пользователей не должны никак влиять друг на друга.
А указание сессии не должно, например, приводить к отсутствию аутентификации.
Также, у одного пользователя не должно быть разных сессий с одним `session_id`.
Или двух параллельных HTTP-запросов к одной и той же сессии.
Не должно возникнуть проблем, если пользователь решит создать новую сессию с тем же `session_id`, сразу по истечению времени жизни предыдущей.

Все случаи, которые вы придумаете, полезно сразу описать в виде тестов.

Подумайте над временем жизни сессии, как его измерять.
Какая точность нужна, какой источник времени подойдёт. 
Могут ли возникнуть проблеммы из-за просроченных тайм-аутов, преждевременных пробуждений, перевода часов, дребезга и т.д.
Подумайте, от какого момента нужно отмерять тайм-аут.
И что делать с ним, если при исполнении запроса вылетело исключение.

Подумайте, где и как нужно создавать поток вычистки сессий, как коммуницировать с ним.
Например, как сообщить ему о необходимости завершиться.
Подобные потоки в ClickHouse уже создаются, можно подсмотреть как это делается.

### Логи

Они в ClickHouse очень хорошие.
Многие ошибки будут понятны просто из сообщений в логах.
При отладке может сильно помочь добавление печати в лог своих сообщений.

### Ориентировочный объем кода

У меня получилось вот так:
```
Core/ErrorCodes.cpp         |    3
Interpreters/Context.cpp    |  156 ++++++++++++++++++++++++++++++++++++++++++++
Interpreters/Context.h      |   35 +++++++++
Server/HTTPHandler.cpp      |   77 +++++++++++++++++----
Server/Server.cpp           |    2
```
