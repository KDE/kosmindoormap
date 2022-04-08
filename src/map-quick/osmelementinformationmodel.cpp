/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "osmelementinformationmodel.h"
#include "osmelementinformationmodel_p.h"
#include "osmelementinformationmodel_data.cpp"
#include "osmaddress.h"

#include <KLocalizedString>

#include <cctype>

using namespace KOSMIndoorMap;

static QString formatDistance(int meter)
{
    if (meter < 1000) {
        return i18n("%1m", meter);
    }
    if (meter < 10000) {
        return i18n("%1km", ((int)meter/100)/10.0);
    }
    return i18n("%1km", (int)qRound(meter/1000.0));
}

bool OSMElementInformationModel::Info::operator<(OSMElementInformationModel::Info other) const
{
    if (category == other.category) {
        return key < other.key;
    }
    return category < other.category;
}

bool OSMElementInformationModel::Info::operator==(OSMElementInformationModel::Info other) const
{
    return category == other.category && key == other.key;
}


OSMElementInformationModel::OSMElementInformationModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

OSMElementInformationModel::~OSMElementInformationModel() = default;

OSMElement OSMElementInformationModel::element() const
{
    return OSMElement(m_element);
}

void OSMElementInformationModel::setElement(const OSMElement &element)
{
    if (m_element == element.element()) {
        return;
    }

    beginResetModel();
    m_element = element.element();
    m_infos.clear();
    if (m_element.type() != OSM::Type::Null) {
        reload();
    }
    endResetModel();
    Q_EMIT elementChanged();
}

void OSMElementInformationModel::clear()
{
    if (m_element.type() == OSM::Type::Null) {
        return;
    }
    beginResetModel();
    m_infos.clear();
    m_element = {};
    endResetModel();
    Q_EMIT elementChanged();
}

QString OSMElementInformationModel::name() const
{
    return valueForKey(Info{m_nameKey, Header}).toString();
}

QString OSMElementInformationModel::category() const
{
    return valueForKey(Info{m_categoryKey, Header}).toString();
}

int OSMElementInformationModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || m_element.type() == OSM::Type::Null) {
        return 0;
    }
    return m_infos.size();
}

QVariant OSMElementInformationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const auto info = m_infos[index.row()];
    switch (role) {
        case TypeRole:
            switch (info.key) {
                case Wikipedia:
                case Phone:
                case Email:
                case Website:
                case OperatorWikipedia:
                case DebugLink:
                    return Link;
                case Address:
                    return PostalAddress;
                case OpeningHours:
                    return OpeningHoursType;
                default:
                    return String;
            }
        case KeyRole:
            return info.key;
        case KeyLabelRole:
            if (info.key == DebugKey) {
                return debugTagKey(index.row());
            }
            return keyName(info.key);
        case ValueRole:
            switch (info.key) {
                case DebugKey: return debugTagValue(index.row());
                case Wikipedia: return i18n("Wikipedia");
                default: return valueForKey(info);
            }
        case ValueUrlRole:
            return urlify(valueForKey(info), info.key);
        case CategoryRole:
            return info.category;
        case CategoryLabelRole:
            return categoryLabel(info.category);
    }

    return {};
}

QHash<int, QByteArray> OSMElementInformationModel::roleNames() const
{
    auto r = QAbstractListModel::roleNames();
    r.insert(KeyRole, "key");
    r.insert(KeyLabelRole, "keyLabel");
    r.insert(ValueRole, "value");
    r.insert(ValueUrlRole, "url");
    r.insert(CategoryRole, "category");
    r.insert(CategoryLabelRole, "categoryLabel");
    r.insert(TypeRole, "type");
    return r;
}

#define M(name, key, category) { name, OSMElementInformationModel::key, OSMElementInformationModel::category }
struct {
    const char *keyName;
    OSMElementInformationModel::Key m_key;
    OSMElementInformationModel::KeyCategory m_category;

    constexpr inline OSMElementInformationModel::Key key() const { return m_key; }
    constexpr inline OSMElementInformationModel::KeyCategory category() const { return m_category; }
} static constexpr const simple_key_map[] = {
    M("addr:city", Address, Contact),
    M("addr:street", Address, Contact),
    M("amenity", Category, Header),
    M("bicycle_parking", BicycleParking, Parking),
    M("brand", Name, Header),
    M("brand:wikipedia", Wikipedia, UnresolvedCategory),
    M("bus_routes", Routes, Main),
    M("buses", Routes, Main),
    M("capacity", Capacity, UnresolvedCategory),
    M("capacity:charging", CapacityCharing, Parking),
    M("capacity:disabled", CapacityDisabled, Parking),
    M("capacity:parent", CapacityParent, Parking),
    M("capacity:women", CapacityWomen, Parking),
    M("centralkey", CentralKey, Accessibility),
    M("changing_table", DiaperChangingTable, UnresolvedCategory),
    M("charge", Fee, UnresolvedCategory),
    M("contact:city", Address, Contact),
    M("contact:email", Email, Contact),
    M("contact:phone", Phone, Contact),
    M("contact:street", Address, Contact),
    M("contact:website", Website, Contact),
    M("cuisine", Cuisine, Main),
    M("diaper", DiaperChangingTable, UnresolvedCategory),
    M("email", Email, Contact),
    M("fee", Fee, UnresolvedCategory),
    M("leisure", Category, Header),
    M("maxstay", MaxStay, Parking),
    M("mx:realtime_available", AvailableVehicles, Main),
    M("mx:remaining_range", RemainingRange, Main),
    M("mx:vehicle", Category, Header),
    M("network", Network, Operator),
    M("office", Category, Header),
    M("old_name", OldName, UnresolvedCategory),
    M("opening_hours", OpeningHours, OpeningHoursCategory),
    M("operator", OperatorName, Operator),
    M("operator:email", Email, Contact),
    M("operator:phone", Phone, Contact),
    M("operator:website", Website, Contact),
    M("operator:wikipedia", OperatorWikipedia, Operator),
    M("parking:fee", Fee, Parking),
    M("payment:cash", PaymentCash, Payment),
    M("payment:coins", PaymentCash, Payment),
    M("payment:notes", PaymentCash, Payment),
    M("phone", Phone, Contact),
    M("room", Category, Header),
    M("route_ref", Routes, Main),
    M("shop", Category, Header),
    M("takeaway", Takeaway, Main),
    M("toilets:fee", Fee, Toilets),
    M("toilets:wheelchair", Wheelchair, Toilets),
    M("tourism", Category, Header),
    M("url", Website, Contact),
    M("website", Website, Contact),
    M("wheelchair", Wheelchair, Accessibility),
};
#undef M
static_assert(isSortedLookupTable(simple_key_map), "key map is not sorted!");

template <typename KeyMapEntry, std::size_t N>
void OSMElementInformationModel::addEntryForKey(const char *keyName, const KeyMapEntry(&map)[N])
{
    const auto it = std::lower_bound(std::begin(map), std::end(map), keyName, [](const auto &lhs, auto rhs) {
        return std::strcmp(lhs.keyName, rhs) < 0;
    });
    if (it != std::end(map) && std::strcmp((*it).keyName, keyName) == 0) {
        m_infos.push_back(Info{(*it).key(), (*it).category()});
    }
}

void OSMElementInformationModel::reload()
{
    m_nameKey = NoKey;
    m_categoryKey = NoKey;

    for (auto it = m_element.tagsBegin(); it != m_element.tagsEnd(); ++it) {
        if (std::strncmp((*it).key.name(), "name", 4) == 0) {
            m_infos.push_back(Info{Name, Header});
            continue;
        }
        if (std::strncmp((*it).key.name(), "wikipedia", 9) == 0) {
            m_infos.push_back(Info{Wikipedia, UnresolvedCategory});
            continue;
        }
        addEntryForKey((*it).key.name(), simple_key_map);
        addEntryForKey((*it).key.name(), payment_generic_type_map);
        addEntryForKey((*it).key.name(), payment_type_map);
        addEntryForKey((*it).key.name(), diet_type_map);
        addEntryForKey((*it).key.name(), socket_type_map);
        addEntryForKey((*it).key.name(), authentication_type_map);
    }

    std::sort(m_infos.begin(), m_infos.end());
    m_infos.erase(std::unique(m_infos.begin(), m_infos.end()), m_infos.end());
    resolveCategories();
    resolveHeaders();

    // if we don't have a primary group, promote a suitable secondary one
    for (auto cat : {Parking, Toilets}) {
        if (promoteMainCategory(cat)) {
            break;
        }
    }

    // resolve all remaining unresolved elements to the primary category
    for (auto &info : m_infos) {
        if (info.category == UnresolvedCategory) {
            info.category = Main;
        }
    }
    std::sort(m_infos.begin(), m_infos.end());
    m_infos.erase(std::unique(m_infos.begin(), m_infos.end()), m_infos.end());

    if (m_debug) {
        m_infos.push_back(Info{ DebugLink, DebugCategory });
        const auto count = std::distance(m_element.tagsBegin(), m_element.tagsEnd());
        std::fill_n(std::back_inserter(m_infos), count, Info{ DebugKey, DebugCategory });
    }
}

void OSMElementInformationModel::resolveCategories()
{
    if (m_infos.empty() || m_infos[0].category != UnresolvedCategory) {
        return;
    }
    for (auto &info : m_infos) {
        if (info.category != UnresolvedCategory) {
            break;
        }
        switch (info.key) {
            case Fee:
                if (m_element.tagValue("parking:fee").isEmpty() && (!m_element.tagValue("parking").isEmpty()
                    || m_element.tagValue("amenity") == "parking" || m_element.tagValue("amenity") == "bicycle_parking"))
                {
                    info.category = Parking;
                } else if (m_element.tagValue("toilets:fee").isEmpty() && (m_element.tagValue("toilets") == "yes" || m_element.tagValue("amenity") == "toilets")) {
                    info.category = Toilets;
                } else {
                    info.category = Main;
                }
                break;
            case Capacity:
                if (m_element.tagValue("amenity").endsWith("rental")) {
                    info.category = Main;
                } else {
                    info.category = Parking;
                }
                break;
            default:
            {
                // for anything else: if it's not clearly something we have a secondary group for, resolve it to Main
                const auto amenity = m_element.tagValue("amenity");
                if ((amenity != "parking" && amenity != "toilets")
                    || !m_element.tagValue("office").isEmpty()
                    || (!m_element.tagValue("room").isEmpty() && m_element.tagValue("room") != "toilets")
                    || !m_element.tagValue("shop").isEmpty()
                    || !m_element.tagValue("tourism").isEmpty()) {
                    info.category = Main;
                }
                break;
            }
        }
    }
    std::sort(m_infos.begin(), m_infos.end());
}

void OSMElementInformationModel::resolveHeaders()
{
    for (auto key : { Name, Network, OperatorName, Category }) {
        if (m_nameKey != NoKey) {
            break;
        }

        const auto it = std::find_if(m_infos.begin(), m_infos.end(), [key](Info info) {
            return info.key == key;
        });
        if (it == m_infos.end()) {
            continue;
        }

        m_nameKey = (*it).key;
        m_infos.erase(it);
        break;
    }

    // we use the categories as header if there is no name, so don't duplicate that
    const auto it = std::find_if(m_infos.begin(), m_infos.end(), [](Info info) {
        return info.key == Category;
    });
    if (it == m_infos.end() || m_nameKey == Category) {
        return;
    }

    m_infos.erase(it);
    m_categoryKey = Category;
}

bool OSMElementInformationModel::promoteMainCategory(OSMElementInformationModel::KeyCategory cat)
{
    const auto hasMain = std::any_of(m_infos.begin(), m_infos.end(), [](const auto &info) {
        return info.category == Main;
    });

    if (hasMain) {
        return true;
    }

    bool didPromote = false;
    for (auto &info : m_infos) {
        if (info.category == cat) {
            info.category = (info.key == Wheelchair ? Accessibility : Main);
            didPromote = true;
        }
    }

    if (didPromote) {
        std::sort(m_infos.begin(), m_infos.end());
    }
    return didPromote;
}

QString OSMElementInformationModel::categoryLabel(OSMElementInformationModel::KeyCategory cat) const
{
    switch (cat) {
        case UnresolvedCategory:
        case Header:
        case Main:          return {};
        case OpeningHoursCategory: return i18n("Opening Hours");
        case Contact:       return i18n("Contact");
        case Payment:       return i18n("Payment");
        case Toilets:       return i18n("Toilets");
        case Accessibility: return i18n("Accessibility");
        case Parking:       return i18n("Parking");
        case Operator:      return i18n("Operator");
        case DebugCategory: return QStringLiteral("Debug");
    }
    return {};
}

QString OSMElementInformationModel::debugTagKey(int row) const
{
    const auto tagCount = std::distance(m_element.tagsBegin(), m_element.tagsEnd());
    const auto tagIdx = row - (rowCount() - tagCount);
    return QString::fromUtf8((*(m_element.tagsBegin() + tagIdx)).key.name());
}

QString OSMElementInformationModel::debugTagValue(int row) const
{
    const auto tagCount = std::distance(m_element.tagsBegin(), m_element.tagsEnd());
    const auto tagIdx = row - (rowCount() - tagCount);
    return QString::fromUtf8((*(m_element.tagsBegin() + tagIdx)).value);
}

QString OSMElementInformationModel::keyName(OSMElementInformationModel::Key key) const
{
    switch (key) {
        case NoKey:
        case Name:
        case Category: return {};
        case OldName: return i18n("Formerly");
        case Routes: return i18n("Routes");
        case Cuisine: return i18n("Cuisine");
        case Diet: return i18n("Diet");
        case Takeaway: return i18n("Takeaway");
        case Socket: return i18nc("electrical power socket", "Socket");
        case OpeningHours: return {};
        case AvailableVehicles: return i18n("Available vehicles");
        case Fee: return i18n("Fee");
        case Authentication: return i18n("Authentication");
        case BicycleParking: return i18n("Bicycle parking");
        case Capacity: return i18n("Capacity");
        case CapacityDisabled: return i18n("Disabled parking spaces");
        case CapacityWomen: return i18n("Women parking spaces");
        case CapacityParent: return i18n("Parent parking spaces");
        case CapacityCharing: return i18n("Parking spaces for charging");
        case MaxStay: return i18n("Maximum stay");
        case DiaperChangingTable: return i18n("Diaper changing table");
        case Wikipedia: return {};
        case Address: return i18n("Address");
        case Phone: return i18n("Phone");
        case Email: return i18n("Email");
        case Website: return i18n("Website");
        case PaymentCash: return i18n("Cash");
        case PaymentDigital: return i18n("Digital");
        case PaymentDebitCard: return i18n("Debit cards");
        case PaymentCreditCard: return i18n("Credit cards");
        case PaymentStoredValueCard: return i18n("Stored value cards");
        case Wheelchair: return i18n("Wheelchair access");
        case CentralKey: return i18n("Central key");
        case OperatorName: return {};
        case Network: return i18nc("transport network", "Network");
        case OperatorWikipedia: return {};
        case RemainingRange: return i18nc("remaining travel range of a battery powered vehicle", "Remaining range");
        case DebugLink: return QStringLiteral("OSM");
        case DebugKey: return {};
    }
    return {};
}

QVariant OSMElementInformationModel::valueForKey(Info info) const
{
    switch (info.key) {
        case NoKey: return {};
        case Name: return QString::fromUtf8(m_element.tagValue("name", "brand", QLocale()));
        case Category:
        {
            QList<QByteArray> l;
            l += m_element.tagValue("amenity").split(';');
            l += m_element.tagValue("shop").split(';');
            l += m_element.tagValue("tourism").split(';');
            l += m_element.tagValue("vending").split(';');
            l += m_element.tagValue("office").split(';');
            l += m_element.tagValue("leisure").split(';');
            l += m_element.tagValue("mx:vehicle");
            if (l.isEmpty()) {
                l += m_element.tagValue("room").split(';');
            }
            QStringList out;
            out.reserve(l.size());

            // TODO drop general categories if specific ones are available (e.g. restaurant vs fast_food)

            for (auto it = l.begin(); it != l.end();++it) {
                (*it) = (*it).trimmed();
                if ((*it).isEmpty() || (*it) == "yes" || (*it) == "vending_machine") {
                    continue;
                }
                out.push_back(translateValue((*it).constData(), amenity_map));
            }

            std::sort(out.begin(), out.end());
            out.erase(std::unique(out.begin(), out.end()), out.end());
            return QLocale().createSeparatedList(out);
        }
        case OldName:
        {
            const auto l = QString::fromUtf8(m_element.tagValue("old_name")).split(QLatin1Char(';'));
            return l.join(QLatin1String(", "));
        }
        case Routes:
        {
            auto l = QString::fromUtf8(m_element.tagValue("route_ref", "bus_routes", "buses")).split(QLatin1Char(';'), Qt::SkipEmptyParts);
            for (auto &s : l) {
                s = s.trimmed();
            }
            return QLocale().createSeparatedList(l);
        }
        case Cuisine: return translateValues(m_element.tagValue("cuisine"), cuisine_map);
        case Diet:
        {
            QStringList l;
            for (const auto &d : diet_type_map) {
                const auto v = m_element.tagValue(d.keyName);
                const auto label = d.label.toString();
                if (v == "yes") {
                    l.push_back(label);
                } else if (v == "only") {
                    l.push_back(i18n("only %1", label));
                } else if (v == "no") {
                    l.push_back(i18n("no %1", label));
                }
            }
            return l.join(QLatin1String(", "));
        }
        case Takeaway: return translatedBoolValue(m_element.tagValue("takeaway")); // TODO decode (yes/only/no) and translate
        case Socket:
        {
            QStringList l;
            for (const auto &socket : socket_type_map) {
                const auto value = m_element.tagValue(socket.keyName);
                if (value.isEmpty() || value == "no") {
                    continue;
                }

                auto s = socket.label.toString();

                QStringList details;
                if (value != "yes") {
                    details.push_back(QString::fromUtf8(value));
                }

                const auto current = m_element.tagValue(QByteArray(socket.keyName + QByteArray(":current")).constData());
                if (!current.isEmpty()) {
                    if (std::all_of(current.begin(), current.end(), [](char c) { return std::isdigit(c); })) {
                        details.push_back(i18nc("electrical current/Ampere value", "%1 A", QString::fromUtf8(current)));
                    } else {
                        details.push_back(QString::fromUtf8(current));
                    }
                }
                const auto output = m_element.tagValue(QByteArray(socket.keyName + QByteArray(":output")).constData());
                if (!output.isEmpty()) {
                    if (std::all_of(output.begin(), output.end(), [](char c) { return std::isdigit(c); })) {
                        details.push_back(i18nc("electrical power/kilowatt value", "%1 kW", QString::fromUtf8(output)));
                    } else {
                        details.push_back(QString::fromUtf8(output));
                    }
                }

                if (!details.empty()) {
                    s += QLatin1String(" (") + details.join(QLatin1String(", ")) + QLatin1Char(')');
                }
                l.push_back(s);
            }
            return QLocale().createSeparatedList(l);
        }
        case OpeningHours: return QString::fromUtf8(m_element.tagValue("opening_hours"));
        case AvailableVehicles:
        {
            const auto total = m_element.tagValue("mx:realtime_available").toInt();
            QStringList types;
            for (const auto &v : available_vehicles_map) {
                const auto b = m_element.tagValue(v.keyName);
                if (b.isEmpty()) {
                    continue;
                }
                types.push_back(v.label.subs(b.toInt()).toString());
            }

            if (types.isEmpty()) {
                return QLocale().toString(total);
            } else if (types.size() == 1) {
                return types.at(0);
            } else {
                return i18n("%1 (%2)", total, QLocale().createSeparatedList(types));
            }
        }
        case Fee:
        {
            QByteArray fee;
            switch (info.category) {
                case Parking: fee = m_element.tagValue("parking:fee", "fee"); break;
                case Toilets: fee = m_element.tagValue("toilets:fee", "fee"); break;
                default: fee = m_element.tagValue("fee");
            }
            auto s = QString::fromUtf8(fee);
            const auto charge = QString::fromUtf8(m_element.tagValue("charge"));
            if (s.isEmpty()) {
                return charge;
            }
            if (!charge.isEmpty()) {
                s += QLatin1String(" (") + charge + QLatin1Char(')');
            }
            return s;
        }
        case Authentication:
        {
            QStringList l;
            for (const auto &auth : authentication_type_map) {
                const auto v = m_element.tagValue(auth.keyName);
                if (v.isEmpty() || v == "no") {
                    continue;
                }
                l.push_back(auth.label.toString());
            }
            return QLocale().createSeparatedList(l);
        }
        case BicycleParking: return translateValues(m_element.tagValue("bicycle_parking"), bicycle_parking_map);
        case Capacity: return QString::fromUtf8(m_element.tagValue("capacity"));
        case CapacityDisabled: return capacitryValue("capacity:disabled");
        case CapacityWomen: return capacitryValue("capacity:women");
        case CapacityParent: return capacitryValue("capacity:parent");
        case CapacityCharing: return capacitryValue("capacity:charging");
        case MaxStay: return QString::fromUtf8(m_element.tagValue("maxstay"));
        case DiaperChangingTable:
            // TODO look for changing_table:location too
            return translatedBoolValue(m_element.tagValue("changing_table", "diaper"));
        case Wikipedia: return wikipediaUrl(m_element.tagValue("wikipedia", "brand:wikipedia", QLocale()));
        case Address: return QVariant::fromValue(OSMAddress(m_element));
        case Phone: return QString::fromUtf8(m_element.tagValue("contact:phone", "phone", "telephone", "operator:phone"));
        case Email: return QString::fromUtf8(m_element.tagValue("contact:email", "email", "operator:email"));
        case Website: return QString::fromUtf8(m_element.tagValue("website", "contact:website", "url", "operator:website"));
        case PaymentCash:
        {
            const auto coins = m_element.tagValue("payment:coins");
            const auto notes = m_element.tagValue("payment:notes");
            if (coins.isEmpty() && notes.isEmpty()) {
                return translatedBoolValue(m_element.tagValue("payment:cash"));
            }
            if (coins == "yes" && notes == "yes") {
                return i18n("yes");
            }
            if (coins == "yes") {
                return i18nc("payment option", "coins only");
            }
            if (notes == "yes") {
                return i18nc("payment option", "notes only");
            }
            return i18n("no");
        }
        case PaymentDigital:
        case PaymentDebitCard:
        case PaymentCreditCard:
        case PaymentStoredValueCard:
            return paymentMethodValue(info.key);
        case Wheelchair:
        {
            QByteArray wheelchair;
            if (info.category == Toilets) {
                wheelchair = m_element.tagValue("toilets:wheelchair", "wheelchair");
            } else {
                wheelchair = m_element.tagValue("wheelchair");
            }
            const auto a = translateValue(wheelchair.constData(), wheelchair_map);
            const auto d = QString::fromUtf8(m_element.tagValue("wheelchair:description", QLocale()));
            if (!d.isEmpty()) {
                return QString(a + QLatin1String(" (") + d + QLatin1Char(')'));
            }
            return a;
        }
        case CentralKey:
            // translate enum values
            return QString::fromUtf8(m_element.tagValue("centralkey"));
        case OperatorName: return QString::fromUtf8(m_element.tagValue("operator"));
        case Network: return QString::fromUtf8(m_element.tagValue("network"));
        case OperatorWikipedia: return wikipediaUrl(m_element.tagValue("operator:wikipedia", QLocale()));
        case RemainingRange:
        {
            const auto range = m_element.tagValue("mx:remaining_range").toInt();
            return formatDistance(range);
        }
        case DebugLink: return m_element.url();
        case DebugKey: return {};
    }
    return {};
}

QVariant OSMElementInformationModel::urlify(const QVariant& v, OSMElementInformationModel::Key key) const
{
    if (v.type() != QVariant::String) {
        return v;
    }
    const auto s = v.toString();

    switch (key) {
        case Email:
            if (!s.startsWith(QLatin1String("mailto:"))) {
                return QString(QLatin1String("mailto:") + s);
            }
            return s;
        case Phone:
        {
            if (s.startsWith(QLatin1String("tel:"))) {
                return s;
            }
            QString e = QLatin1String("tel:") + s;
            e.remove(QLatin1Char(' '));
            return e;
        }
        case Website:
        case DebugLink:
            if (s.startsWith(QLatin1String("http"))) {
                return s;
            }
            return QString(QLatin1String("https://") + s);
        default:
            return {};
    }

    return {};
}

QString OSMElementInformationModel::paymentMethodList(OSMElementInformationModel::Key key) const
{
    QStringList l;
    for (const auto &payment : payment_type_map) {
        if (payment.key() != key) {
            continue;
        }
        if (m_element.tagValue(payment.keyName) == "yes") {
            l.push_back(payment.label.toString());
        }
    }
    std::sort(l.begin(), l.end());
    return QLocale().createSeparatedList(l);
}

QString OSMElementInformationModel::paymentMethodValue(OSMElementInformationModel::Key key) const
{
    const auto s = paymentMethodList(key);
    if (!s.isEmpty()) {
        return s;
    }

    for (const auto &payment : payment_generic_type_map) {
        if (payment.key() != key) {
            continue;
        }
        const auto s = m_element.tagValue(payment.keyName);
        if (!s.isEmpty()) {
            return QString::fromUtf8(s);
        }
    }
    return {};
}

QUrl OSMElementInformationModel::wikipediaUrl(const QByteArray &wp) const
{
    if (wp.isEmpty()) {
        return {};
    }

    const auto s = QString::fromUtf8(wp);
    const auto idx = s.indexOf(QLatin1Char(':'));
    if (idx < 0) {
        return {};
    }

    QUrl url;
    url.setScheme(QStringLiteral("https"));
    url.setHost(QStringView(s).left(idx) + QLatin1String(".wikipedia.org"));
    url.setPath(QLatin1String("/wiki/") + QStringView(s).mid(idx + 1));
    return url;
}

QString OSMElementInformationModel::capacitryValue(const char *prop) const
{
    const auto v = m_element.tagValue(prop);
    return translatedBoolValue(v);
}

QString OSMElementInformationModel::translatedBoolValue(const QByteArray &value) const
{
    if (value == "yes") {
        return i18n("yes");
    }
    if (value == "no") {
        return i18n("no");
    }
    return QString::fromUtf8(value);
}
