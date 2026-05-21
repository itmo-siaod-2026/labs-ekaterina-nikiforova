#set page(
  paper: "a4",
  margin: (x: 2.5cm, y: 1.5cm),
)
#set text(font: "Times New Roman", size: 12pt, lang: "ru")

#grid(
  columns: (1fr, auto),
  [
    #set text(weight: "bold", size: 11pt)
    #align(center)[
      Федеральное государственное автономное образовательное учреждение высшего образования \
      «Национальный исследовательский университет ИТМО»
    ]
  ],
)

#v(1em)
#line(length: 100%, stroke: 1.5pt)

#v(0.5em)
#align(center)[
  #set text(size: 11pt)
  #text(
    weight: "bold",
  )[Факультет Программной инженерии и компьютерной техники] \
  #text(
    weight: "bold",
  )[Образовательная программа Проектирование и разработка систем больших данных] \
  #text(
    weight: "bold",
  )[Направление подготовки (специальность) 09.04.04 - Программная инженерия]
]
#v(2.5em)

#v(6em)

#align(center)[
  #text(size: 16pt, weight: "bold")[ОТЧЕТ] \
  #v(0.5em)
  #text(size: 13pt)[
    по лабораторной работе №4 \
    по курсу «Структуры и алгоритмы в базах данных и распределенных системах» \
    на тему: «Thread-safe хеш-таблица с закрытой адресацией.»
  ]
]

#v(7em)

#grid(
  columns: (1fr, auto),
  gutter: 1em,
  [],
  [
    #set align(left)
    #block(width: 8cm)[
      #grid(
        columns: (auto, 1fr),
        gutter: 1em,
        rows: 2.2em,

        [Обучающийся:],
        [
          Никифорова Е. А., P4135
        ],

        [Преподаватель:],
        [
          Платонов А. В., доцент
        ],

        [Преподаватель:],
        [
          Портнов П. В., ассистент
        ],
      )
    ]
  ],
)

#v(15em)

#align(center)[
  #text[2026 г.]
]

#pagebreak()

#show outline.entry.where(
  level: 1,
): it => {
  v(12pt, weak: true)
  strong(it)
}

#outline(title: "Содержание", indent: 1em)
#pagebreak()

#align(center)[= Задание]
#v(1em)

== Concurrent hash-map

Требуется реализовать thread-safe хеш-таблицу с закрытой адресацией.

*Требования к структуре:*
- минимально необходимый набор операций:
    + put(key: K, value: V),
    + get(key: K) -> V,
    + size() -> usize,
    + clear(),
    + merge(key: K, value: V, merger: Fn(V, V) -> V) -> V,
    + итератор по парам ключ-значения

- (почти) никогда не блокирующие операции чтения
- однозначный наблюдаемый порядок между завершёнными операциями
- За более формальной спецификацией см. Javadoc к ConcurrentHashMap (https://docs.oracle.com/en/java/javase/25/docs/api/java.base/java/util/concurrent/ConcurrentHashMap.html) в JDK.
- При этом не требуется реализовывать конкретный интерфейс для таблицы из выбранного ЯП, это просто референс.

*В работе также требуется:*
- Написать бенчмарки (сравнивать перф уместно с не-thread-safe версией)
- Написать concurrency-тесты с использованием специализированного инструментария (например, если Java, jcstress)
- Нарисовать графики по числовым результатам
- Объяснить интересные результаты в отчётё

#pagebreak()

= Реализация


#pagebreak()

= Тестирование

=== Результаты бенчмарков:
