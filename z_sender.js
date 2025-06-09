const { exec } = require('child_process');
const L2 = require('./build/Release/l2raw');

// Создаем Ethernet-кадр: [DST_MAC][SRC_MAC][ETH_TYPE][DATA]
let rawPacket = Buffer.from([
  // dst MAC
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  // src MAC
  0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc,
  // ethertype (0x0800 = IPv4)
  0x08, 0x00,
  // payload (например, просто "hello")
  0x68, 0x65, 0x6c, 0x6c, 0x6f
]);

(async () => {
    while (true) {
        try {
            // console.time('timer');
            L2.sender('eth0', rawPacket);
            // console.timeEnd('timer');
            // console.log('Packet sent!');
            exec("awk '/MemTotal/ {total=$2} /MemAvailable/ {free=$2} END {printf(\"Memory used: %.2f MB\", (total-free)/1024)}' /proc/meminfo", (error, stdout, stderr) => {
                console.log(stdout);
            });
        } catch (e) {
            console.error('Ошибка:', e.message);
        }
        await new Promise(r => setTimeout(r, 100));
    }
})();
