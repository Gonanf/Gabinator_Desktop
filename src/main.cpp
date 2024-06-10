#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <opencv2/opencv.hpp>
#include <windows.h>
#include "debug.hpp"
#include "USB_Protocol.hpp"
#pragma once

int main()
{
    USB protocolo;

    while (true)
    {
        if (protocolo.HasDevice() == false)
        {
            std::cout << "Looking for a connection" << std::endl;
            protocolo.ObtainNewUSB();
        }
        else
        {
            std::cout << "Sending Images" << std::endl;
            protocolo.SendCaptureToUSB();
        }
    }

    std::cin.get();
}