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

typedef struct iac_spi_init_params_t {
    uint8_t mode;
    uint8_t bits_per_word;
    uint32_t max_speed_hz;
} iac_spi_init_params_t;


int iac_spi_init(const char *, const iac_spi_init_params_t *);
int iac_spi_transfer(const int fd, uint8_t *, const uint32_t);
