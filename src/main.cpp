#include <iostream>
#include <windows.h>
#include <opencv2/opencv.hpp>
#include "libusb.h"

#define RECONNECT_TRIES 5

libusb_context *contexto = NULL;
libusb_device **dispositivos = NULL;
int result = 0;
ssize_t cant = 0;

libusb_device *Celular = NULL;
libusb_device_descriptor CelularDesc;

const char *manufacturer = "PCHost";
const char *modelName = "PCHost1";
const char *description = "Description";
const char *version = "1.0";
const char *uri = "http://example.com";
const char *serialNumber = "666";

std::vector<uchar> CaptureScreen()
{
    cv::Mat img;
    HWND hWnd = GetDesktopWindow();
    HDC Window = GetDC(hWnd);
    HDC Chwnd = CreateCompatibleDC(Window);
    SetStretchBltMode(Chwnd, COLORONCOLOR);
    int Scale = 1;
    int XScreen = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int YScreen = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int Width = GetSystemMetrics(SM_CXSCREEN);
    int Height = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP Bitmap = CreateCompatibleBitmap(Window, Width, Height);

    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = Width;
    bi.biHeight = -Height;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    img.create(Height, Width, CV_8UC4);

    SelectObject(Chwnd, Bitmap);

    StretchBlt(Chwnd, 0, 0, Width, Height, Window, XScreen, YScreen, Width, Height, SRCCOPY); // change SRCCOPY to NOTSRCCOPY for wacky colors !
    GetDIBits(Chwnd, Bitmap, 0, Height, img.data, (BITMAPINFO *)&bi, DIB_RGB_COLORS);
    DeleteObject(Bitmap);
    DeleteDC(Chwnd);
    ReleaseDC(hWnd, Window);

    std::vector<uchar> buf;
    cv::imencode(".png", img, buf);
    return buf;
}
libusb_device_handle *CelHandle;

int8_t AccesoryProtocol(libusb_device_handle *handle)
{
    int error = 0;
    std::cout << "Sending device version request..." << std::endl;
    unsigned char ioBuffer[2];
    int devVersion;
    int response;
    try
    {
        response = libusb_control_transfer(handle, 0xC0, 51, 0, 0, ioBuffer, 2, 100);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    std::cout << "Sending device version request sent, waiting response..." << std::endl;
    if (response < 0)
    {
        std::cout << "Error: " << response << std::endl;
        return -1;
    }
    std::cout << "--------RESPONSE--------" << std::endl;
    std::cout << (int)ioBuffer[0] << std::endl;
    std::cout << (int)ioBuffer[1] << std::endl;
    std::cout << "--------------------------" << std::endl;
    devVersion = ioBuffer[1] << 8 | ioBuffer[0];
    if (!(devVersion == 1 || devVersion == 2))
        return -1;
    fprintf(stdout, "Version Code Device: %d\n", devVersion);
    std::cout << "Sending accesory identification..." << std::endl;
    response = libusb_control_transfer(handle, 0x40, 52, 0, 0, (unsigned char *)manufacturer, strlen(manufacturer) + 1, 0);
    if (response < 0)
    {
        std::cout << "Error: " << response << std::endl;
        return -1;
    }
    response = libusb_control_transfer(handle, 0x40, 52, 0, 1, (unsigned char *)modelName, strlen(modelName) + 1, 0);
    if (response < 0)
    {
        std::cout << "Error: " << response << std::endl;
        return -1;
    }
    response = libusb_control_transfer(handle, 0x40, 52, 0, 2, (unsigned char *)description, strlen(description) + 1, 0);
    if (response < 0)
    {
        std::cout << "Error: " << response << std::endl;
        return -1;
    }
    response = libusb_control_transfer(handle, 0x40, 52, 0, 3, (unsigned char *)version, strlen(version) + 1, 0);
    if (response < 0)
    {
        std::cout << "Error: " << response << std::endl;
        return -1;
    }
    response = libusb_control_transfer(handle, 0x40, 52, 0, 4, (unsigned char *)uri, strlen(uri) + 1, 0);
    if (response < 0)
    {
        std::cout << "Error: " << response << std::endl;
        return -1;
    }
    response = libusb_control_transfer(handle, 0x40, 52, 0, 5, (unsigned char *)serialNumber, strlen(serialNumber) + 1, 0);
    if (response < 0)
    {
        std::cout << "Error: " << response << std::endl;
        return -1;
    }

    fprintf(stdout, "Accessory Identification sent\n");

    return 1;
}


int8_t ObtainNewUSB()
{
    cant = libusb_get_device_list(NULL, &dispositivos);
    if (cant > 0)
    {
        for (int i = 0; i < cant; i++)
        {
            libusb_get_device_descriptor(dispositivos[i], &CelularDesc);
            Celular = dispositivos[i];
            libusb_device_handle *handle;
            std::cout << "Opening device " << CelularDesc.idProduct << "..." << std::endl;
            int result = libusb_open(Celular,&handle);
            if (result < 0){
                std::cout << "Failed to open device " << result << std::endl;
                if (result == LIBUSB_ERROR_NOT_SUPPORTED){
                    std::cout << "This device cannot be opened" << std::endl;
                }
                continue;
            }
            libusb_set_auto_detach_kernel_driver(handle, 1);
            result = AccesoryProtocol(handle);
            if (result > 0)
            {
                result = libusb_control_transfer(handle, 0x40, 53, 0, 0, NULL, 0, 0);
                if (result < 0)
                {
                    std::cout << "Error: " << result << std::endl;
                    if (result == LIBUSB_ERROR_NOT_SUPPORTED){
                        std::cout << "This device cannot control transfer" << std::endl;
                    }
                    continue;
                }
                fprintf(stdout, "Accessory Identification received\n");
                int global_tries = RECONNECT_TRIES;
                while (global_tries-- >= 0){
                        std::cout << "Trying to open in accesory mode..." << std::endl;
                        CelHandle = libusb_open_device_with_vid_pid(NULL, CelularDesc.idVendor, CelularDesc.idProduct);
                        if (CelHandle != NULL){
                            libusb_close(handle);
                            std::cout << "Success" << std::endl;
                            return 1;
                        }
                    Sleep(5000);
                }
            }
            else{
                std::cout << "This device does not support Accesory Mode" << std::endl;
            }
        }
        return -1;
    }
    libusb_free_device_list(dispositivos, 1);
}

uint8_t GetDeviceEndPoint(libusb_device_handle *handle)
{
    libusb_device *device = libusb_get_device(handle);
    libusb_config_descriptor *desc;
    libusb_get_active_config_descriptor(device, &desc);
    const libusb_interface *interfaces = desc->interface;
    for (int i = 0; i < desc->bNumInterfaces; i++)
    {
        libusb_interface interface = interfaces[i];
        const libusb_interface_descriptor *AltDescriptor = interface.altsetting;
        for (int j = 0; j < interface.num_altsetting; j++)
        {
            const libusb_endpoint_descriptor *endpoints = AltDescriptor[j].endpoint;
            for (int e = 0; e < AltDescriptor[j].bNumEndpoints; e++)
            {
                uint8_t TransferType = 0x03 & endpoints[e].bEndpointAddress;
                if (TransferType == LIBUSB_TRANSFER_TYPE_BULK)
                {
                    return endpoints[e].bEndpointAddress;
                }
            }
        }
    }
    return 0;
}


int SendCaptureToUSB()
{
    int endsize;
    uint8_t endpoint = GetDeviceEndPoint(CelHandle);
    if (endpoint != 0)
    {
        std::vector<uchar> buf = CaptureScreen();
        std::cout << (int)buf.data() << std::endl;
        int r = libusb_bulk_transfer(CelHandle, endpoint, buf.data(), buf.size(), &endsize, 0);
        if (r < 0)
        {
            std::cout << "Transfer error " << r << std::endl;
            return -1;
        }

        std::cout << "BYTES: " << endsize << std::endl;
        return 1;
    }
    else{
        return -1;
    }
}

int main()
{

    result = libusb_init(&contexto);
    assert(result == 0);

    while (true)
    {
        if (CelHandle == NULL)
        {
            std::cout << "Looking for a connection" << std::endl;
            ObtainNewUSB();
        }
        else
        {
            std::cout << "sending" << std::endl;
             if (SendCaptureToUSB() < 0){
                
                libusb_close(CelHandle);
                CelHandle = NULL;
             }
        }
    }

    libusb_exit(contexto);

    system("PAUSE");
}