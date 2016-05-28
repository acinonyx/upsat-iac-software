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
#include <linux/spi/spidev.h>
#include <m3api/xiApi.h>
#include <wand/magick_wand.h>
#include "iac.h"
#include "camera.h"
#include "image.h"
#include "spi.h"

#define IAC_VERSION                     "0.1.0"

typedef struct config_t {
    int exposure;
    double gain;
    int auto_wb;
    char *spi_device;
} config_t;

static int usage(const char *, const char *);
static config_t parse_args(int, char **);
static HANDLE get_image(XI_IMG *, const config_t *);
static MagickWand ***create_tiles(const XI_IMG *, const config_t *);
static int transfer_tiles(MagickWand ***, const config_t *);

static int usage(const char *name, const char *version)
{
    fprintf(stderr, "Image acquisition controller utility, version %s\n"
            "\n"
            "Usage: %s [OPTIONS]...\n"
            "Options:\n"
            "  -e, --exposure=EXPOSURE       Set camera exposure time in microseconds\n"
            "  -g, --gain=GAIN               Set camera gain in dB\n"
            "  -w                            Enable camera automatic white balance\n"
            "  -D, --spi-dev=DEVICE          SPI device to use (default: %s)\n"
            "  --help                        Display help and exit\n"
            "  --version                     Output version and exit\n"
            "\n", version, name, IAC_SPI_DEFAULT_DEVICE);

    return IAC_SUCCESS;
}


static config_t parse_args(int argc, char **argv)
{
    int opt;
    config_t config;
    static struct option long_options[] = {
        { "exposure", required_argument, 0 , 'e' },
        { "gain", required_argument, 0, 'g' },
        { "auto-wb", no_argument, 0, 'w' },
        { "spi-device", required_argument, 0, 'D' },
        { "help", no_argument, 0, 0 },
        { "version", no_argument, 0, 0 },
        { 0, 0, 0, 0 }
    };
    int long_index = 0;

    memset(&config, 0, sizeof(config));

    /* Parse command line options */
    while ((opt = getopt_long(argc,
                              argv,
                              "e:g:D:w",
                              long_options,
                              &long_index)) != -1) {
        switch (opt) {
        case 0:
            switch (long_index) {
            case 5:
                fprintf(stderr,
                        "Image acquisition controller utility, version %s\n",
                        IAC_VERSION);
                exit(EXIT_SUCCESS);
                break;
            default:
                exit(usage(argv[0], IAC_VERSION));
            }
            break;
        case 'e':
            config.exposure = atoi(optarg);
            break;
        case 'g':
            config.gain = atof(optarg);
            break;
        case 'w':
            config.auto_wb = 1;
            break;
        case 'D':
            config.spi_device = optarg;
            break;
        default:
            exit(usage(argv[0], IAC_VERSION));
        }
    }

    return config;
}


static HANDLE get_image(XI_IMG *image, const config_t *config)
{
    HANDLE handle;
    iac_cam_init_params_t init_params = {
        config->exposure,
        config->gain,
        config->auto_wb,
    };

    /* Open camera */
    if (iac_cam_open(&handle) == IAC_FAILURE)
        exit(EXIT_FAILURE);

    /* Initialize camera */
    if (iac_cam_init(&handle, &init_params) == IAC_FAILURE) {
        fprintf(stderr, "Unable to initialize camera!\n");
        iac_cam_close(&handle);
        exit(EXIT_FAILURE);
    }

    /* Acquire image from camera */
    memset(image, 0, sizeof(XI_IMG));
    image->size = sizeof(XI_IMG);

    if (iac_cam_acquire(&handle, image) == IAC_FAILURE) {
        fprintf(stderr, "Unable to acquire image!\n");
        iac_cam_close(&handle);
        exit(EXIT_FAILURE);
    }

    return handle;
}


static MagickWand ***create_tiles(const XI_IMG *image, const config_t *config)
{
    MagickWand *wand;
    MagickWand ***wands;
    iac_image_read_params_t params = {
        image->width,
        image->height,
        IAC_IMAGE_FORMAT,
        IAC_IMAGE_DEPTH,
    };

    /* Read camera image into wand */
    iac_image_init();
    wand = iac_image_read_blob(&params,
                          (const unsigned char *) image->bp,
                          (const size_t) image->bp_size);
    wands = iac_image_tiles(wand, IAC_IMAGE_DIVS);

    return wands;
}


static int transfer_tiles(MagickWand ***wands, const config_t *config)
{
    char *device = IAC_SPI_DEFAULT_DEVICE;
    iac_spi_init_params_t params = {
        IAC_SPI_MODE,
        IAC_SPI_BITS,
        IAC_SPI_MAX_HZ,
    };
    int fd;
    int i, j;
    size_t size;
    unsigned char *blob;

    /* Initialize SPI */
    fd = iac_spi_init(device, &params);
    if (fd == -1)
        return IAC_FAILURE;

    /* Write tiles to SPI */
    for (i = 0; i < IAC_IMAGE_DIVS; i++) {
        for (j = 0; j < IAC_IMAGE_DIVS; j++) {
            blob = iac_image_blob(wands[i][j], &size);
            if (blob == NULL)
                return IAC_FAILURE;
            size = htonl((uint32_t) size);
            if (iac_spi_transfer(fd,
                                 (uint8_t *) &size,
                                 sizeof(&size)) == IAC_FAILURE)
                return IAC_FAILURE;
            if (iac_spi_transfer(fd, blob, (uint32_t) size) == IAC_FAILURE)
                return IAC_FAILURE;
            MagickRelinquishMemory(blob);
        }
    }

    return IAC_SUCCESS;
}


int main(int argc, char **argv)
{
    HANDLE handle;
    XI_IMG image;
    config_t config;
    MagickWand ***wands;

    config = parse_args(argc, argv);
    handle = get_image(&image, &config);
    wands = create_tiles(&image, &config);
    if (transfer_tiles(wands, &config) == IAC_FAILURE) {
        fprintf(stderr, "Failed to transfer tiles!\n");
        return EXIT_FAILURE;
    }

    iac_image_tiles_destroy(wands, IAC_IMAGE_DIVS);

    /* Terminate image */
    iac_image_term();

    /* Close camera */
    if (iac_cam_close(&handle) == IAC_FAILURE) {
        fprintf(stderr, "Unable to close camera!\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

