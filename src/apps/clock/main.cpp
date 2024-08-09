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

#include "app.h"

#include "drv_can_socketcan.h"

#include <string>
#include <cstring>
#include <vector>
#include <iostream>

/******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/


/******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

int main(int argc, const char *argv[]) /* main entry point for controller startup */
{
    std::string current_exec_name = argv[0]; // Name of the current exec program
    std::string arg1;

    if (argc == 2)
    {
        arg1 = argv[1];
    }
    else
    {
        std::cout << "Usage:" << std::endl;
        std::cout << argv[0] << " <can-interface-name>" << std::endl;
        return -1;
    }

    /****************************************AppStart();*****************************************/
    uint32_t ticks;

    /* Initialize the CANopen stack. Stop execution if an
     * error is detected.
     */
    CO_LNX_SKTCAN Linux_Socketcan_CanDriver;
    COLnxSktCanInit(&Linux_Socketcan_CanDriver, (char *)arg1.c_str());
    AppSpec.Drv->Can = &Linux_Socketcan_CanDriver.super;

    CONodeInit(&Node, &AppSpec);
    if (CONodeGetErr(&Node) != CO_ERR_NONE)
    {
        while (1)
            ; /* error not handled */
    }

    /* Use CANopen software timer to create a cyclic function
     * call to the callback function 'AppClock()' with a period
     * of 1s (equal: 1000ms).
     */
    ticks = COTmrGetTicks(&Node.Tmr, 1000, CO_TMR_UNIT_1MS);
    COTmrCreate(&Node.Tmr, 0, ticks, AppClock, &Node);

    /* Start the CANopen node and set it automatically to
     * NMT mode: 'OPERATIONAL'.
     */
    CONodeStart(&Node);
    CONmtSetMode(&Node.Nmt, CO_OPERATIONAL);

    /* In the background loop we execute elapsed action
     * callback functions.
     */
    while (1)
    {
        CONodeProcess(&Node);
        COTmrProcess(&Node.Tmr);
    }
}

/******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/
