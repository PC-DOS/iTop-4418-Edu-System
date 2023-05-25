#include <stdio.h>
#include <stdlib.h>

#include <libcec.h>

int main(int argc, char *argv[])
{
    if (!CECOpen()) {
        printf("failed to CECOpen()\n");
        return -1;
    }

    unsigned int paddr;
    if (!CECGetTargetCECPhysicalAddress(&paddr)) {
        printf("failed to CECGetTargetCECPhysicalAddress()\n");
        return -1;
    }

    printf("Device physical address is %X.%X.%X.%X\n",
          (paddr & 0xF000) >> 12, (paddr & 0x0F00) >> 8,
          (paddr & 0x00F0) >> 4, paddr & 0x000F);

    enum CECDeviceType devtype = CEC_DEVICE_PLAYER;
    int laddr = CECAllocLogicalAddress(paddr, devtype);
    if (!laddr) {
        printf("CECAllocLogicalAddress() failed!!!\n");
        return -1;
    }

    unsigned char *buffer = (unsigned char *)malloc(CEC_MAX_FRAME_SIZE);

    int size;
    unsigned char lsrc, ldst, opcode;

    while (true) {

        size = CECReceiveMessage(buffer, CEC_MAX_FRAME_SIZE, 100000);

        if (!size) { // no data available
            continue;
        }

        if (size == 1) continue; // "Polling Message"

        lsrc = buffer[0] >> 4;

        /* ignore messages with src address == laddr */
        if (lsrc == laddr) continue;

        opcode = buffer[1];

        if (opcode == CEC_OPCODE_REQUEST_ACTIVE_SOURCE) {
            printf("### ignore message...\n");
            continue;
        }

        if (CECIgnoreMessage(opcode, lsrc)) {
            printf("### ignore message coming from address 15 (unregistered) or ...\n");
            continue;
        }

        if (!CECCheckMessageSize(opcode, size)) {
            printf("### invalid message size ###\n");
            continue;
        }

        /* check if message broadcast/directly addressed */
        if (!CECCheckMessageMode(opcode, (buffer[0] & 0x0F) == CEC_MSG_BROADCAST ? 1 : 0)) {
            printf("### invalid message mode (directly addressed/broadcast) ###\n");
            continue;
        }

        ldst = lsrc;

//TODO: macroses to extract src and dst logical addresses
//TODO: macros to extract opcode

        switch (opcode) {

            case CEC_OPCODE_GIVE_PHYSICAL_ADDRESS:
            {
                /* responce with "Report Physical Address" */
                buffer[0] = (laddr << 4) | CEC_MSG_BROADCAST;
                buffer[1] = CEC_OPCODE_REPORT_PHYSICAL_ADDRESS;
                buffer[2] = (paddr >> 8) & 0xFF;
                buffer[3] = paddr & 0xFF;
                buffer[4] = devtype;
                size = 5;
                break;
            }

#if 0
            case CEC_OPCODE_SET_MENU_LANGUAGE:
            {
                printf("the menu language will be changed!!!\n");
                continue;
            }
#endif

            case CEC_OPCODE_REPORT_PHYSICAL_ADDRESS: // TV
            case CEC_OPCODE_ACTIVE_SOURCE:           // TV, CEC Switches
            case CEC_OPCODE_ROUTING_CHANGE:          // CEC Switches
            case CEC_OPCODE_ROUTING_INFORMATION:     // CEC Switches
            case CEC_OPCODE_SET_STREAM_PATH:         // CEC Switches
            case CEC_OPCODE_SET_SYSTEM_AUDIO_MODE:   // TV
            case CEC_OPCODE_DEVICE_VENDOR_ID:        // ???

            // messages we want to ignore
            case CEC_OPCODE_SET_MENU_LANGUAGE:
            {
                printf("### ignore message!!! ###\n");
                continue;
            }

            case CEC_OPCODE_DECK_CONTROL:
                if (buffer[2] == CEC_DECK_CONTROL_MODE_STOP) {
                    printf("### DECK CONTROL : STOP ###\n");
                }
                continue;

            case CEC_OPCODE_PLAY:
                if (buffer[2] == CEC_PLAY_MODE_PLAY_FORWARD) {
                    printf("### PLAY MODE : PLAY ###\n");
                }
                continue;

            case CEC_OPCODE_STANDBY:
                printf("### switching device into standby... ###\n");
                continue;

            case CEC_OPCODE_GIVE_DEVICE_POWER_STATUS:
            {
                /* responce with "Report Power Status" */
                buffer[0] = (laddr << 4) | ldst;
                buffer[1] = CEC_OPCODE_REPORT_POWER_STATUS;
                buffer[2] = 0x00; // "On"
                size = 3;
                break;
            }

//TODO: check
#if 0
            case CEC_OPCODE_REQUEST_ACTIVE_SOURCE:
            {
                /* responce with "Active Source" */
                buffer[0] = (laddr << 4) | CEC_MSG_BROADCAST;
                buffer[1] = CEC_OPCODE_ACTIVE_SOURCE;
                buffer[2] = (paddr >> 8) & 0xFF;
                buffer[3] = paddr & 0xFF;
                size = 4;
                break;
            }
#endif

            case CEC_OPCODE_REQUEST_ARC_INITIATION:
            {
		        // CTS: ignore request from non-adjacent device
		        if (paddr & 0x0F00) {
			        continue;
                }

                /* activate ARC Rx functionality */
                printf("info: enable ARC Rx channel!!!\n");
//TODO: implement
                /* responce with "Initiate ARC" */
                buffer[0] = (laddr << 4) | ldst;
                buffer[1] = CEC_OPCODE_INITIATE_ARC;
                size = 2;
                break;
            }

            case CEC_OPCODE_REPORT_ARC_INITIATED:
                continue;

            case CEC_OPCODE_REQUEST_ARC_TERMINATION:
            {
                /* de-activate ARC Rx functionality */
                printf("info: disable ARC Rx channel!!!\n");
//TODO: implement
                /* responce with "Terminate ARC" */
                buffer[0] = (laddr << 4) | ldst;
                buffer[1] = CEC_OPCODE_TERMINATE_ARC;
                size = 2;
                break;
            }

            case CEC_REPORT_ARC_TERMINATED:
                continue;

            default:
            {
                /* send "Feature Abort" */
                buffer[0] = (laddr << 4) | ldst;
                buffer[1] = CEC_OPCODE_FEATURE_ABORT;
                buffer[2] = CEC_OPCODE_ABORT;
                buffer[3] = 0x04; // "refused"
                size = 4;
            }
        }

//TODO: protect with mutex
        if (CECSendMessage(buffer, size) != size) {
            printf("CECSendMessage() failed!!!\n");
        }
    }

    if (buffer) {
        free(buffer);
    }

    if (!CECClose()) {
        printf("CECClose() failed!\n");
    }

    return 0;
}
