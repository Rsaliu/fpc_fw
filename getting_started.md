## Cloning repo

- use `git clone --recursive https://github.com/Rsaliu/fpc_fw.git`

## Updating repo with submodule

- use `git submodule --update --init --recursive`

## Assumptions

- You have vscode installed


## Install ESP IDF

- Install ESP-IDF (by Espressif Systems) extension on vscode

This should setup to build and flash the code.

## Build the code 

- press `Ctrl+Shift+P` 
- then search for ESP-IDF in the search box,
- then click `ESP-IDF: Build Your Project`

## Flash the code to the MCU

- press `Ctrl+Shift+P` 
- then search for ESP-IDF in the search box,
- then click `ESP-IDF: Select Port to Use (COM,tty,usbserial)`
- then enter the COM port, for example COM5,
- then press `Ctrl+Shift+P` 
- then search for UART in the search box,
- then select `ESP:IDF: Flash (UART) Your Project`

## How to add your own Library

- Using the pump library at `components/pump` as a reference, the folder structure of a library looks like this:
```
- components
    - Your Library
        - src
            - Your library c files
        - CMakeLists.txt // use the one in pump library as a reference
        - Your library headers
        - test
            - Your test c files
            - CMakeLists.txt // use the one in pump library as a reference
```
- A faster way would be to make a copy of the `pump` library and edit it for your own implementation.

## How to run tests

In this project, we will be using  `Unity` Test Framework

### To run tests
- edit the file at: `unity-app/main/test_app_main.c`
- add the name of your test by including a new line with `unity_run_test_by_name("<name-of-your-test>");`
- press `Ctrl+Shift+P` 
- then search for `ESP-IDF: Unit Test: Build and Flash Unit Test App for Testing` in the search box
- click it, and it should run the test.
