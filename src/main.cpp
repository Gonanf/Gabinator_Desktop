#include <iostream>
#include <windows.h>
#include <opencv2/opencv.hpp>
#include "libusb.h"

libusb_context *contexto = NULL;
libusb_device **dispositivos = NULL;
int result = 0;
ssize_t cant = 0;

libusb_device *Celular = NULL;
libusb_device_descriptor CelularDesc;

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

void ObtainNewUSB(){
        while (true)
    {
            libusb_device **newDispositivos = NULL;
            ssize_t cantNew = libusb_get_device_list(contexto, &newDispositivos);
            assert(cantNew > 0);
            if (cant < cantNew)
            {
                Celular = newDispositivos[cant];
                libusb_get_device_descriptor(Celular, &CelularDesc);
                printf("Vendor:Device = %04x:%04x\n", CelularDesc.idVendor, CelularDesc.idProduct);
                libusb_free_device_list(newDispositivos, 1);
                libusb_free_device_list(dispositivos, 1);
                break;
            }

    }
}

void SendCaptureToUSB(){
    //TODO: make this
}

int main()
{



    result = libusb_init(&contexto);
    assert(result == 0);


    cant = libusb_get_device_list(contexto, &dispositivos);
    assert(cant > 0);

    while (true)
    {
        if (Celular == NULL){
            ObtainNewUSB();
        }
        else{
            SendCaptureToUSB();
        }
    }
    


    libusb_exit(contexto);

    system("PAUSE");
}