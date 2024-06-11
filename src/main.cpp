#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <vector>
#include "gabinator.hpp"


int main()
{
    USB protocolo;
    Settings config;
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
            protocolo.SendCaptureToUSB(config.compression);
        }
    }
    std::cin.get();
}
