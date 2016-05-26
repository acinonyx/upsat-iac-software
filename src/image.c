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
#include <wand/magick_wand.h>
#include "iac.h"
#include "image.h"

static void iac_image_exception(const MagickWand *);

static void iac_image_exception(const MagickWand *wand)
{
    char *desc;
    ExceptionType severity;

    desc = MagickGetException(wand, &severity);
    fprintf(stderr, "%s %s %ld %s\n", GetMagickModule(), desc);
    desc = (char *) MagickRelinquishMemory(desc);

}


void iac_image_init(void)
{

    MagickWandGenesis();

}


void iac_image_term(void)
{

    MagickWandTerminus();

}


MagickWand *iac_image_read(const iac_image_read_params_t *params, const unsigned char *data, const size_t data_size)
{
    MagickWand *wand = NULL;

    /* Create wand */
    wand = NewMagickWand();
    if (!wand) {
        iac_image_destroy(wand);
        return(NULL);
    }

    /* Set width and height of image */
    if (MagickSetSize(wand, params->width, params->height) == MagickFalse) {
        iac_image_exception(wand);
        iac_image_destroy(wand);
        return(NULL);
    }

    /* Set format of image */
    if (MagickSetFormat(wand, params->format) == MagickFalse) {
        iac_image_exception(wand);
        iac_image_destroy(wand);
        return(NULL);
    }

    /* Set color depth of image */
    if (MagickSetDepth(wand, params->depth) == MagickFalse) {
        iac_image_exception(wand);
        iac_image_destroy(wand);
        return(NULL);
    }

    /* Read blob into wand */
    if (MagickReadImageBlob(wand, data, data_size) == MagickFalse) {
        iac_image_exception(wand);
        iac_image_destroy(wand);
        return(NULL);
    }

    return(wand);
}


void iac_image_destroy(MagickWand *wand)
{

    if (wand)
        DestroyMagickWand(wand);

}


MagickWand ***iac_image_tiles(MagickWand *wand, const unsigned int divs)
{

    MagickWand ***mwtiles = NULL;
    size_t width;
    size_t height;
    unsigned int i, j;

    /* Calculate width and height of tile */
    width = MagickGetImageWidth(wand);
    width = width / divs + (width % divs ? 1 : 0);
    height = MagickGetImageHeight(wand);
    height = height / divs + (height % divs ? 1 : 0);

    /* Allocate memory for rows */
    mwtiles = malloc(sizeof(MagickWand **) * divs);

    for (i = 0; i < divs; i++) {

        /* Allocate memory for columns */
        mwtiles[i] = malloc(sizeof(MagickWand *) * divs);
        for (j = 0; j < divs; j++) {
            mwtiles[i][j] = NULL;

            /* Crop image to tiles */
            mwtiles[i][j] = CloneMagickWand(wand);
            if (MagickCropImage(mwtiles[i][j],
                                width,
                                height,
                                (ssize_t) (j * width),
                                (ssize_t) (i * height)) == MagickFalse) {
                iac_image_exception(mwtiles[i][j]);
                iac_image_destroy(mwtiles[i][j]);
                /* XXX: Free previous wands */
                DestroyMagickWand(mwtiles[i][j]);
                free(mwtiles[i]);
                return NULL;
            }
        }
    }

    return mwtiles;
}


unsigned char *iac_image_blob(MagickWand *wand, size_t *data_size)
{
    unsigned char *blob;

    if(MagickSetImageFormat(wand, IAC_IMAGE_BLOB_FORMAT) == MagickFalse) {
        iac_image_exception(wand);
        return NULL;
    }

    blob = MagickGetImageBlob(wand, data_size);
    if(blob == NULL) {
        iac_image_exception(wand);
        return NULL;
    }

    return blob;
}