# ROS Laser Scanner

Утилита принимает на вход 2D карту окружения в формате [OccupancyGrid.msg](docs.ros.org/jade/api/nav_msgs/html/msg/OccupancyGrid.html) и [параметры сканирования](#Формат-входных-данных), генерирует лазерный скан окружения и сохраняет результат в формате [LaserScan.msg](docs.ros.org/api/sensor_msgs/html/msg/LaserScan.html).

Требования для установки
------------------------

* GNU Make 4.0+
* gcc 5.0+
* cpp std 11+

Установка
------------

Чтобы собрать исполняемый файл, необходимо в корневой директории выполнить команду:

```
make
```
Чтобы удалить исполняемый файл и временные бинарные файлы, необходимо выполнить команду:

```
make clean
```

Формат входных данных
---------------------

Параметры сканирования задаются в формате [YAML](wiki.ros.org/YAML%20Overview).
Ниже приведен пример входного файла, в котором перечислены все возможные параметры. Все параметры кроме `grid` являются не обязательными. Они имеют значения по умолчанию, указанные в примере. Значение `range_max: -1.0` означает, что максимальное расстояние не ограничено. При записи результата в файл подставляется значение максимального расстояния, полученного при сканировании. Если параметры `angle_min` и `angle_max` отличаются больше чем на 2π, `angle_max`
 автоматически заменится на `angle_min + 2π`.
```
grid: map.msg
angle_min: 0.0
angle_max: 6.28318530
angle_increment: 0.001
range_min: 0.0
range_max: -1.0
position: [0.0, 0.0]
```

Дополнительные возможности
--------------------------

Утилита также может сгенерировать ".pgm" файл -- визуализацию результата сканирования. <br>
Значение цветов на изображении:

* чёрный       -- препятствия
* темно-серый  -- область видимости датчика
* светло-серый -- "неизвестные" клетки
* белый        -- пустые клетки

Датчик помечен на изображении одним пикселем чёрного цвета. Верхний левый угол изображения соответствует клетке (0, 0) сетки, ось OY направлена вниз.

Инструкция по использованию
---------------------------

```
./scanner
-i имя_файла (обязательно) имя входного файла     (.yaml)
-o имя_файла (обязательно) имя выходного файла    (.msg)
-v имя_файла (опционально) имя файла визуализации (.pgm)
-h           (опционально) вывести подсказку
```

Примеры
-------

В директории [example](github.com/vasilyPML30/ros_laser_scan/tree/master/example) есть пример входных и выходных данных.
Файлы "scan1.msg" и "visual1.pgm" были получены выполнением в директории example команд:

```
../scanner -i params1.yaml -o scan1.msg -v visual1.pgm

```
Файлы "scan2.msg" и "visual2.pgm" были получены выполнением в директории example команд:

```
./scanner -i params2.yaml -o scan2.msg -v visual2.pgm
```

Также в example есть изображение (sample.pgm), показывающее как выглядело окружение до сканирования.

Алгоритм сканирования
---------------------

* В процессе сканирования перебираются значения угла от `angle_min` до `angle_max` с шагом, равным `angle_increment`.
* Для каждого значения угла из начальной точке пускается луч, и перебираются все клетки сетки, пересекаемые этим лучом.
* Клетки перебираются, пока не встретится препятствие (значение 0), неизвестная область (значение -1) или граница сетки.
* Если луч достиг границы сетки или неизвестной области, или если полученное значение не лежит в заданном диапазоне, результату присваивается значение -1. Иначе результат имеет корректное значение расстояния до ближайшего препятствия.
