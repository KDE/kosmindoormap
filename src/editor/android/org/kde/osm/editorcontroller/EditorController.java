/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

package org.kde.osm.editorcontroller;

import android.content.Context;
import android.content.pm.PackageManager;

public class EditorController
{
    public static boolean hasVespucci(android.content.Context context)
    {
        try {
            context.getPackageManager().getPackageInfo("de.blau.android", 0);
        } catch (PackageManager.NameNotFoundException e) {
            return false;
        }
        return true;
    }
}
