#include <iostream>
#include "packeter.hpp"
#include "libusb.h"
#include "debug.hpp"
#pragma once

#define INTERFACE_T 0
#define RECONNECT_TRIES 5

// Protocolo USB y pasada de datos
class USB
{
private:
    libusb_context *contexto = NULL;
    libusb_device **dispositivos = NULL;
    libusb_device_handle *CelHandle = NULL;
    uint8_t endpoint = 0;

    int result = 0;
    ssize_t cant = 0;

    libusb_device *Celular = NULL;
    libusb_device_descriptor CelularDesc;

    const char *manufacturer = "Chaos";
    const char *modelName = "EEST";
    const char *description = "Gabinator";
    const char *version = "1.0";
    const char *uri = "https://github.com/Gonanf/Gabinator_Android/tree/master";
    const char *serialNumber = "1990";

    uint8_t InterfaceNum;

    // Tratar de conectarse con el dispositivo accesorio
    int8_t ObtainAccesory()
    {
        int f = RECONNECT_TRIES;
        while (f-- > 0)
        {
            CelHandle = libusb_open_device_with_vid_pid(NULL, 0x18d1, 0x2d01);
            if (CelHandle == NULL)
            {
                std::cout << "Retrying opening accesory USB" << std::endl;
                CelHandle = libusb_open_device_with_vid_pid(NULL, 0x18d1, 0x2d00);
                if (CelHandle != NULL)
                {
                    dMessage("Accesory opened 0x2d00");
                    return 1;
                }
            }
            else
            {
                dMessage("Accesory opened 0x2d01");
                return 1;
            }
            Sleep(3000);
        }
        if (CelHandle == NULL)
        {
            return -1;
        }
    }

    // Mandar protocolo AOA sobre la informacion de la aplicacion para gabinaor android
    int8_t AccesoryProtocol(libusb_device_handle *handle)
    {
        int error = 0;
        dMessage("Sending device version request...");
        unsigned char ioBuffer[2];
        int devVersion;
        int response;
        try
        {
            response = libusb_control_transfer(handle, 0xC0, 51, 0, 0, ioBuffer, 2, 100);
        }
        catch (const std::exception &e)
        {
            errMessage(e.what());
        }
        dMessage("Sending device version request sent, waiting response...");
        if (response < 0)
        {
            errMessage(libusb_error_name(response));
            return -1;
        }
        dMessage("--------RESPONSE--------");
        dMessage((int)ioBuffer[0]);
        dMessage((int)ioBuffer[1]);
        dMessage("--------------------------");

        devVersion = ioBuffer[1] << 8 | ioBuffer[0];
        if (!(devVersion == 1 || devVersion == 2))
            return -1;
        dMessage(devVersion);
        dMessage("Sending accesory identification...");
        response = libusb_control_transfer(handle, 0x40, 52, 0, 0, (unsigned char *)manufacturer, strlen(manufacturer) + 1, 0);
        if (response < 0)
        {
            errMessage(libusb_error_name(response));
            return -1;
        }
        response = libusb_control_transfer(handle, 0x40, 52, 0, 1, (unsigned char *)modelName, strlen(modelName) + 1, 0);
        if (response < 0)
        {
            errMessage(libusb_error_name(response));
            return -1;
        }
        response = libusb_control_transfer(handle, 0x40, 52, 0, 2, (unsigned char *)description, strlen(description) + 1, 0);
        if (response < 0)
        {
            errMessage(libusb_error_name(response));
            return -1;
        }
        response = libusb_control_transfer(handle, 0x40, 52, 0, 3, (unsigned char *)version, strlen(version) + 1, 0);
        if (response < 0)
        {
            errMessage(libusb_error_name(response));
            return -1;
        }
        response = libusb_control_transfer(handle, 0x40, 52, 0, 4, (unsigned char *)uri, strlen(uri) + 1, 0);
        if (response < 0)
        {
            errMessage(libusb_error_name(response));
            return -1;
        }
        response = libusb_control_transfer(handle, 0x40, 52, 0, 5, (unsigned char *)serialNumber, strlen(serialNumber) + 1, 0);
        if (response < 0)
        {
            errMessage(libusb_error_name(response));
            return -1;
        }

        dMessage("Accessory Identification sent");
        return 1;
    }

    // Obtener el endpoint de transferencia
    uint8_t GetDeviceEndPoint(libusb_device_handle *handle)
    {
        libusb_device *device = libusb_get_device(handle);
        libusb_config_descriptor *desc;
        libusb_get_active_config_descriptor(device, &desc);
        const libusb_interface *interfaces = desc->interface;
        for (int i = 0; i < desc->bNumInterfaces; i++)
        {
            libusb_interface interface = interfaces[i];
            std::cout << interface.num_altsetting << std::endl;
            const libusb_interface_descriptor *AltDescriptor = interface.altsetting;
            for (int j = 0; j < interface.num_altsetting; j++)
            {
                const libusb_endpoint_descriptor *endpoints = AltDescriptor[j].endpoint;
                for (int e = 0; e < AltDescriptor[j].bNumEndpoints; e++)
                {
                    uint8_t TransferType = 0x03 & endpoints[e].bEndpointAddress;
                    if (TransferType == LIBUSB_TRANSFER_TYPE_BULK)
                    {
                        InterfaceNum = j;
                        dMessage(endpoints[e].bEndpointAddress);
                        return endpoints[e].bEndpointAddress;
                    }
                }
            }
        }
        return 0;
    }

    // Limpieza y to eso
    

public:
    // Enviar datos de imagen a travez de uSB
    int SendCaptureToUSB()
    {
        int endsize;
        if (endpoint == 0)
        {
            bool fixed = false;
            errMessage("Couldnt find an endpoint...retryng");
            for (int i = 0; i < RECONNECT_TRIES; i++)
            {
                endpoint = GetDeviceEndPoint(CelHandle);
                if (endpoint != 0)
                    fixed = true;
            }
            return fixed ? 1 : -1;
        }
        std::vector<uchar> image_data = CaptureScreen();
        dMessage(image_data.size());
        int r = libusb_bulk_transfer(CelHandle, endpoint, image_data.data(), image_data.size(), &endsize, 0);
        if (r < 0)
        {
            errMessage(libusb_error_name(r));
            return -1;
        }
        dMessage(endsize);
        return 1;
    }

    // AOA (Android Accesory Protocol)
    int8_t ObtainNewUSB()
    {
        cant = libusb_get_device_list(NULL, &dispositivos);
        if (cant < 0)
            goto error;
        for (int i = 0; i < cant; i++)
        {
            libusb_get_device_descriptor(dispositivos[i], &CelularDesc);
            Celular = dispositivos[i];
            libusb_device_handle *handle;
            dMessage(CelularDesc.idProduct);
            int result = libusb_open(Celular, &handle);
            if (result < 0)
            {
                dMessage(libusb_error_name(result));
                continue;
            }
            libusb_set_configuration(handle, INTERFACE_T);
            if (libusb_kernel_driver_active(handle, INTERFACE_T))
            {
                libusb_set_auto_detach_kernel_driver(handle, INTERFACE_T);
            }
            result = AccesoryProtocol(handle);
            if ((CelularDesc.idVendor == 0x18D1 && CelularDesc.idProduct == 0x2D01 || CelularDesc.idVendor == 0x18d1 && CelularDesc.idProduct == 0x2D00 && result >= 0))
            {
                std::cout << std::endl
                          << "ACCESORY DEVICE FOUND" << std::endl
                          << std::endl;
                if (ObtainAccesory() < 0)
                {
                    goto error;
                }
                dMessage("Claiming interface... ");
                result = libusb_claim_interface(CelHandle, INTERFACE_T);
                if (result < 0)
                {
                    goto error;
                }
                goto good;
            }

            if (result <= 0)
            {
                dMessage("This device does not support Accesory Mode");
                continue;
            }
            result = libusb_control_transfer(handle, 0x40, 53, 0, 0, NULL, 0, 0);
            if (result < 0)
            {
                dMessage(libusb_error_name(result));
                continue;
            }
            dMessage("Accessory Identification received");
            libusb_device_descriptor dev;
            std::cout << "PID: " << CelularDesc.idProduct << " VID: " << CelularDesc.idVendor << std::endl;
            if (ObtainAccesory() < 0)
            {
                goto error;
            }
            dMessage("Claiming interface... ");
            result = libusb_claim_interface(CelHandle, INTERFACE_T);
            if (result < 0)
            {
                goto error;
            }
            goto good;
        }
    libusb_free_device_list(dispositivos, 1);
    return 1;

    good:
        libusb_free_device_list(dispositivos, 1);
        libusb_set_configuration(CelHandle, INTERFACE_T);
        if (libusb_kernel_driver_active(CelHandle, INTERFACE_T))
        {
            libusb_set_auto_detach_kernel_driver(CelHandle, INTERFACE_T);
        }
        endpoint = GetDeviceEndPoint(CelHandle);
        return 1;

    error:
        errMessage(libusb_error_name(result));
        libusb_free_device_list(dispositivos, 1);
        libusb_release_interface(CelHandle, INTERFACE_T);
        libusb_attach_kernel_driver(CelHandle, INTERFACE_T);
        libusb_close(CelHandle);
        return -1;
    }

    bool HasDevice(){
        return CelHandle != NULL ? true : false;
    }

    ~USB()
    {
        dMessage("Closing USB Protocol");
        PAUSE;
        libusb_free_device_list(dispositivos, 1);
        libusb_release_interface(CelHandle, INTERFACE_T);
        libusb_attach_kernel_driver(CelHandle, INTERFACE_T);
        libusb_reset_device(CelHandle);
        libusb_close(CelHandle);
        libusb_exit(contexto);
    }

    //Inicio de Libusb
    USB(){
        dMessage("Starting USB Protocol");
        PAUSE;
        result = libusb_init(&contexto);
        if (result != 0)
            errMessage(libusb_error_name(result));
        start;
    }
};
