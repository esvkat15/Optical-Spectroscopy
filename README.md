# Optical-Spectroscopy
Optical Spectroscopy project code for EECE 490B

development setup:

open a shell in the folder where u would like to set up the project:
```bash
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
git clone git@github.com:esvkat15/Optical-Spectroscopy.git
git clone https://github.com/raspberrypi/pico-sdk.git
cp pico-sdk/external/pico_sdk_import.cmake Optical-Spectroscopy
cd Optical-Spectroscopy
mkdir build
cd build
cmake -DPICO_SDK_PATH=../../pico-sdk ..
```

development testing:

1. open a shell in the build folder of the project
```bash
make
```
2. hold the bootsel button on the pico while connecting the USB
3. copy the .uf2 file from the build folder onto the pico drive
4. open a shell in the user_interface folder of the project
```bash
dotnet run
```
or if in WSL
```bash
cmd.exe /C """dotnet run"""
```