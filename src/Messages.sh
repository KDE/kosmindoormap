#! /usr/bin/env bash
# SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

$XGETTEXT `find . -name \*.cpp -o -name \*.h -name \*.qml` -o $podir/kosmindoormap.pot
