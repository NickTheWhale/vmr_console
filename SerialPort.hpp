/*
* Author: Manash Kumar Mandal
* Modified Library introduced in Arduino Playground which does not work
* This works perfectly
* LICENSE: MIT
*/

#pragma once

#define ARDUINO_WAIT_TIME 10
#define MAX_DATA_LENGTH 1023

#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <string>

class SerialPort
{
private:
    HANDLE handler;
    bool connected;
    COMSTAT status;
    DWORD errors;
public:
    explicit SerialPort(const char* portName, DWORD baudRate = CBR_9600);
    ~SerialPort();

    int readSerialPort(const char* buffer, unsigned int buf_size);
    bool writeSerialPort(const char* buffer, unsigned int buf_size);
    bool isConnected();
    void closeSerial();
    int flushInputBuffer();
    int flushOutputBuffer();
    int inputBufferSize();
};