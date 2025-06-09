const { exec } = require('child_process');
const L2 = require('./build/Release/l2raw');

(async () => {
    console.log('Scanning...');
    while (true) {
        try {
            // console.time('timer');
            const packet = L2.scanner('eth0');
            // console.timeEnd('timer');
            console.log(packet.toString('hex'));
            // exec("awk '/MemTotal/ {total=$2} /MemAvailable/ {free=$2} END {printf(\"Memory used: %.2f MB\", (total-free)/1024)}' /proc/meminfo", (error, stdout, stderr) => {
                // console.log(stdout);
            // });
        } catch (e) {
            console.error('Ошибка:', e.message);
        }
        await new Promise(r => setTimeout(r, 100));
    }
})();
