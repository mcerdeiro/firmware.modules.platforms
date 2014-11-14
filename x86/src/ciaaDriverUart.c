/* Copyright 2014, Mariano Cerdeiro
 *
 * This file is part of CIAA Firmware.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/** \brief CIAA Uart Posix Driver
 **
 ** Simulated UART Driver for Posix for testing proposes
 **
 **/

/** \addtogroup CIAA_Firmware CIAA Firmware
 ** @{ */
/** \addtogroup Drivers CIAA Drivers
 ** @{ */
/** \addtogroup UART UART Drivers
 ** @{ */

/*
 * Initials     Name
 * ---------------------------
 * MaCe         Mariano Cerdeiro
 */

/*
 * modification history (new versions first)
 * -----------------------------------------------------------
 * 20140528 v0.0.1 initials initial version
 */

/*==================[inclusions]=============================================*/
#include "ciaaDriverUart.h"
#include "ciaaDriverUart_Internal.h"
#include "ciaaPOSIX_stdio.h"
#include "ciaaPOSIX_stdlib.h"
#include "ciaaPOSIX_string.h"
#include "OS_Internal.h"
#include "stdio.h"
#include "fcntl.h"

#include "sys/socket.h"
#include "sys/types.h"
#include "netinet/in.h"


/*==================[macros and definitions]=================================*/
/** \brief Cygwin only support deprecated macro FASYNC */
 #ifndef O_ASYNC
	#define O_ASYNC FASYNC
 #endif
 
/** \brief Pointer to Devices */
typedef struct  {
   ciaaDevices_deviceType * const * const devices;
   uint8_t countOfDevices;
} ciaaDriverConstType;

/*==================[internal data declaration]==============================*/
/** \brief Struct for describe uart pipes */
typedef struct ciaaDriverUart_serverStruct {
   struct sigaction signalAction;
   struct sockaddr_in serverData;
   struct sockaddr_in clientData;
   socklen_t clientDataSize;
   int serverSocket;
   int clientSocket;
   bool conected;
} ciaaDriverUart_serverType;

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/
/** \brief Device for UART 0 */
ciaaDriverUart_serverType ciaaDriverUart_server0;

/** \brief Device for UART 1 */
ciaaDriverUart_serverType ciaaDriverUart_server1;

static ciaaDevices_deviceType ciaaDriverUart_device0 = {
   "uart/0",                        /** <= driver name */
   ciaaDriverUart_open,             /** <= open function */
   ciaaDriverUart_close,            /** <= close function */
   ciaaDriverUart_read,             /** <= read function */
   ciaaDriverUart_write,            /** <= write function */
   ciaaDriverUart_ioctl,            /** <= ioctl function */
   NULL,                            /** <= seek function is not provided */
   NULL,                            /** <= upper layer */
   (void*)&ciaaDriverUart_uart0,    /** <= layer */
   (void*)&ciaaDriverUart_server0   /** <= server handlers as lower layer */
};

/** \brief Device for UART 1 */
static ciaaDevices_deviceType ciaaDriverUart_device1 = {
   "uart/1",                        /** <= driver name */
   ciaaDriverUart_open,             /** <= open function */
   ciaaDriverUart_close,            /** <= close function */
   ciaaDriverUart_read,             /** <= read function */
   ciaaDriverUart_write,            /** <= write function */
   ciaaDriverUart_ioctl,            /** <= ioctl function */
   NULL,                            /** <= seek function is not provided */
   NULL,                            /** <= upper layer */
   (void*)&ciaaDriverUart_uart1,    /** <= layer */
   (void*)&ciaaDriverUart_server1   /** <= server handlers as lower layer */
};

static ciaaDevices_deviceType * const ciaaUartDevices[] = {
   &ciaaDriverUart_device0,
   &ciaaDriverUart_device1
};

static ciaaDriverConstType const ciaaDriverUartConst = {
   ciaaUartDevices,
   2
};

/*==================[external data definition]===============================*/
/** \brief Uart 0 */
ciaaDriverUart_uartType ciaaDriverUart_uart0;

/** \brief Uart 1 */
ciaaDriverUart_uartType ciaaDriverUart_uart1;

/*==================[internal functions definition]==========================*/
static void ciaaDriverUart_rxIndication(ciaaDevices_deviceType const * const device)
{
   /* receive the data and forward to upper layer */
   ciaaDriverUart_uartType * uart = device->layer;

   ciaaSerialDevices_rxIndication(device->upLayer, uart->rxBuffer.length);
}

static void ciaaDriverUart_txConfirmation(ciaaDevices_deviceType const * const device)
{
   /* receive the data and forward to upper layer */
   ciaaDriverUart_uartType * uart = device->layer;

   ciaaSerialDevices_txConfirmation(device->upLayer, uart->txBuffer.length);
}

int ciaaDriverUart_configureScokect(int socket) {
   int result;

   result = fcntl(socket, F_SETOWN, getpid());
   if (result < 0)
   {
      perror("Error setting signal process owner: ");
   }
   if (result >= 0) {
      result = fcntl(socket, F_GETFL, 0);
      if (result < 0)
      {
         perror("Error getting socket flags: ");
      }
      else
      {
         result = fcntl(socket, F_SETFL, result | O_ASYNC);
         if (result < 0)
         {
            perror("Error setting socket asincronous flags: ");
         }
      }
   }
   return (result);
}

static ciaaDevices_deviceType * ciaaDriverUart_getDeviceServer(int fileDescriptor)
{
   ciaaDevices_deviceType * device;
   ciaaDriverUart_serverType * server;
   bool found = false;
   int index;

   for(index = 0; index < ciaaDriverUartConst.countOfDevices; index++)
   {
      device = ciaaUartDevices[index];
      server = device->loLayer;
      if (fileDescriptor == server->serverSocket)
      {
         found = true;
         break;
      }
   }
   if (!found) device = NULL;

   return device;
}

static void ciaaDriverUart_signalHandler(int signal, siginfo_t * info, void * context)
{
   ciaaDevices_deviceType * device = ciaaUartDevices[0];
   ciaaDriverUart_uartType * uart = device->layer;
   ciaaDriverUart_serverType * server = device->loLayer;
   int result;

	printf("Signal received\r\n");   
   if (server)
   {
		if (!server->conected)
      {
         server->clientDataSize = sizeof(server->clientData);
         server->clientSocket = accept(server->serverSocket, (struct sockaddr *) &(server->clientData), &(server->clientDataSize));
         if (server->clientSocket > 0) {
				ciaaDriverUart_configureScokect(server->clientSocket);
				printf("Client Conected\r\n");
				server->conected = 1;
			}
		} 
		else
		{
			result = recv(server->clientSocket, uart->rxBuffer.buffer, sizeof(uart->rxBuffer), MSG_DONTWAIT);
			if (result < 0) {
				printf("Client disconected\r\n");
				server->conected = 0;
			} else if (result > 0) {
				uart->rxBuffer.length = result;
				ciaaDriverUart_rxIndication(device);
			}
			//result = send(server->clientSocket, uart->txBuffer.buffer, uart->txBuffer.length, MSG_DONTWAIT);
			//uart->txBuffer.length = 0;
		}
   }
}

/*==================[external functions definition]==========================*/
extern ciaaDevices_deviceType * ciaaDriverUart_open(char const * path,
      ciaaDevices_deviceType * device, uint8_t const oflag)
{
   char buffer[128];
   int deviceIndex;
   int result;
   ciaaDriverUart_serverType * server = device->loLayer;

   if (device == &ciaaDriverUart_device0)
   {
      deviceIndex = 0;
   }
   else if (device == &ciaaDriverUart_device1)
   {
      deviceIndex = 1;
   }
   else
   {
      deviceIndex = -1;
   }

   PreCallService();
   if (deviceIndex == 0) {
      bzero(server, sizeof(*server));

      server->signalAction.sa_sigaction = &ciaaDriverUart_signalHandler;
      server->signalAction.sa_flags = SA_SIGINFO;

      result = sigaction(SIGIO, &server->signalAction, NULL);
      if (result < 0)
      {
         perror("Error setting signal handler: ");
      }
      else
      {
         result = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
         if (result > 0)
         {
            server->serverSocket = result;
         }
         else
         {
            perror("Error creating server socket: ");
         }
      }

      if (server->serverSocket) {
         result = ciaaDriverUart_configureScokect(server->serverSocket);
      }

      if (result >= 0)
      {
         server->serverData.sin_family = AF_INET;
         server->serverData.sin_addr.s_addr = INADDR_ANY;
         server->serverData.sin_port = htons(2000 + deviceIndex);

         result = bind(server->serverSocket,(struct sockaddr *) &(server->serverData), sizeof(server->serverData));

         if (result < 0)
         {
            perror("Error binding socket address: ");
         }
         else
         {
            result = listen(server->serverSocket, 1);
            if (result < 0) perror("Error listen on socket: ");
         }
      }
   }
   PostCallService();
   return device;
}

extern int32_t ciaaDriverUart_close(ciaaDevices_deviceType const * const device)
{
//   ciaaDriverUart_pipesType * pipes = device->loLayer;

   //PreCallService();
   //fclose(pipes->read);
   //fclose(pipes->write);
   //PostCallService();
   return 0;
}

extern int32_t ciaaDriverUart_ioctl(ciaaDevices_deviceType const * const device, int32_t const request, void * param)
{
   int32_t ret = -1;

   if((device == ciaaDriverUartConst.devices[0]) ||
      (device == ciaaDriverUartConst.devices[1]) )
   {
      switch(request)
      {
         case ciaaPOSIX_IOCTL_STARTTX:
            ciaaDriverUart_txConfirmation(device);
         break;
      }
   }
   return ret;
}

extern int32_t ciaaDriverUart_read(ciaaDevices_deviceType const * const device, uint8_t* buffer, uint32_t size)
{
   ciaaDriverUart_uartType * uart = device->layer;

   /* receive the data and forward to upper layer */
   //uart->rxBuffer.length = fread(uart->rxBuffer.buffer, 1, sizeof(uart->rxBuffer), pipes->read);

   if (size > uart->rxBuffer.length)
   {
      size = uart->rxBuffer.length;
   }

   /* copy received bytes to upper layer */
   ciaaPOSIX_memcpy(buffer, &uart->rxBuffer.buffer[0], size);

   return size;
}

extern int32_t ciaaDriverUart_write(ciaaDevices_deviceType const * const device, uint8_t const * const buffer, uint32_t const size)
{
   int32_t ret;

   ciaaDriverUart_uartType * uart = device->layer;
   ciaaDriverUart_serverType * server = device->loLayer;

   /* show writed data */
   ciaaPOSIX_memcpy(&uart->txBuffer.buffer, buffer, size);
   uart->txBuffer.length = size;
   uart->txBuffer.buffer[uart->txBuffer.length] = 0;
   ciaaPOSIX_printf("Send Data: %s\r\n", uart->txBuffer.buffer);

   /* write data */
   //ret = send(server->clientSocket, uart->txBuffer.buffer, uart->txBuffer.length, 0);
   //if (ret < 0) server->conected = false;

   return ret;
}

void ciaaDriverUart_init(void)
{
   uint8_t loopi;

   /* add uart driver to the list of devices */
   for(loopi = 0; loopi < ciaaDriverUartConst.countOfDevices; loopi++) {
      /* add each device */
      ciaaSerialDevices_addDriver(ciaaDriverUartConst.devices[loopi]);
   }
}


/*==================[interrupt hanlders]=====================================*/
extern void ciaaDriverUart_uart0_rxIndication(void)
{
   ciaaDriverUart_rxIndication(&ciaaDriverUart_device0);
}

extern void ciaaDriverUart_uart0_txConfirmation(void)
{
   ciaaDriverUart_txConfirmation(&ciaaDriverUart_device0);
}

extern void ciaaDriverUart_uart1_rxIndication(void)
{
   ciaaDriverUart_rxIndication(&ciaaDriverUart_device1);
}

extern void ciaaDriverUart_uart1_txConfirmation(void)
{
   ciaaDriverUart_txConfirmation(&ciaaDriverUart_device1);
}
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/


