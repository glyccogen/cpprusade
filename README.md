# c++rusade

Набор небольших C++-задач.

## Smart Pointers
* `compressed-pair` --- реализовать пару объектов с empty base optimization, чтобы
  пустые типы не занимали отдельное место;
* `unique` --- реализовать `UniquePtr`, указатель с уникальным владением объектом.
* `shared` --- реализовать `SharedPtr`, указатель с разделяемым владением.
* `weak` --- реализовать `WeakPtr`, слабую ссылку на объект под управлением
  `SharedPtr`.
* `shared-from-this` --- реализовать `EnableSharedFromThis`, способ получать
  `SharedPtr`/`WeakPtr` из самого объекта.

Подробности по каждой задаче лежат в README внутри их директорий.
