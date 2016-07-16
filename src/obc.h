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

#ifndef __OBC_H
#define __OBC_H

typedef struct iac_obc_block_t {
    uint8_t tile;
    uint16_t index;
    uint8_t *data;
    size_t data_size;
} iac_obc_block_t;

typedef struct iac_obc_packet_t {
    uint8_t *buf;
    size_t size;
} iac_obc_packet_t;

iac_obc_packet_t iac_obc_packet(const iac_obc_block_t *);

#endif
