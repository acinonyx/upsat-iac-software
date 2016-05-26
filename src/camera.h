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

#ifndef __CAMERA_H
#define __CAMERA_H

typedef struct iac_cam_init_params_t {
    int exposure;
    double gain;
    int auto_wb;
} iac_cam_init_params_t;

int iac_cam_open(HANDLE *);
int iac_cam_close(const HANDLE *);
int iac_cam_init(const HANDLE *, const iac_cam_init_params_t *);
int iac_cam_acquire(const HANDLE *, XI_IMG *);

#endif
