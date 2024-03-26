#include <iostream>
#include <windows.h>
#include <opencv2/opencv.hpp>

int main(){
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
    
    BITMAPINFOHEADER  bi;
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

    StretchBlt(Chwnd, 0, 0, Width, Height, Window, XScreen, YScreen, Width, Height, SRCCOPY);  //change SRCCOPY to NOTSRCCOPY for wacky colors !
	GetDIBits(Chwnd, Bitmap, 0, Height, img.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    DeleteObject(Bitmap);
    DeleteDC(Chwnd);
    ReleaseDC(hWnd, Window);

    std::vector<uchar> buf;
	cv::imencode(".png", img, buf);

	cv::imwrite("Screenshot.png", img);

	buf.clear();
    
    std::cout << "Hello, world!" << std::endl;
    system("PAUSE");
}