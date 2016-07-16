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

#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include "iac.h"
#include "utils.h"
#include "obc.h"

iac_obc_packet_t iac_obc_packet(const iac_obc_block_t *block)
{
    iac_obc_packet_t packet;

    packet.buf = NULL;
    packet.size = 0;
    packet.buf = iac_serialize(packet.buf, &packet.size, block->tile);
    packet.buf = iac_serialize_short(packet.buf, &packet.size, block->index);
    packet.buf = iac_serialize_data(packet.buf,
                                    &packet.size,
                                    block->data,
                                    block->data_size);
    if (block->data_size < IAC_OBC_BLOCK_SIZE)
        packet.buf = iac_serialize_pad(packet.buf,
                                       &packet.size,
                                       IAC_OBC_BLOCK_PADDING,
                                       IAC_OBC_BLOCK_SIZE - block->data_size);
    packet.buf = iac_serialize(packet.buf,
                               &packet.size,
                               iac_lrc(packet.buf, packet.size));

    return packet;
}
