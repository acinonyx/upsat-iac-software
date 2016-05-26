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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <m3api/xiApi.h>
#include <wand/magick_wand.h>
#include "iac.h"
#include "camera.h"
#include "image.h"
#include "spi.h"

#define IAC_VERSION                     "0.1.0"

static int usage(const char *, const char *);

static int usage(const char *name, const char *version)
{
    fprintf(stderr, "Image acquisition controller utility, version %s\n"
            "\n"
            "Usage: %s [OPTIONS]...\n"
            "Options:\n"
            "        -e <exposure>           Set camera exposure time in microseconds\n"
            "        -g <gain>               Set camera gain in dB\n"
            "        -w                      Enable camera automatic white balance\n"
            "        -D <spidev>             SPI device to use (default: %s)\n"
            "\n", version, name, IAC_SPI_DEFAULT_DEVICE);

    return IAC_SUCCESS;
}

int main(int argc, char **argv)
{
    HANDLE cam_handle;
    iac_cam_init_params_t cam_init_params;
    XI_IMG cam_image;
    iac_image_read_params_t image_read_params;
    MagickWand *image_wand;
    MagickWand ***tile_wands;
    iac_spi_init_params_t spi_init_params;
    char *spi_device = IAC_SPI_DEFAULT_DEVICE;
    int opt;
    int fd;
    int i, j;
    unsigned char *tile_blob;
    size_t tile_data_size;

    /* Parse command line options */
    memset(&cam_init_params, 0, sizeof(cam_init_params));
    while ((opt = getopt(argc, argv, "e:g:D:w")) != -1) {
        switch (opt) {
        case 'e':
            cam_init_params.exposure = atoi(optarg);
            break;
        case 'g':
            cam_init_params.gain = atof(optarg);
            break;
        case 'w':
            cam_init_params.auto_wb = 1;
            break;
        case 'D':
            spi_device = optarg;
            break;
        default:
            return usage(argv[0], IAC_VERSION);
        }
    }

    /* Open camera */
    if (iac_cam_open(&cam_handle) == IAC_FAILURE)
        return EXIT_FAILURE;

    /* Initialize camera */
    if (iac_cam_init(&cam_handle, &cam_init_params) == IAC_FAILURE) {
        fprintf(stderr, "Unable to initialize camera!\n");
        iac_cam_close(&cam_handle);
        return EXIT_FAILURE;
    }

    /* Acquire image from camera */
    memset(&cam_image, 0, sizeof(cam_image));
    cam_image.size = sizeof(cam_image);

    if (iac_cam_acquire(&cam_handle, &cam_image) == IAC_FAILURE) {
        fprintf(stderr, "Unable to acquire image!\n");
        iac_cam_close(&cam_handle);
        return EXIT_FAILURE;
    }

    /* Read camera image into wand */
    image_read_params.width = cam_image.width;
    image_read_params.height = cam_image.height;
    image_read_params.format = IAC_IMAGE_FORMAT;
    image_read_params.depth = IAC_IMAGE_DEPTH;
    iac_image_init();
    image_wand = iac_image_read(&image_read_params,
                                (const unsigned char *) cam_image.bp,
                                (const size_t) cam_image.bp_size);
    tile_wands = iac_image_tiles(image_wand, IAC_IMAGE_DIVS);

    /* Initialize SPI */
    fd = iac_spi_init(spi_device, &spi_init_params);
    if (fd == -1)
        return EXIT_FAILURE;

    /* Write tiles to SPI */
    for (i = 0; i < IAC_IMAGE_DIVS; i++) {
        for (j = 0; j < IAC_IMAGE_DIVS; j++) {
            tile_blob = iac_image_blob(tile_wands[i][j], &tile_data_size);
            if (tile_blob == NULL)
                return EXIT_FAILURE;
            tile_data_size = htonl((uint32_t) tile_data_size);
            iac_spi_transfer(fd, (uint8_t *) &tile_data_size, sizeof(&tile_data_size));
            iac_spi_transfer(fd, tile_blob, (uint32_t) tile_data_size);
        }
    }

    /* Terminate image */
    iac_image_term();

    /* Close camera */
    if (iac_cam_close(&cam_handle) == IAC_FAILURE) {
        fprintf(stderr, "Unable to close camera!\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

