const { spawn } = require('child_process');

// Команда и её параметры
const command = './scanner';
const args = ['eth0', '0.1'];

// Запускаем команду
const process = spawn(command, args);

let i = 0;

// Обрабатываем стандартный вывод
process.stdout.on('data', (data) => {
    // console.log(`${i}=${data.slice(0, 20)}=${data.slice(-10)}`);
    console.log(`${i}=${data}`);
    console.log(++i);
});

// Обрабатываем ошибки вывод
process.stderr.on('data', (data) => {
    console.log(`Системное сообщение!\n${data}`);
});

// Обрабатываем завершение процесса
process.on('close', (code) => {
    console.log(`Процесс завершён с кодом: ${code}`);
});
