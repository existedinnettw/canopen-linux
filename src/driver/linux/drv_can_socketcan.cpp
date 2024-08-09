/******************************************************************************
   @author Sandro Gort

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
******************************************************************************/

/******************************************************************************
* INCLUDES
******************************************************************************/

#include "drv_can_socketcan.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

/******************************************************************************
* PRIVATE TYPE DEFINITION
******************************************************************************/

/******************************************************************************
* PRIVATE DEFINES
******************************************************************************/

/******************************************************************************
* PRIVATE VARIABLES
******************************************************************************/
int CanSocket = -1;
char CanInterfaceName[32];

/******************************************************************************
* PRIVATE FUNCTIONS
******************************************************************************/

static void    DrvCanInit   (void);
static void    DrvCanEnable (uint32_t baudrate);
static int16_t DrvCanSend   (CO_IF_FRM *frm);
static int16_t DrvCanRead   (CO_IF_FRM *frm);
static void    DrvCanReset  (void);
static void    DrvCanClose  (void);

/******************************************************************************
* PUBLIC VARIABLE
******************************************************************************/

const CO_IF_CAN_DRV Linux_Socketcan_CanDriver = {
    DrvCanInit,
    DrvCanEnable,
    DrvCanRead,
    DrvCanSend,
    DrvCanReset,
    DrvCanClose
};

/******************************************************************************
* PUBLIC FUNCTIONS
******************************************************************************/

/******************************************************************************
* PRIVATE FUNCTIONS
******************************************************************************/

static void DrvCanInit(void)
{
    printf("%s\n", __func__);

    CanSocket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (CanSocket < 0) {
        printf("Creating CAN socket failed\n");
        while (1){} 
    }

    struct sockaddr_can addr {};
    addr.can_family = AF_CAN;
    addr.can_ifindex = if_nametoindex(CanInterfaceName);
    if (addr.can_ifindex == 0) {
        printf("Invalid Parameter, can interface name unknown (%s)\n",CanInterfaceName);
        while (1){} 
    }

    int fd = bind(CanSocket, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr));
    if (fd < 0) {
        printf("Open CAN socket %s failed\n", CanInterfaceName);
        while (1){} 
    }

    struct timeval tvRx {
        0, 1
    };
    setsockopt(CanSocket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char *>(&tvRx), sizeof tvRx);
    struct timeval tvTx {
        0, 100000
    };
    setsockopt(CanSocket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char *>(&tvTx),
             sizeof tvTx);

    printf("CAN socket initialization success\n");
}

static void DrvCanEnable(uint32_t baudrate)
{
    printf("%s\n", __func__);

    /* Currently CAN socket needs to be enabled beforehand via command line

    sudo modprobe vcan
    sudo ip link add dev can0 type vcan
    sudo ip link set up can0
    */
}

static int16_t DrvCanSend(CO_IF_FRM *frm)
{
    // std::cout << __func__ << std::endl;

    struct can_frame frame {};
    frame.can_id = frm->Identifier;
    frame.can_dlc = frm->DLC;
    for (uint8_t i = 0; i < 8; ++i) {
        frame.data[i] = frm->Data[i];
    }
    int16_t bytesWritten = write(CanSocket, &frame, sizeof(frame));
    if (bytesWritten < 0) {
        printf("DrvCanSend failed\n");
        return 0;
    }
    return sizeof(CO_IF_FRM);
}

static int16_t DrvCanRead (CO_IF_FRM *frm)
{
    //std::cout << __func__ << std::endl;

    struct can_frame frame {};
    auto bytesReceived = read(CanSocket, &frame, sizeof(frame));
    if (bytesReceived <= 0)
    {
        //std::cout << "DrvCanRead failed" << std::endl;
        return 0;
    }
    frm->Identifier = frame.can_id;
    frm->DLC = frame.can_dlc;
    for (uint8_t i = 0; i < 8; ++i) {
        frm->Data[i] = frame.data[i];
    }
    return sizeof(CO_IF_FRM);
}

static void DrvCanReset(void)
{
    printf("%s\n", __func__);

    DrvCanClose();
    DrvCanInit();
}

static void DrvCanClose(void)
{
    printf("%s\n", __func__);

    close(CanSocket);
}
