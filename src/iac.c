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
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/spi/spidev.h>
#include <linux/limits.h>
#include <m3api/xiApi.h>
#include <wand/magick_wand.h>
#include "iac.h"
#include "camera.h"
#include "image.h"
#include "spi.h"
#include "obc.h"

#define IAC_VERSION                     "0.2.0"

typedef struct config_t {
    char *input;
    size_t width;
    size_t height;
    char *output;
    char *prefix;
    int exposure;
    double gain;
    int auto_wb;
    char *spi_device;
    int verbose;
} config_t;

static int usage(const char *, const char *);
static config_t parse_args(int, char **);
static HANDLE get_cam_image(XI_IMG *, const config_t *);
static MagickWand ***tile_cam_image(const XI_IMG *, const config_t *);
static MagickWand ***tile_file_image(const config_t *);
static int transfer_tiles(MagickWand ***, const config_t *);
static int write_tiles(MagickWand ***, const config_t *);

static int usage(const char *name, const char *version)
{
    fprintf(stderr, "Image acquisition controller utility, version %s\n"
            "\n"
            "Usage: %s [OPTIONS]...\n"
            "Options:\n"
            "  -i, --input=FILE              Read raw image from file\n"
            "      --width=SIZE              Width of raw image file\n"
            "      --height=SIZE             Height of raw image file\n"
            "  -o, --output=DIRECTORY        Write tiles to directory\n"
            "      --prefix=NAME             Filename prefix for output tiles\n"
            "  -e, --exposure=EXPOSURE       Set camera exposure time in microseconds\n"
            "  -g, --gain=GAIN               Set camera gain in dB\n"
            "  -w                            Enable camera automatic white balance\n"
            "  -D, --spi-dev=DEVICE          SPI device to use (default: %s)\n"
            "  -v                            Verbose output\n"
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
        { "input", required_argument, 0, 'i' },
        { "width", required_argument, 0 , 0 },
        { "height", required_argument, 0 , 0 },
        { "output", required_argument, 0, 'o' },
        { "prefix", required_argument, 0, 0 },
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
                              "i:o:e:g:D:w",
                              long_options,
                              &long_index)) != -1) {
        switch (opt) {
        case 0:
            switch (long_index) {
            case 1:
                config.width = (size_t) atoi(optarg);
                break;
            case 2:
                config.height = (size_t) atoi(optarg);
                break;
            case 4:
                config.prefix = optarg;
                break;
            case 10:
                fprintf(stderr,
                        "Image acquisition controller utility, version %s\n",
                        IAC_VERSION);
                exit(EXIT_SUCCESS);
                break;
            default:
                exit(usage(argv[0], IAC_VERSION));
            }
            break;
        case 'i':
            config.input = optarg;
            break;
        case 'o':
            config.output = optarg;
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
        case 'v':
            config.verbose = 1;
            break;
        default:
            exit(usage(argv[0], IAC_VERSION));
        }
    }

    /* Validation */
    if (config.input && !(config.width && config.height)) {
        fprintf(stderr, "Width and height of image must be specified!\n");
        exit(EXIT_FAILURE);
    }

    if (config.output && !(config.prefix)) {
        fprintf(stderr, "Filename prefix must be specified!\n");
        exit(EXIT_FAILURE);
    }

    return config;
}


static HANDLE get_cam_image(XI_IMG *image, const config_t *config)
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


static MagickWand ***tile_cam_image(const XI_IMG *image, const config_t *config)
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


static MagickWand ***tile_file_image(const config_t *config)
{
    MagickWand *wand;
    MagickWand ***wands;
    iac_image_read_params_t params = {
        config->width,
        config->height,
        IAC_IMAGE_FORMAT,
        IAC_IMAGE_DEPTH,
    };

    /* Read file image into wand */
    iac_image_init();
    wand = iac_image_read_file(&params, config->input);
    wands = iac_image_tiles(wand, IAC_IMAGE_DIVS);

    return wands;
}


static int write_tiles(MagickWand ***wands, const config_t *config)
{
    int i, j;
    char filename[PATH_MAX];

    /* Write tiles to files */
    for (i = 0; i < IAC_IMAGE_DIVS; i++) {
        for (j = 0; j < IAC_IMAGE_DIVS; j++) {
            /* Set image format of files */
            if (MagickSetImageFormat(wands[i][j],
                                     IAC_IMAGE_BLOB_FORMAT) == MagickFalse) {
                iac_image_exception(wands[i][j]);
                return IAC_FAILURE;
            }
            snprintf(filename,
                     PATH_MAX,
                     "%s/%s-%u-%u.%s",
                     config->output,
                     config->prefix,
                     i,
                     j,
                     IAC_IMAGE_BLOB_FORMAT);
            if (MagickWriteImage(wands[i][j], filename) == MagickFalse) {
                iac_image_exception(wands[i][j]);
                return IAC_FAILURE;
            }
        }
    }

    return IAC_SUCCESS;
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
    iac_obc_block_t block;
    iac_obc_packet_t packet;
    size_t size, mod;
    size_t k, blocks;
    uint16_t blocks_data;
    uint8_t resp;
    unsigned char *blob;

    /* Initialize SPI */
    fd = iac_spi_init(device, &params);
    if (fd == -1)
        return IAC_FAILURE;

    /* Write tiles to SPI */
    for (i = 0; i < IAC_IMAGE_DIVS; i++) {
        for (j = 0; j < IAC_IMAGE_DIVS; j++) {
            blob = iac_image_get_blob(wands[i][j], &size);
            if (blob == NULL)
                return IAC_FAILURE;

            mod = size % IAC_OBC_BLOCK_SIZE;
            block.tile = (uint8_t) (j + i * IAC_IMAGE_DIVS);
            blocks = ((size - 1) / IAC_OBC_BLOCK_SIZE) + 1;

            for (k = 0; k <= blocks; k++) {
                block.index = (uint16_t) k;
                switch (k) {
                case 0:
                    /* Transfer number of tile blocks */
                    blocks_data = htons((uint16_t) blocks);
                    block.data = (uint8_t *) &blocks_data;
                    block.data_size = sizeof(blocks_data);
                    break;
                case 1:
                    /* First block of tile */
                    block.data = blob;
                default:
                    /* Transfer tile block */
                    if (k == blocks && mod)
                        block.data_size = mod;
                    else
                        block.data_size = IAC_OBC_BLOCK_SIZE;
                    break;
                }
                do {
                    usleep(IAC_OBC_BLOCK_USLEEP);
                    /* Pack tile block */
                    packet = iac_obc_packet(&block);
                    /* Transfer packets */
                    if (iac_spi_transfer(fd,
                                         packet.buf,
                                         (uint32_t) packet.size) == IAC_FAILURE)
                        return IAC_FAILURE;
                    resp = packet.buf[0];
                    free(packet.buf);
                } while (resp != IAC_OBC_BLOCK_ACK);
                /* Next block */
                block.data += IAC_OBC_BLOCK_SIZE;
            }

            MagickRelinquishMemory(blob);
        }
    }
    close(fd);

    return IAC_SUCCESS;
}


int main(int argc, char **argv)
{
    HANDLE handle;
    XI_IMG image;
    config_t config;
    MagickWand ***wands;

    config = parse_args(argc, argv);
    if (!config.input) {
        handle = get_cam_image(&image, &config);
        wands = tile_cam_image(&image, &config);
    }
    else {
        wands = tile_file_image(&config);
    }

    if (!config.output) {
        if (transfer_tiles(wands, &config) == IAC_FAILURE) {
            fprintf(stderr, "Failed to transfer tiles!\n");
            return EXIT_FAILURE;
        }
    }
    else {
        if (write_tiles(wands, &config) == IAC_FAILURE) {
            fprintf(stderr, "Failed to write tiles!\n");
            return EXIT_FAILURE;
        }
    }

    iac_image_tiles_destroy(wands, IAC_IMAGE_DIVS);

    /* Terminate image */
    iac_image_term();

    if (!config.input) {
        /* Close camera */
        if (iac_cam_close(&handle) == IAC_FAILURE) {
            fprintf(stderr, "Unable to close camera!\n");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

