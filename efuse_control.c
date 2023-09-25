//------------------------------------------------------------------------------
/**
 * @file odroid_efuse_control.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID Mac Request server control application.
 * @version 0.2
 * @date 2023-09-18
 *
 * @package apt install python3 python3-pip
 *          python3 -m pip install aiohttp asyncio
 *
 * @copyright Copyright (c) 2022
 *
 */
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <getopt.h>

#include "lib_mac/lib_mac.h"
#include "lib_nlp/lib_nlp.h"
#include "lib_efuse/lib_efuse.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// 문자열 변경 함수. 입력 포인터는 반드시 메모리가 할당되어진 변수여야 함.
//------------------------------------------------------------------------------
static void tolowerstr (char *p)
{
    int i, c = strlen(p);

    for (i = 0; i < c; i++, p++)
        *p = tolower(*p);
}

//------------------------------------------------------------------------------
static void toupperstr (char *p)
{
    int i, c = strlen(p);

    for (i = 0; i < c; i++, p++)
        *p = toupper(*p);
}

//------------------------------------------------------------------------------
#define OPTION_SERVER_FACTORT   0x1
#define OPTION_EFUSE_UUID       0x2
#define OPTION_EFUSE_WRITE      0x4
#define OPTION_EFUSE_ERASE      0x8
#define OPTION_EFUSE_READ       0x10
#define OPTION_EFUSE_FORCE      0x20
#define OPTION_MAC_PRINT        0x40

unsigned int OPT_CONTROL = 0;
const char *OPT_BOARD_NAME = "m1s";
// 3aaef75c-a164-433b-8d34-001e06530012
const char *OPT_EFUSE_UUID = NULL;

static void print_usage(const char *prog)
{
    puts("");
    printf("Usage: %s [-F | -D] [-B:Board name] [-e | -w | -f | -r | -p]] [ -u:uuid(-w | -f )]\n", prog);
    puts("");
    puts("  -B --board_name     request board name.(default m1s)\n"
         "  -D --developer      developer server ctrl (default)\n"
         "  -F --factory        factory server ctrl\n"
         "  -e --efuse_erase    efuse erase\n"
         "  -w --efuse_write    efuse write\n"
         "  -u --efuse_uuid     efuse uuid data (if none, get uuid from the server)\n"
         "  -f --force_write    force efuse write\n"
         "  -r --efuse_read     efuse data read & display\n"
         "  -p --mac_print      print mac address\n"
         "\n"
         "  e.g) efuse write (with uuid)\n"
         "       efuse_control [-D:Developer | -F:Factory] -B [Board:m1s] -w -u [uuid]\n"
         "  e.g) efuse write (with server uuid)\n"
         "       efuse_control [-D:Developer | -F:Factory] -B [Board:m1s] -w\n"
         "  e.g) efuse erase\n"
         "       efuse_control [-D:Developer | -F:Factory] -B [Board:m1s] -e\n"
    );
    exit(1);
}

//------------------------------------------------------------------------------
static void parse_opts (int argc, char *argv[])
{
    while (1) {
        static const struct option lopts[] = {
            { "board_name",  1, 0, 'B' },
            { "developer",   0, 0, 'D' },
            { "factory",     0, 0, 'F' },
            { "efuse_erase", 0, 0, 'e' },
            { "efuse_write", 0, 0, 'w' },
            { "efuse_uuid",  1, 0, 'u' },
            { "efuse_force", 0, 0, 'f' },
            { "efuse_read" , 0, 0, 'r' },
            { "mac_print",   0, 0, 'p' },
            { NULL, 0, 0, 0 },
        };
        int c;

        c = getopt_long(argc, argv, "B:DFewu:frp", lopts, NULL);

        if (c == -1)
            break;

        switch (c) {
        case 'F':
            OPT_CONTROL |=  OPTION_SERVER_FACTORT;
            break;
        case 'D':
            OPT_CONTROL &= ~OPTION_SERVER_FACTORT;
            break;
        case 'B':
            OPT_BOARD_NAME = optarg;
            break;
        case 'e':
            OPT_CONTROL |=  OPTION_EFUSE_ERASE;
            break;
        case 'w':
            OPT_CONTROL |=  OPTION_EFUSE_WRITE;
            break;
        case 'u':
            // uuid (36bytes) : 3aaef75c-a164-433b-8d34-001e06530012
            OPT_CONTROL |=  OPTION_EFUSE_UUID;
            toupperstr (optarg);
            OPT_EFUSE_UUID = optarg;
            break;
        case 'f':
            OPT_CONTROL |= OPTION_EFUSE_FORCE;
            break;
        case 'r':
            OPT_CONTROL |= OPTION_EFUSE_READ;
            break;
        case 'p':
            OPT_CONTROL |= OPTION_MAC_PRINT;
            break;
        default:
            print_usage(argv[0]);
            break;
        }
    }
}

//------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    char mac_uuid[EFUSE_SIZE_M1S +1], mac[MAC_STR_SIZE+1];
    struct nlp_info nlp;

    parse_opts(argc, argv);

    if (argc < 2)
        print_usage(argv[0]);

    memset (&nlp, 0, sizeof(nlp));
    if (OPT_CONTROL & OPTION_EFUSE_FORCE) {
        OPT_CONTROL |= OPTION_EFUSE_ERASE;
        OPT_CONTROL |= OPTION_EFUSE_WRITE;
    }

    if (OPT_CONTROL & OPTION_EFUSE_READ) {
        memset (mac_uuid, 0, sizeof(mac_uuid));
        if (efuse_control (mac_uuid, EFUSE_READ)) {
            memset (mac,  0, sizeof(mac));
            efuse_get_mac (mac_uuid, mac);
            printf ("EFUSE Read = %s, MAC = %s\n", mac_uuid, mac);
        }
    }
    if (OPT_CONTROL & OPTION_EFUSE_ERASE) {
        memset (mac_uuid, 0, sizeof(mac_uuid));
        if (efuse_control (mac_uuid, EFUSE_ERASE))
            printf ("EFUSE Erase.\n");
    }

    if (OPT_CONTROL & OPTION_EFUSE_WRITE) {
        memset (mac_uuid, 0, sizeof(mac_uuid));
        if (efuse_control (mac_uuid, EFUSE_READ)) {
            if (efuse_valid_check (mac_uuid)) {
                printf ("EFUSE has already been written. %s.\n", mac_uuid);
            }  else {
                if (OPT_EFUSE_UUID != NULL) {
                    memset (mac_uuid, 0, sizeof(mac_uuid));
                    strncpy (mac_uuid, OPT_EFUSE_UUID, strlen(OPT_EFUSE_UUID));
                    if (efuse_control (mac_uuid, EFUSE_WRITE))
                        printf ("EFUSE write %s.\n", mac_uuid);
                } else {
                    int server = OPT_CONTROL & OPTION_SERVER_FACTORT ?
                                    MAC_SERVER_FACTORY : MAC_SERVER_DEVELOPER;

                    memset (mac_uuid, 0, sizeof(mac_uuid));
                    if (mac_server_request (server, REQ_TYPE_UUID, OPT_BOARD_NAME, mac_uuid)) {
                        toupperstr(mac_uuid);
                        if (efuse_valid_check (mac_uuid)) {
                            if (efuse_control (mac_uuid, EFUSE_WRITE))
                                printf ("EFUSE write %s.(mac_server %s)\n",
                                    mac_uuid, server ? "FACTORY" : "DEVELOPER");
                        }
                    }
                }
            }
        }
    }

    if (OPT_CONTROL & OPTION_MAC_PRINT) {
        if (nlp_init (&nlp, NULL)) {
            memset (mac,  0, sizeof(mac));
            efuse_get_mac (mac_uuid, mac);
            if (nlp_printf (&nlp, MSG_TYPE_MAC, mac, 0))
                printf ("mac address (%s) printed.\n", mac);
        }
    }
    return 0;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
