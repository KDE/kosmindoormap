/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "localization_p.h"

#include <KLazyLocalizedString>

namespace KOSMIndoorMap {

// TODO expand this, see https://taginfo.openstreetmap.org/keys/cuisine#values
static constexpr const ValueMapEntry cuisine_map[] = {
    { "american", kli18nc("OSM::cuisine", "American") },
    { "arab", kli18nc("OSM::cuisine", "Arab") },
    { "argentinian", kli18nc("OSM::cuisine", "Argentinian") },
    { "asian", kli18nc("OSM::cuisine", "Asian") },
    { "austrian", kli18nc("OSM::cuisine", "Austrian") },
    { "barbecue", kli18nc("OSM::cuisine", "BBQ") },
    { "bbq", kli18nc("OSM::cuisine", "BBQ") },
    { "brazilian", kli18nc("OSM::cuisine", "Brazilian") },
    { "breakfast", kli18nc("OSM::cuisine", "Breakfast") },
    { "burger", kli18nc("OSM::cuisine", "Burger") },
    { "cake", kli18nc("OSM::cuisine", "Cake") },
    { "chicken", kli18nc("OSM::cuisine", "Chicken") },
    { "chinese", kli18nc("OSM::cuisine", "Chinese") },
    { "coffee_shop", kli18nc("OSM::cuisine", "Coffee Shop") },
    { "cookies", kli18nc("OSM::cuisine", "Cookies") },
    { "crepe", kli18nc("OSM::cuisine", "Crêpe") },
    { "donut", kli18nc("OSM::cuisine", "Donut") },
    { "falafel", kli18nc("OSM::cuisine", "Falafel") },
    { "fish", kli18nc("OSM::cuisine", "Fish") },
    { "fish_and_chips", kli18nc("OSM::cuisine", "Fish & Chips") },
    { "french", kli18nc("OSM::cuisine", "French") },
    { "german", kli18nc("OSM::cuisine", "German") },
    { "greek", kli18nc("OSM::cuisine", "Greek") },
    { "ice_cream", kli18nc("OSM::cuisine", "Ice Cream") },
    { "indian", kli18nc("OSM::cuisine", "Indian") },
    { "indonesian", kli18nc("OSM::cuisine", "Indonesian") },
    { "international", kli18nc("OSM::cuisine", "International") },
    { "italian", kli18nc("OSM::cuisine", "Italian") },
    { "italian_pizza", kli18nc("OSM::cuisine", "Pizza") },
    { "japanese", kli18nc("OSM::cuisine", "Japanese") },
    { "juice", kli18nc("OSM::cuisine", "Juice") },
    { "kebab", kli18nc("OSM::cuisine", "Kebab") },
    { "korean", kli18nc("OSM::cuisine", "Korean") },
    { "lebanese", kli18nc("OSM::cuisine", "Lebanese") },
    { "local", kli18nc("OSM::cuisine", "Local") },
    { "mediterranean", kli18nc("OSM::cuisine", "Mediterranean") },
    { "mexican", kli18nc("OSM::cuisine", "Mexican") },
    { "noodle", kli18nc("OSM::cuisine", "Noodle") },
    { "pakistani", kli18nc("OSM::cuisine", "Pakistani") },
    { "pancake", kli18nc("OSM::cuisine", "Pancake") },
    { "pasta", kli18nc("OSM::cuisine", "Pasta") },
    { "pizza", kli18nc("OSM::cuisine", "Pizza") },
    { "polish", kli18nc("OSM::cuisine", "Polish") },
    { "portuguese", kli18nc("OSM::cuisine", "Portuguese") },
    { "ramen", kli18nc("OSM::cuisine", "Ramen") },
    { "regional", kli18nc("OSM::cuisine", "Regional") },
    { "salad", kli18nc("OSM::cuisine", "Salad") },
    { "sandwich", kli18nc("OSM::cuisine", "Sandwich") },
    { "sausage", kli18nc("OSM::cuisine", "Sausage") },
    { "seafood", kli18nc("OSM::cuisine", "Seafood") },
    { "soup", kli18nc("OSM::cuisine", "Soup") },
    { "spanish", kli18nc("OSM::cuisine", "Spanish") },
    { "steak", kli18nc("OSM::cuisine", "Steak") },
    { "steak_house", kli18nc("OSM::cuisine", "Steak") },
    { "sushi", kli18nc("OSM::cuisine", "Sushi") },
    { "tapas", kli18nc("OSM::cuisine", "Tapas") },
    { "thai", kli18nc("OSM::cuisine", "Thai") },
    { "turkish", kli18nc("OSM::cuisine", "Turkish") },
    { "vegetarian", kli18nc("OSM::cuisine", "Vegetarian") },
    { "vietnamese", kli18nc("OSM::cuisine", "Vietnamese") },
};
static_assert(isSortedLookupTable(cuisine_map), "cuising map is not sorted!");

// diet types offered at restaurants
struct {
    const char *keyName;
    const KLazyLocalizedString label;

    constexpr inline OSMElementInformationModel::Key key() const { return OSMElementInformationModel::Diet; }
    constexpr inline OSMElementInformationModel::KeyCategory category() const { return OSMElementInformationModel::Main; }
} static constexpr const diet_type_map[] = {
    { "diet:gluten_free", kli18nc("OSM::diet_type", "gluten free") },
    { "diet:halal", kli18nc("OSM::diet_type", "halal") },
    { "diet:kosher", kli18nc("OSM::diet_type", "kosher") },
    { "diet:lactose_free", kli18nc("OSM::diet_type", "lactose free") },
    { "diet:vegan", kli18nc("OSM::diet_type", "vegan") },
    { "diet:vegetarian", kli18nc("OSM::diet_type", "vegetarian") },
};
static_assert(isSortedLookupTable(diet_type_map), "diet type map is not sorted!");

// generic payment types (excluding cash, that's handled separately)
struct {
    const char *keyName;
    OSMElementInformationModel::Key m_key;

    constexpr inline OSMElementInformationModel::Key key() const { return m_key; }
    constexpr inline OSMElementInformationModel::KeyCategory category() const { return OSMElementInformationModel::Payment; }
} static constexpr const payment_generic_type_map[] = {
    { "payment:account_cards", OSMElementInformationModel::PaymentDebitCard },
    { "payment:credit_cards", OSMElementInformationModel::PaymentCreditCard },
    { "payment:debit_cards", OSMElementInformationModel::PaymentDebitCard },
    { "payment:electronic_purses", OSMElementInformationModel::PaymentStoredValueCard },
};
static_assert(isSortedLookupTable(payment_generic_type_map), "generic payment type map is not sorted!");

// payment vendor types only, generic ones go into the list above and are handled separately
struct {
    const char *keyName;
    OSMElementInformationModel::Key m_key;
    const KLazyLocalizedString label;

    constexpr inline OSMElementInformationModel::Key key() const { return m_key; }
    constexpr inline OSMElementInformationModel::KeyCategory category() const { return OSMElementInformationModel::Payment; }
} static constexpr const payment_type_map[] = {
    { "payment:american_express", OSMElementInformationModel::PaymentCreditCard, kli18nc("OSM::payment_method", "American Express") },
    { "payment:apple_pay", OSMElementInformationModel::PaymentDigital, kli18nc("OSM::payment_method", "Apple Pay") },
    { "payment:diners_club", OSMElementInformationModel::PaymentCreditCard, kli18nc("OSM::payment_method", "Diners Club") },
    { "payment:discover_card", OSMElementInformationModel::PaymentCreditCard, kli18nc("OSM::payment_method", "Discover Card") },
    { "payment:jcb", OSMElementInformationModel::PaymentCreditCard, kli18nc("OSM::payment_method", "JCB") },
    { "payment:girocard", OSMElementInformationModel::PaymentDebitCard, kli18nc("OSM::payment_method", "Girocard") },
    { "payment:google_pay", OSMElementInformationModel::PaymentDigital, kli18nc("OSM::payment_method", "Google Pay") },
    { "payment:maestro", OSMElementInformationModel::PaymentDebitCard, kli18nc("OSM::payment_method", "Maestro") },
    { "payment:mastercard", OSMElementInformationModel::PaymentCreditCard, kli18nc("OSM::payment_method", "Mastercard") },
    { "payment:unionpay", OSMElementInformationModel::PaymentCreditCard, kli18nc("OSM::payment_method", "UnionPay") },
    { "payment:v_pay", OSMElementInformationModel::PaymentCreditCard, kli18nc("OSM::payment_method", "V Pay") },
    { "payment:vpay", OSMElementInformationModel::PaymentCreditCard, kli18nc("OSM::payment_method", "V Pay") },
    { "payment:visa", OSMElementInformationModel::PaymentCreditCard, kli18nc("OSM::payment_method", "Visa") },
};

static constexpr const ValueMapEntry wheelchair_map[] = {
    { "limited", kli18nc("OSM::wheelchair_access", "limited") },
    { "no", kli18nc("OSM::wheelchair_access", "no") },
    { "yes", kli18nc("OSM::wheelchair_access", "yes") },
};
static_assert(isSortedLookupTable(wheelchair_map), "wheelchair access map is not sorted!");

// socket types for charging stations
struct {
    const char *keyName;
    const KLazyLocalizedString label;

    constexpr inline OSMElementInformationModel::Key key() const { return OSMElementInformationModel::Socket; }
    constexpr inline OSMElementInformationModel::KeyCategory category() const { return OSMElementInformationModel::Main; }
} static constexpr const socket_type_map[] = {
    { "socket:chademo", kli18nc("OSM::charging_station_socket", "Chademo") },
    { "socket:schuko", kli18nc("OSM::charging_station_socket", "Schuko") },
    { "socket:tesla_standard", kli18nc("OSM::charging_station_socket", "Tesla") },
    { "socket:tesla_supercharger", kli18nc("OSM::charging_station_socket", "Tesla Supercharger") },
    { "socket:tesla_supercharger_ccs", kli18nc("OSM::charging_station_socket", "Tesla Supercharger CCS") },
    { "socket:type2", kli18nc("OSM::charging_station_socket", "Type 2") },
    { "socket:type2_cable", kli18nc("OSM::charging_station_socket", "Type 2 cable") },
    { "socket:type2_combo", kli18nc("OSM::charging_station_socket", "Type 2 CCS") },
    { "socket:typee", kli18nc("OSM::charging_station_socket", "Type E") },
};
static_assert(isSortedLookupTable(socket_type_map), "socket type map is not sorted!");

// charging station authentication methods
struct {
    const char *keyName;
    const KLazyLocalizedString label;

    constexpr inline OSMElementInformationModel::Key key() const { return OSMElementInformationModel::Authentication; }
    constexpr inline OSMElementInformationModel::KeyCategory category() const { return OSMElementInformationModel::Main; }
} static constexpr const authentication_type_map[] = {
    { "authentication:app", kli18nc("OSM::charging_station_authentication", "app") },
    { "authentication:membership_card", kli18nc("OSM::charging_station_authentication", "membership card") },
    { "authentication:nfc", kli18nc("OSM::charging_station_authentication", "NFC") },
    { "authentication:none", kli18nc("OSM::charging_station_authentication", "none") },
    { "authentication:phone_call", kli18nc("OSM::charging_station_authentication", "phone call") },
    { "authentication:short_message", kli18nc("OSM::charging_station_authentication", "SMS") },
};
static_assert(isSortedLookupTable(authentication_type_map), "authentication type map is not sorted!");

// bicycle parking values
// see https://taginfo.openstreetmap.org/keys/?key=bicycle_parking#values
static constexpr const ValueMapEntry bicycle_parking_map[] = {
    { "anchors", kli18nc("OSM::bicycle_parking", "anchors") },
    { "bollard", kli18nc("OSM::bicycle_parking", "bollard") },
    { "building", kli18nc("OSM::bicycle_parking", "building") },
    { "ground_slots", kli18nc("OSM::bicycle_parking", "ground slots") },
    { "lockers", kli18nc("OSM::bicycle_parking", "lockers") },
    { "racks", kli18nc("OSM::bicycle_parking", "racks") },
    { "shed", kli18nc("OSM::bicycle_parking", "shed") },
    { "stands", kli18nc("OSM::bicycle_parking", "stands") },
    { "wall_loops", kli18nc("OSM::bicycle_parking", "wall loops") },
    { "wide_stands", kli18nc("OSM::bicycle_parking", "wide stands") },
};
static_assert(isSortedLookupTable(bicycle_parking_map), "bicycle parking map is not sorted!");

// shared vehicle types
// tag keys are our extension, based on KPublicTransport data
static constexpr const ValueMapEntry available_vehicles_map[] = {
    { "mx:realtime_available:bike", kli18ncp("available rental vehicles", "%1 bike", "%1 bikes") },
    { "mx:realtime_available:pedelec", kli18ncp("available rental vehicles", "%1 pedelec", "%1 pedelecs") },
    { "mx:realtime_available:scooter", kli18ncp("available rental vehicles", "%1 kick scooter", "%1 kick scooters") },
    { "mx:realtime_available:motorcycle", kli18ncp("available rental vehicles", "%1 moped", "%1 mopeds") },
    { "mx:realtime_available:car", kli18ncp("available rental vehicles", "%1 car", "%1 cars") },
};

// gender neutral/gender segregated facilities
struct {
    const char *keyName;
    const KLazyLocalizedString label;

    constexpr inline OSMElementInformationModel::Key key() const { return OSMElementInformationModel::Gender; }
    constexpr inline OSMElementInformationModel::KeyCategory category() const { return OSMElementInformationModel::UnresolvedCategory; }
} static constexpr const gender_type_map[] = {
    { "female", kli18nc("OSM::gender_segregation", "female") },
    { "male", kli18nc("OSM::gender_segregation", "male") },
    { "unisex", kli18nc("OSM::gender_segregation", "unisex") },
};
static_assert(isSortedLookupTable(gender_type_map), "gender type map is not sorted!");

// tactile writing variants
struct {
    const char *keyName;
    const KLazyLocalizedString label;

    constexpr inline OSMElementInformationModel::Key key() const { return OSMElementInformationModel::TactileWriting; }
    constexpr inline OSMElementInformationModel::KeyCategory category() const { return OSMElementInformationModel::Accessibility; }
} static constexpr const tactile_writing_map[] = {
    { "tactile_writing:braille", kli18nc("tactile writing", "braille") },
    { "tactile_writing:embossed_printed_letters", kli18nc("tactile writing", "embossed printed letters") },
    { "tactile_writing:engraved_printed_letters", kli18nc("tactile writing", "engraved printed letters") },
};
static_assert(isSortedLookupTable(tactile_writing_map), "tactile writing type map is not sorted!");

}
