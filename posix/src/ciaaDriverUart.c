/* Copyright 2014, ACSE & CADIEEL
 *    ACSE   : http://www.sase.com.ar/asociacion-civil-sistemas-embebidos/ciaa/
 *    CADIEEL: http://www.cadieel.org.ar
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
 ** Simulated UART Driver for Posix
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
#include "ciaaPOSIX_stdlib.h"

/*==================[macros and definitions]=================================*/
typedef struct  {
   ciaaDevices_deviceType const * const * const devices;
   uint8_t countOfDevices;
} ciaaDriverConstType;

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/
/** \brief Device for UART 0 */
static ciaaDevices_deviceType const ciaaDriverUart_device0 = {
   "uart/0",               /** <= driver name */
   ciaaDriverUart_open,    /** <= open function */
   ciaaDriverUart_close,   /** <= close function */
   ciaaDriverUart_read,    /** <= read function */
   ciaaDriverUart_write,   /** <= write function */
   ciaaDriverUart_ioctl,   /** <= ioctl function */
   NULL,                   /** <= seek function is not provided */
   NULL                    /** <= user specific area */
};

/** \brief Device for UART 1 */
static ciaaDevices_deviceType const ciaaDriverUart_device1 = {
   "uart/1",               /** <= driver name */
   ciaaDriverUart_open,    /** <= open function */
   ciaaDriverUart_close,   /** <= close function */
   ciaaDriverUart_read,    /** <= read function */
   ciaaDriverUart_write,   /** <= write function */
   ciaaDriverUart_ioctl,   /** <= ioctl function */
   NULL,                   /** <= seek function is not provided */
   NULL                    /** <= user specific area */
};

static ciaaDevices_deviceType const * const ciaaUartDevices[] = {
   &ciaaDriverUart_device0,
   &ciaaDriverUart_device1
};

static ciaaDriverConstType ciaaDriverUartConst = {
   ciaaUartDevices,
   2
};

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
extern int32_t ciaaDriverUart_open(ciaaDevices_deviceType const * const device, uint8_t const oflag)
{
   // TODO 
   return 0;
}

extern int32_t ciaaDriverUart_close(ciaaDevices_deviceType const * const device)
{
   // TODO
   return 0;
}

extern int32_t ciaaDriverUart_ioctl(ciaaDevices_deviceType const * const device, int32_t const request, void * param)
{
   // TODO
   return 0;
}

extern int32_t ciaaDriverUart_read (ciaaDevices_deviceType const * const device, uint8_t* buffer, uint32_t size)
{
   // TODO
   return 0;
}

extern int32_t ciaaDriverUart_write(ciaaDevices_deviceType const * const device, uint8_t const * const buffer, uint32_t const size)
{
   return 0;
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

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/


