# Base-N Encoding

В этой задаче требуется реализовать кодирование и декодирование данных в
[Base16](https://en.wikipedia.org/wiki/Base16),
[Base32](https://en.wikipedia.org/wiki/Base32) и
[Base64](https://en.wikipedia.org/wiki/Base64) по стандарту RFC 4648.

Декодер должен принимать только каноническую запись и бросать
`std::invalid_argument` при некорректном вводе.

Подробности ожидаемого поведения лучше смотреть в тестах.
