/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSMINDOORMAP_OSMELEMENTINFORMATIONMODEL_H
#define KOSMINDOORMAP_OSMELEMENTINFORMATIONMODEL_H

#include "osmelement.h"

#include <osm/element.h>

#include <wikidata/entities.h>
#include <wikidata/wikidataquerymanager.h>

#include <QAbstractListModel>

namespace KOSMIndoorMap {

/** Model containing information about a selected element.
 *  Exact content depends on the type of element and the available information.
 */
class OSMElementInformationModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(KOSMIndoorMap::OSMElement element READ element WRITE setElement NOTIFY elementChanged)
    Q_PROPERTY(QString name READ name NOTIFY elementChanged)
    Q_PROPERTY(QString category READ category NOTIFY elementChanged)
    Q_PROPERTY(bool allowOnlineContent MEMBER m_allowOnlineContent NOTIFY allowOnlineContentChanged)
    Q_PROPERTY(bool debug MEMBER m_debug)

public:
    explicit OSMElementInformationModel(QObject *parent = nullptr);
    ~OSMElementInformationModel();

    enum Role {
        KeyRole = Qt::UserRole,
        KeyLabelRole,
        ValueRole,
        ValueUrlRole,
        CategoryRole,
        CategoryLabelRole,
        TypeRole,
    };
    enum KeyCategory {
        UnresolvedCategory,
        Header,
        Main,
        OpeningHoursCategory,
        Contact,
        Payment,
        Toilets,
        Accessibility,
        Parking,
        Operator,
        DebugCategory,
    };
    Q_ENUM(KeyCategory)
    enum Key {
        NoKey,
        Name,
        Category,
        Image,
        Logo,
        OldName,
        Description,
        Routes,
        Cuisine,
        Diet,
        Takeaway,
        Socket,
        OpeningHours,
        AvailableVehicles,
        Fee,
        Authentication,
        BicycleParking,
        Capacity,
        CapacityDisabled,
        CapacityWomen,
        CapacityParent,
        CapacityCharing,
        MaxStay,
        DiaperChangingTable,
        Gender,
        Wikipedia,
        Address,
        Phone,
        Email,
        Website,
        PaymentCash,
        PaymentDigital,
        PaymentDebitCard,
        PaymentCreditCard,
        PaymentStoredValueCard,
        Wheelchair,
        WheelchairLift,
        CentralKey,
        SpeechOutput,
        TactileWriting,
        OperatorName,
        Network,
        OperatorWikipedia,
        RemainingRange,
        DebugLink,
        DebugKey,
    };
    Q_ENUM(Key)
    enum Type {
        String,
        Link,
        PostalAddress,
        OpeningHoursType,
        ImageType,
    };
    Q_ENUM(Type)

    [[nodiscard]] OSMElement element() const;
    void setElement(const OSMElement &element);
    Q_INVOKABLE void clear();

    [[nodiscard]] QString name() const;
    [[nodiscard]] QString category() const;

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void elementChanged();
    void allowOnlineContentChanged();

private:
    struct Info;

    void reload();
    /** Process online content, filter out things we don't want, etc. */
    void resolveOnlineContent();
    /** Resolve elements who's group depends on context. */
    void resolveCategories();
    /** Make sure we have at least one naming element. */
    void resolveHeaders();
    [[nodiscard]] bool promoteMainCategory(KeyCategory cat);
    [[nodiscard]] QString categoryLabel(KeyCategory cat) const;
    [[nodiscard]] QString debugTagKey(int row) const;
    [[nodiscard]] QString debugTagValue(int row) const;
    [[nodiscard]] QUrl debugTagUrl(int row) const;
    [[nodiscard]] QString keyName(Key key) const;
    [[nodiscard]] QVariant valueForKey(Info info) const;
    [[nodiscard]] QVariant urlify(const QVariant &v, Key key) const;
    [[nodiscard]] QString paymentMethodList(Key key) const;
    [[nodiscard]] QString paymentMethodValue(Key key) const;
    [[nodiscard]] QUrl wikipediaUrl(const QByteArray &wp) const;
    [[nodiscard]] QString capacitryValue(const char *prop) const;
    [[nodiscard]] QString translatedBoolValue(const QByteArray &value) const;

    template <typename KeyMapEntry, std::size_t N>
    void addEntryForKey(const char *keyName, const KeyMapEntry(&map)[N]);
    template <typename KeyMapEntry, std::size_t N>
    void addEntryForLocalizedKey(const char *keyName, const KeyMapEntry(&map)[N]);

    OSM::Element m_element;

    struct Info {
        Key key;
        KeyCategory category;

        [[nodiscard]] bool operator<(Info other) const;
        [[nodiscard]] bool operator==(Info other) const;
    };
    std::vector<Info> m_infos;
    OSM::Languages m_langs;
    Key m_nameKey = NoKey;
    Key m_categoryKey = NoKey;
    bool m_allowOnlineContent = false;
    bool m_debug = false;

    Wikidata::QueryManager m_wikidataMgr;
    QHash<Wikidata::Q, QString> m_wikidataImageMap;
};

}

#endif // KOSMINDOORMAP_OSMELEMENTINFORMATIONMODEL_H
