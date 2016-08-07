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
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "utils.h"

uint8_t *iac_serialize(uint8_t *buf,
                       size_t *buf_size,
                       const uint8_t data)
{

    buf = realloc(buf, *buf_size + sizeof(data));
    *buf_size += sizeof(data);
    memcpy(buf + *buf_size, &data, sizeof(data));

    return buf;
}


uint8_t *iac_serialize_short(uint8_t *buf,
                             size_t *buf_size,
                             uint16_t data)
{

    buf = realloc(buf, *buf_size + sizeof(data));
    *buf_size += sizeof(data);
    data = htons(data);
    memcpy(buf + *buf_size, &data, sizeof(data));

    return buf;
}


uint8_t *iac_serialize_long(uint8_t *buf,
                            size_t *buf_size,
                            uint32_t data)
{

    buf = realloc(buf, *buf_size + sizeof(data));
    *buf_size += sizeof(data);
    data = htonl(data);
    memcpy(buf + *buf_size, &data, sizeof(data));

    return buf;
}


uint8_t *iac_serialize_pad(uint8_t *buf,
                           size_t *buf_size,
                           const uint8_t data,
                           const size_t size)
{

    buf = realloc(buf, *buf_size + size);
    memset(buf + *buf_size, data, size);
    *buf_size += size;

    return buf;
}


uint8_t *iac_serialize_data(uint8_t *buf,
                            size_t *buf_size,
                            const uint8_t *data,
                            const size_t size)
{

    buf = realloc(buf, *buf_size + size);
    memcpy(buf + *buf_size, data, size);
    *buf_size += size;

    return buf;
}


uint8_t iac_lrc(const uint8_t *buf, const size_t size)
{
    uint8_t lrc = 0;
    size_t i;

    for (i = 0; i < size; i++) {
        lrc ^= buf[i];
    }

    return lrc;
}
