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
#include <m3api/xiApi.h>
#include "iac.h"
#include "camera.h"

int iac_cam_open(HANDLE *handle)
{

    /* Open camera */
    if (xiOpenDevice(IAC_CAM_DEVICE, handle) != XI_OK) {
        fprintf(stderr, "Unable to open camera!\n");
        return IAC_FAILURE;
    }

    return IAC_SUCCESS;
}


int iac_cam_close(const HANDLE *handle)
{

    /* Close camera */
    if (xiCloseDevice(*handle) != XI_OK) {
        fprintf(stderr, "Unable to close camera!\n");
        return IAC_FAILURE;
    }

    return IAC_SUCCESS;
}


int iac_cam_init(const HANDLE *handle, const iac_cam_init_params_t *params)
{

    /* Use safe buffer */
    if (xiSetParamInt(*handle, XI_PRM_BUFFER_POLICY, XI_BP_SAFE) != XI_OK) {
        fprintf(stderr, "Unable to set safe buffer policy!\n");
        return IAC_FAILURE;
    }

    /* Set exposure */
    if (params->exposure) {
        if (xiSetParamInt(*handle, XI_PRM_EXPOSURE, params->exposure) != XI_OK) {
            fprintf(stderr, "Unable to set exposure!\n");
            return IAC_FAILURE;
        }
    }

    /* Set gain */
    if (params->gain) {
        if (xiSetParamFloat(*handle, XI_PRM_GAIN, (float) params->gain) != XI_OK) {
            fprintf(stderr, "Unable to set gain!\n");
            return IAC_FAILURE;
        }
    }

    /* Set white balance */
    if (params->auto_wb) {
        if (xiSetParamInt(*handle, XI_PRM_AUTO_WB, params->auto_wb) != XI_OK) {
            fprintf(stderr, "Unable to set auto white balance!\n");
            return IAC_FAILURE;
        }
    }

    /* Set image format */
    if (xiSetParamInt(*handle, XI_PRM_IMAGE_DATA_FORMAT, IAC_CAM_FORMAT) != XI_OK) {
        fprintf(stderr, "Unable to set image format!\n");
        return IAC_FAILURE;
    }

    return IAC_SUCCESS;
}


int iac_cam_acquire(const HANDLE *handle, XI_IMG *image)
{

    /* Start acquiring */
    if (xiStartAcquisition(*handle) != XI_OK) {
        fprintf(stderr, "Unable to start acquisition!\n");
        return IAC_FAILURE;
    }

    /* Get image */
    if (xiGetImage(*handle, IAC_CAM_ACQUIRE_TIMEOUT, image) != XI_OK) {
        fprintf(stderr, "Unable to get image!\n");
        return IAC_FAILURE;
    }

    /* Stop acquiring */
    if (xiStopAcquisition(*handle) != XI_OK) {
        fprintf(stderr, "Unable to stop acquisition!\n");
        return IAC_FAILURE;
    }

    return IAC_SUCCESS;
}
