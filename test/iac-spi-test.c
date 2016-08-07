/*
 * Copyright (C) 2016 Libre Space Foundation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <linux/spi/spidev.h>
#include "iac.h"
#include "spi.h"

int verbose = 0;

int main(int argc, char **argv)
{
    char *device = IAC_SPI_DEFAULT_DEVICE;
    iac_spi_init_params_t params = {
        IAC_SPI_MODE,
        IAC_SPI_BITS,
        IAC_SPI_MAX_HZ,
    };
    int fd;
    unsigned char blob[16] = {
        0x00, 0x00, 0x00, 0x0C,
        0xFE, 0xED, 0xDE, 0xAD,
        0xBE, 0xEF, 0xFE, 0xED,
        0xDE, 0xAD, 0xBE, 0xEF,
    };

    /* Initialize SPI */
    fd = iac_spi_init(device, &params);
    if (fd == -1)
        return IAC_FAILURE;

    /* Write to SPI */
    if (iac_spi_transfer(fd, blob, sizeof(blob)) == IAC_FAILURE)
        return IAC_FAILURE;

    /* Print response */
    IAC_VERBOSE("Response: 0x%02x\n", blob[0]);

    close(fd);
    return EXIT_SUCCESS;
}
