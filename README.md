# Зимняя школа 2025 по RISC-V

## Характиристики моего ноута

### CPU & GPU

 - AMD Ryzen 5 3500U с встроенной графикой
 - AMD Radeon Vega 8

### ОС

 - Windows (зря)

### ПО для GPU

 - AMD APP SDK (устаревшее, так как для OpenCL 2.0)
 - Какой-то свежий драйвер с [сайта AMD](https://www.amd.com/en/support/download/drivers.html)

## Моя инструкция по запуску

```bash
git clone https://github.com/BZ6/winter25/
cd winter25
gcc -o main.exe main.c -I 'C:\Program Files (x86)\AMD APP SDK\2.9-1\include' -L 'C:\Program Files (x86)\AMD APP SDK\2.9-1\lib\x86_64' -l OpenCL
./main.exe
```
