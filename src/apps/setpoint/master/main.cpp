#include "app.h"

#include "drv_can_socketcan.h"

#include <string>
#include <cstring>
#include <vector>
#include <iostream>

int main(int argc, const char *argv[])
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
