Выполнено 
 реализация графа как набора BDD, анализ графа как применение операций к BDD.
 поддержка команды check <число>
 поддержка команды move <число>
 разбор команд робота на основе регулярных выражений.
 поддержка запятой в команде move основанная на регулярных выражениях.
 поддержка запятой в команде check, основанная на регулярных выражениях.
 поддержка двоеточия в команде move основанная на регулярных выражениях.
 поддержка оператора not в командах move,check основанная на регулярных выражениях
 
 поддержка команды back <число>
 поддержка запятой в команде back основанная на регулярных выражениях.
 поддержка двоеточия в команде back основанная на регулярных выражениях.
 поддержка оператора not в командах back основанная на регулярных выражениях
 
 
 используются регулярные выражения std::regex (с++ 11) 
 BDD строятся с помощью библиотеки BuDDy
 