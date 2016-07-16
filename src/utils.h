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

#ifndef __UTILS_H
#define __UTILS_H

uint8_t *iac_serialize(uint8_t *,
                       size_t *,
                       const uint8_t);
uint8_t *iac_serialize_short(uint8_t *,
                             size_t *,
                             const uint16_t);
uint8_t *iac_serialize_long(uint8_t *,
                            size_t *,
                            const uint32_t);
uint8_t *iac_serialize_pad(uint8_t *,
                           size_t *,
                           const uint8_t,
                           const size_t);
uint8_t *iac_serialize_data(uint8_t *,
                            size_t *,
                            const uint8_t *,
                            const size_t);
uint8_t iac_lrc(const uint8_t *, const size_t);

#endif
