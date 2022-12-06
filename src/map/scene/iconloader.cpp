/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "iconloader_p.h"

#include <QBuffer>
#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QIconEngine>
#include <QImageReader>
#include <QPainter>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

using namespace KOSMIndoorMap;

/** Device pixel ratio preserving simple icon engine for our SVG assets. */
class IconEngine : public QIconEngine
{
public:
    explicit IconEngine(QIODevice *svgFile, const IconData &iconData)
        : m_iconData(iconData)
        , m_image(renderStyledSvg(svgFile, iconData.size))
    {
    }

    ~IconEngine() = default;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QList<QSize> availableSizes(QIcon::Mode mode, QIcon::State state) const override
#else
    QList<QSize> availableSizes(QIcon::Mode mode, QIcon::State state) override
#endif
    {
        Q_UNUSED(mode);
        Q_UNUSED(state);
        return { m_image.size() / m_image.devicePixelRatio() };
    }

    QIconEngine* clone() const override
    {
        auto engine = new IconEngine;
        engine->m_image = m_image;
        return engine;
    }

    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;

    static QString findSvgAsset(const QString &name);

private:
    explicit IconEngine() = default;
    QImage renderStyledSvg(QIODevice *svgFile, const QSizeF &size);

    IconData m_iconData;
    QImage m_image;
};

static bool operator<(const IconData &lhs, const IconData &rhs)
{
    if (lhs.name == rhs.name) {
        if (lhs.color.rgb() == rhs.color.rgb()) {
            return lhs.size.width() < rhs.size.width();
        }
        return lhs.color.rgb() < rhs.color.rgb();
    }
    return lhs.name < rhs.name;
}

static bool operator==(const IconData &lhs, const IconData &rhs)
{
    return lhs.name == rhs.name && lhs.color == rhs.color && lhs.size == rhs.size;
}

QIcon IconLoader::loadIcon(const IconData &iconData) const
{
    // check our cache
    auto it = std::lower_bound(m_cache.begin(), m_cache.end(), iconData, [](const auto &lhs, const auto &rhs) { return lhs.data < rhs; });
    if (it != m_cache.end() && (*it).data == iconData) {
        return (*it).icon;
    }

    // check if it's one of our bundled assets
    const QString path = IconEngine::findSvgAsset(iconData.name);
    QFile f(path);
    if (f.open(QFile::ReadOnly)) {
        CacheEntry entry;
        entry.data = iconData;
        entry.icon = QIcon(new IconEngine(&f, iconData));
        it = m_cache.insert(it, std::move(entry));
        return (*it).icon;
    }

    // TODO file system URLs

    // XDG icons
    const auto icon = QIcon::fromTheme(iconData.name);
    if (icon.isNull()) {
        qWarning() << "Failed to find icon:" << iconData.name;
    }
    return icon;
}

QString IconEngine::findSvgAsset(const QString &name)
{
    return QLatin1String(":/org.kde.kosmindoormap/assets/icons/") + name + QLatin1String(".svg");
}

void IconEngine::paint(QPainter *painter, const QRect &rect, [[maybe_unused]] QIcon::Mode mode, [[maybe_unused]] QIcon::State state)
{
    // check if our pre-rendered image cache has a resolution high enough for this
    const auto threshold = std::max<int>(1, std::max(m_image.width(), m_image.height()) * 0.25);
    if (rect.width() > m_image.width() + threshold || rect.height() > m_image.height() + threshold) {
        QFile f(findSvgAsset(m_iconData.name));
        if (f.open(QFile::ReadOnly)) {
            m_image = renderStyledSvg(&f, rect.size());
        }
    }

    painter->drawImage(rect, m_image);
}

QImage IconEngine::renderStyledSvg(QIODevice *svgFile, const QSizeF &size)
{
    // prepare CSS
    const QString css = QLatin1String(".ColorScheme-Text { color:") + m_iconData.color.name(QColor::HexRgb) + QLatin1String("; }");

    // inject CSS (inspired by KIconLoader)
    QByteArray processedContents;
    QXmlStreamReader reader(svgFile);
    QBuffer buffer(&processedContents);
    buffer.open(QIODevice::WriteOnly);
    QXmlStreamWriter writer(&buffer);
    while (!reader.atEnd()) {
        if (reader.readNext() == QXmlStreamReader::StartElement &&
            reader.qualifiedName() == QLatin1String("style") &&
            reader.attributes().value(QLatin1String("id")) == QLatin1String("current-color-scheme")) {
            writer.writeStartElement(QStringLiteral("style"));
            writer.writeAttributes(reader.attributes());
            writer.writeCharacters(css);
            writer.writeEndElement();
            while (reader.tokenType() != QXmlStreamReader::EndElement) {
                reader.readNext();
            }
        } else if (reader.tokenType() != QXmlStreamReader::Invalid) {
            writer.writeCurrentToken(reader);
        }
    }
    buffer.close();

    // render SVG
    buffer.open(QIODevice::ReadOnly);
    buffer.seek(0);
    QImageReader imgReader(&buffer, "svg");
    imgReader.setScaledSize((size.isValid() ? size.toSize() : imgReader.size()) * qGuiApp->devicePixelRatio());
    auto img = imgReader.read();
    img.setDevicePixelRatio(qGuiApp->devicePixelRatio());
    return img;
}
