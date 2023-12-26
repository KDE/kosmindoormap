/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "languages.h"

#include <QLocale>

static bool cmpLang(const std::string &lhs, QStringView rhs)
{
    return QAnyStringView::compare(QAnyStringView(lhs.c_str(), lhs.size()), rhs) == 0;
}

static void addLanguage(OSM::Languages &langs, QStringView langWithScript, QStringView lang)
{
    if (!langWithScript.isEmpty()) {
        for (auto &it : langs.languages) {
            if (cmpLang(it, langWithScript)) {
                langWithScript = {};
                break;
            }
            if (cmpLang(it, lang)) {
                it = langWithScript.toUtf8().constData();
                langWithScript = {};
                break;
            }
        }
    }
    if (!langWithScript.isEmpty()) {
        langs.languages.emplace_back(langWithScript.toUtf8().constData());
    }

    const auto it = std::find_if(langs.languages.begin(), langs.languages.end(), [lang](const auto &l) { return cmpLang(l, lang); });
    if (it == langs.languages.end()) {
        langs.languages.emplace_back(lang.toUtf8().constData());
    }
}

OSM::Languages OSM::Languages::fromQLocale(const QLocale &locale)
{
    Languages langs;

    const auto uiLangs = locale.uiLanguages();
    langs.languages.reserve(uiLangs.size());
    for (const auto &uiLang : uiLangs) {
        QStringView s(uiLang);
        const auto dashCount = s.count(QLatin1Char('-'));
        if (dashCount == 0) {
            addLanguage(langs, {}, s);
            continue;
        }
        if (dashCount > 1) {
            const auto idx = s.lastIndexOf(QLatin1Char('-'));
            s = s.left(idx);
        }

        const auto idx = s.indexOf(QLatin1Char('-'));
        if (s.mid(idx + 1).size() == 2) {
            // second part is a country code, that does not occur in OSM language keys
            addLanguage(langs, {}, s.left(idx));
        } else {
            // second part is a script
            addLanguage(langs, s, s.left(idx));
        }
    }

    return langs;
}
