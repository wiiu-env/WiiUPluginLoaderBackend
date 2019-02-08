/****************************************************************************
 * Copyright (C) 2018 Maschell
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
 ****************************************************************************/
#ifndef __MEMORY_UTILS_UTILS_H_
#define __SCREEN_UTILS_H_

class MemoryUtils {
public:
    static void init();

    static void* alloc(uint32_t size, int32_t align);

    static void free(void * ptr);
private:
    MemoryUtils() {}
    ~MemoryUtils() {}

};
#endif
