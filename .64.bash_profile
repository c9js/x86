#┌────────────────┐
#│ Добавляем цвет │
#└────────────────┘
function color_my_prompt {
# Список цветов
    local BLUE='\[\e[1;34m\]'        # Синий (тёмный)
    local GREEN_LIGHT='\[\e[1;32m\]' # Зеленый (светлый)
    local CLEAR='\[\e[0m\]'          # Сброс цвета
    
# Обновляем основной вид приглашения командной строки
    PS1="$GREEN_LIGHT\u$CLEAR:$BLUE\w$CLEAR$ "
}
export PROMPT_COMMAND=color_my_prompt

#┌──────────────────────────────┐
#│ Отключаем сохранение истории │
#└──────────────────────────────┘
export HISTFILE=/dev/null

#┌────────────────┐
#│ Список алиасов │
#└────────────────┘
alias c='reset'
alias l='c && ls -AFhl --group-directories-first'

#┌─────────────┐
#│ Приветствие │
#└─────────────┘
c
echo 'Добро пожаловать на сервер!'

#┌─────────────────────────────┐
#│ Переходим в рабочий каталог │
#└─────────────────────────────┘
cd "$PATH_WORKSPACE"

#┌───────────────────┐
#│ Пакеты для сборки │
#└───────────────────┘
apk add linux-headers
# apk add gcc libc-dev musl-dev

#┌────────┐
#│ Сборка │
#└────────┘
gcc scanner.c -O3 -Wall -Wextra -Werror -pedantic -flto -o scanner
gcc sender.c -O3 -Wall -Wextra -Werror -pedantic -flto -o sender

#┌───────────────────────────┐
#│ Запускаем прием сообщений │
#└───────────────────────────┘
node scanner.js

# ./wlan.sh 'SSID' 'PASSWORD'
