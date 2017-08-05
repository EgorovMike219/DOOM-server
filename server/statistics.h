#ifndef STATISTICS_H
#define STATISTICS_H

//Функция возвращает ответ со статистикой для игрока с данным id
void answer_from_statistics(int id);

// Функция принимает id игрока и его здоровье и делает запись в соответствующий этому игроку файл
// его id и здоровье (для сбора статистики)
void record_in_statistics(int id, int health);

#endif // STATISTICS_H
