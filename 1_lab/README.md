# Лабораторная работа: Парадигмы программирования с MongoDB

## Описание
Данная лабораторная работа демонстрирует три различные парадигмы программирования для работы с MongoDB:
- **Императивная** (`imperativ/`)
- **Объектно-ориентированная** (`oop/`)
- **Процедурная** (`procedur/`)

## Требования
- Docker
- CMake 3.10+
- C++17 компилятор
- MongoDB C++ Driver

## Быстрый старт

### 1. Запуск MongoDB с Docker

#### Вариант 1: Docker Compose (рекомендуется)
```bash
# Запуск MongoDB с автоматической инициализацией данных
docker-compose up -d

# Проверка статуса
docker-compose ps
```

#### Вариант 2: Docker команды
```bash
# Запуск контейнера
docker run -d --name mongodb-lab -p 27017:27017 -e MONGO_INITDB_ROOT_USERNAME=admin -e MONGO_INITDB_ROOT_PASSWORD=password mongo:latest

# Проверка статуса
docker ps | grep mongodb-lab
```

### 2. Проверка подключения к MongoDB
```bash
# Подключение к MongoDB shell
docker exec -it mongodb-lab mongosh

# В MongoDB shell выполните:
use university
db.students.find().pretty()
```

### 3. Сборка и запуск программ

#### Императивная парадигма
```bash
cd 1_task/imperativ
mkdir build && cd build
cmake ..
make
./main1  # Поиск студентов с возрастом < 19
./main2  # Поиск студентов с фамилией на "А"
```

#### Объектно-ориентированная парадигма
```bash
cd 1_task/oop
mkdir build && cd build
cmake ..
make
./main1  # Поиск студентов с возрастом < 19
./main2  # Поиск студентов с фамилией на "А"
```

#### Процедурная парадигма
```bash
cd 1_task/procedur
mkdir build && cd build
cmake ..
make
./main1  # Поиск студентов с возрастом < 19
./main2  # Поиск студентов с фамилией на "А"
```

## Структура данных

База данных `university` содержит коллекцию `students` с полями:
- `Имя` - имя студента
- `Фамилия` - фамилия студента  
- `Возраст` - возраст студента
- `Группа` - учебная группа
- `Средний_балл` - средний балл успеваемости

## Тестовые запросы

### 1. Студенты с возрастом меньше 19
```javascript
db.students.find({"Возраст": {$lt: 19}})
```

### 2. Студенты с фамилией на "А"
```javascript
db.students.find({"Фамилия": {$regex: "^А"}})
```

## Остановка и очистка

```bash
# Остановка контейнера
docker-compose down

# Или для Docker команды:
docker stop mongodb-lab
docker rm mongodb-lab

# Удаление данных (опционально)
docker volume rm 1_lab_mongodb_data
```

## Устранение неполадок

### MongoDB не запускается
```bash
# Проверка логов
docker logs mongodb-lab

# Проверка занятости порта 27017
netstat -tulpn | grep 27017
```

### Ошибки компиляции
- Убедитесь, что MongoDB C++ Driver установлен
- Проверьте пути в CMakeLists.txt
- Убедитесь, что используется C++17

### Проблемы с подключением
- Проверьте, что контейнер запущен: `docker ps`
- Убедитесь, что порт 27017 не занят другим процессом
- Проверьте логи MongoDB: `docker logs mongodb-lab`
