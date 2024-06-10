#include <iostream>
#include "debug.hpp"
#pragma once


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
    StretchBlt(Chwnd, 0, 0, Width, Height, Window, XScreen, YScreen, Width, Height, SRCCOPY);
    HICON hicon;
    CURSORINFO ci;
    ICONINFO icninf;
    ci.cbSize = sizeof(ci);
    GetCursorInfo(&ci);
    hicon = CopyIcon(ci.hCursor);
    GetIconInfo(hicon, &icninf);
    DrawIcon(Chwnd, ci.ptScreenPos.x - (int)icninf.xHotspot, ci.ptScreenPos.y - (int)icninf.yHotspot, hicon);

    GetDIBits(Chwnd, Bitmap, 0, Height, img.data, (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    DeleteObject(Bitmap);
    DeleteDC(Chwnd);
    ReleaseDC(hWnd, Window);
    std::vector<uchar> buf;
    cv::imencode(".jpg", img, buf);
    return buf;
}

void AddHeader(std::vector<uchar> &vector, size_t size, bool reverse){
        unsigned char bytes[sizeof(size)];
        memcpy(bytes, &size, sizeof(size));
        dMessage(bytes);
        for (int i = 0; i < sizeof(size); i++)
        {
            if (reverse){vector.insert(vector.end() + i, bytes[i]);}
            else{vector.insert(vector.begin() + i, bytes[i]);}
        }
}