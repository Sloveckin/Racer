# DM_MONITOR

## Версия ядра 6.5.0-060500-generic

## Cборка

```bash
make all
```

## Загрузка модуля

```bash
sudo insmod dm_monitor.ko
```

## Закрепление dm_monitor за блочным устройством

```bash
SIZE=2097152

DEV=/dev/mapper/bar # some backend dev to use for our device

dmsetup create my0 --table "0 $SIZE dm_monitor $DEV"
```

## Запуск

```bash

/* Ввод, который не вызывает Data race */

// Ничего выводиться не должно
sudo dmesg | grep -i "Data race"

/* Ввод, вызывающий Data race */

// Находятся записи
sudo dmesg | grep -i "Data race"
```

## Выгрузка модуля

```bash
sudo rmmod dm_monitor
```