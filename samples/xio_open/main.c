// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/tlsio.h"

typedef enum { init, opened, closing, closed } connection_progress;
connection_progress progress = init;
int exit_value = -1;
char* hostname = NULL;
int port = -1;
bool tls = false;

int parse_port(const char* buf)
{
    char* end = NULL;
    long val = strtol(buf, &end, 10);
    if (((val == LONG_MAX || val == LONG_MIN) && errno == ERANGE)   // out of range (long)
        || (val == 0 && end == buf)                                 // no conversion performed
        || (val != (int)val)                                        // out of range (int)
        || (val < 0))                                               // negative port value
    {
        return -1;
    }

    return (int)val;
}

bool parse_args(int argc, char *argv[])
{
    if (argc < 3 || argc > 4 || !argv[1] || !argv[2])
    {
        printf("usage: xio_open <hostname> <port> [tls]\n");
        return false;
    }

    int port_ = parse_port(argv[2]);
    if (port_ == -1)
    {
        printf("error: invalid port\n");
        return false;
    }

    bool tls_ = (argc == 4)
        && (strlen(argv[3]) == strlen("tls"))
        && (strcmp(argv[3], "tls") == 0);

    hostname = argv[1];
    port = (int)port_;
    tls = tls_;

    return true;
}

void on_io_close_complete(void* context)
{
    (void)context;
    printf("<< connection CLOSED\n");
    progress = closed;
}

void on_io_open_complete(void* context, IO_OPEN_RESULT open_result)
{
    (void)context;
    exit_value = open_result;
    printf(open_result == IO_OPEN_OK ? ">> connection OPEN\n" : "ERROR\n");
    progress = opened;
}

int main(int argc, char *argv[])
{
    if (!parse_args(argc, argv)) exit(1);

    if (platform_init() == 0)
    {
        TLSIO_CONFIG tls_io_config;
        tls_io_config.hostname = hostname;
        tls_io_config.port = port;
        const IO_INTERFACE_DESCRIPTION* socket_io_interface =
            tls ? platform_get_default_tlsio() : socketio_get_interface_description();

        XIO_HANDLE tls_io = xio_create(/*tlsio_interface*/socket_io_interface, &tls_io_config);
        if (tls_io != NULL)
        {
            printf("use TLS? %s\n", tls ? "yes" : "no");

            if (xio_open(tls_io, on_io_open_complete, tls_io, NULL, NULL, NULL, NULL) == 0)
            {
                while (progress != closed)
                {
                    if (progress == opened)
                    {
                        progress = closing;
                        int err = xio_close(tls_io, on_io_close_complete, NULL);
                        if (err)
                        {
                            progress = closed;
                            break;
                        }
                    }

                    xio_dowork(tls_io);
                    ThreadAPI_Sleep(1000);
                }
            }
        }

        platform_deinit();
    }

	return exit_value;
}
