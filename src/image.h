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

#ifndef __IMAGE_H
#define __IMAGE_H

typedef struct iac_image_read_params_t {
    size_t width;
    size_t height;
    const char *format;
    size_t depth;
} iac_image_read_params_t;

void iac_image_init(void);
void iac_image_term(void);
void iac_image_destroy(MagickWand *);
MagickWand ***iac_image_tiles(MagickWand *, const unsigned int);
void iac_image_tiles_destroy(MagickWand ***, const unsigned int);
MagickWand *iac_image_read(const iac_image_read_params_t *, const unsigned char *, const size_t);
unsigned char *iac_image_blob(MagickWand *, size_t *);

#endif
