/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef OSM_O5M_H
#define OSM_O5M_H

#include <cstdint>

/** @file o5m.h
 *  Common declarations for O5M file format I/O.
 */

namespace OSM {

enum : uint8_t {
    O5M_BLOCK_RESET = 0xff,
    O5M_BLOCK_NODE = 0x10,
    O5M_BLOCK_WAY = 0x11,
    O5M_BLOCK_RELATION = 0x12,
    O5M_BLOCK_BOUNDING_BOX = 0xdb,
    O5M_BLOCK_TIMESTAMP = 0xdc,
    O5M_BLOCK_HEADER = 0xe0,

    O5M_NUMBER_CONTINUATION = 0b1000'0000,
    O5M_NUMBER_MASK = 0b0111'1111,
    O5M_NUMBER_SIGNED_BIT = 0b1,

    O5M_MEMTYPE_NODE = 0x30,
    O5M_MEMTYPE_WAY = 0x31,
    O5M_MEMTYPE_RELATION = 0x32,

    O5M_TRAILER = 0xfe,
};

enum : uint16_t {
    O5M_STRING_TABLE_SIZE = 15000,
    O5M_STRING_TABLE_MAXLEN = 250,
};

constexpr inline const char O5M_HEADER[] = "o5m2";

}

#endif
