#include <iostream>
#pragma once

#define DEBUG true

#if DEBUG
    #define dMessage(val) std::cout << val << std::endl
    #define start libusb_set_debug(NULL, 3)
    #define PAUSE std::cin.get()
#else
    #define dMessage(val) 
    #define start 
    #define PAUSE
#endif

#define errMessage(val) std::cerr << "ERROR MESSAGE: " << val << std::endl