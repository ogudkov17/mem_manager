### Memory Manager

Задача: 

* Необходимо разработать библиотеку выделения памяти под сетевые пакеты (буфер сетевых данных). Пакеты могут быть различного размера, но допускается задать предельный размер пакета (от 64 байт до 9000) байт при инициализации библиотеки.

* Запрос памяти происходит попакетно (по 1 шт) из различных потоков в произвольные моменты времени

* Библиотека должна быть потокобезопасной и покрытой базовым набором unit-тестов (один-два рабочих сценария)

* Язык программирования - C/C++

### Список необходимого ПО

build-essential, googletest

### Реализация

Класс MemoryManager

Принимает буфер памяти для использования в качестве кучи;
Выделение/освобождение памяти осуществляется методами allocate/deallocate;

Для выделения и управления памятью используется структура MemBlock;
Двунаправленный список блоков MemBlock формирует цепочку занятых и свободных блоков;
Свойство free указывает занят или свободен блок в цепочке;
При возвращении блока функцией deallocate, он объединяется с соседними свободными.

Запросы памяти производятся функцией allocate, аргументом которой является количество запрашиваемой памяти для пакета;

Для безопасности потоков используется мьютекс, блокирующий доступ других потоков при операциях с памятью.

Unit-тесты реализованы для следующих операций:
* Корректное выделение памяти
* Выделение памяти сверх существующей памяти
* Корректное освобождение памяти (в качестве аргумента передается выделенный блок)
* Некорректное освобождение памяти (в качестве аргумента передается указатель не на выделенный блок)
* Выделение двух участков памяти и освобождение памяти в различном порядке
* Выделение двух участков памяти и освобождение памяти в последовательном порядке
* Одновременный запуск определенного количества потоков для проверки работы в многопоточном режиме.
