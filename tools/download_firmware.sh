#!/usr/bin/env bash
#### Description: Script downloads multpile files firmware on esp devise using esptool.

## Set paths to esptool and folder with binary files (firmware).
esptool_path="/path/esp-open-sdk/xtensa-lx106-elf/bin"
esp_bin_path="/path/ESP8266_MESH_DEMO/bin"

## Set names of binary files.
bin_files=( "esp_init_data_default.bin" "blank.bin" "boot_v1.4(b1).bin" "upgrade/user1.1024.new.2.bin" )

## Set parameters of download tool.
port="/dev/ttyUSB0"
baud="115200"
flash_mode="qio"
flash_size="8m"
flash_freq="40m"

for i in "${bin_files[@]}"
do
    sudo python ${esptool_path}/esptool.py --port ${port} --baud ${baud} write_flash --flash_mode ${flash_mode} --flash_size ${flash_size} --flash_freq ${flash_freq} 0xfc000 ${esp_bin_path}/${i}
done
