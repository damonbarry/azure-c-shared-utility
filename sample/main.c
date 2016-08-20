#include <stdbool.h>
#include <stdint.h>
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/tlsio.h"

bool should_quit = false;
int exit_value = -1;

void on_io_open_complete(void* context, IO_OPEN_RESULT open_result)
{
    if (open_result == IO_OPEN_OK) printf("connection is open!\n");
    else printf("error!\n");

    int onoff = 1;
    int time = 5;
    int interval = 5;
    int result = xio_setoption(context, "tcp_keepalive", &onoff);
    if (!!result) printf("error (%d) setting tcp_keepalive\n", result);
    result = xio_setoption(context, "tcp_keepalive_time", &time);
    if (!!result) printf("error (%d) setting tcp_keepalive\n", result);
    result = xio_setoption(context, "tcp_keepalive_interval", &interval);
    if (!!result) printf("error (%d) setting tcp_keepalive\n", result);

    exit_value = open_result;
    should_quit = true;
}

int main()
{
    size_t counter = SIZE_MAX;

    if (platform_init() == 0) {
        TLSIO_CONFIG tls_io_config = { "google.com", 443 };
        const IO_INTERFACE_DESCRIPTION* tlsio_interface = platform_get_default_tlsio();

        XIO_HANDLE tls_io = xio_create(tlsio_interface, &tls_io_config);
        if (tls_io != NULL) {
            if (xio_open(tls_io, on_io_open_complete, tls_io, NULL, NULL, NULL, NULL) == 0) {
                while (!should_quit || counter > 0) {
                    if (should_quit && counter < SIZE_MAX) --counter;
                    if (should_quit && counter == SIZE_MAX) counter = 15;
                    xio_dowork(tls_io);
                    ThreadAPI_Sleep(1000);
                }
            }
        }

        platform_deinit();
    }

	return exit_value;
}
