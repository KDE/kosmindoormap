/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "osmelementinformationmodel.h"
#include "osmelementinformationmodel_p.h"

#include <KLocalizedString>

namespace KOSMIndoorMap {

// TODO expand this, see:
// - https://taginfo.openstreetmap.org/keys/?key=amenity#values
// - https://taginfo.openstreetmap.org/keys/?key=shop#values
// - https://taginfo.openstreetmap.org/keys/?key=tourism#values
// - parts of https://taginfo.openstreetmap.org/keys/?key=office#values
// - parts of https://taginfo.openstreetmap.org/keys/?key=room#values
static constexpr const ValueMapEntry amenity_map[] = {
    { "alcohol", I18N_NOOP2("OSM::amenity/shop", "Alcohol") },
    { "atm", I18N_NOOP2("OSM::amenity/shop", "ATM") },
    { "attraction", I18N_NOOP2("OSM::amenity/shop", "Attraction") },
    { "bag", I18N_NOOP2("OSM::amenity/shop", "Bag") },
    { "bakery", I18N_NOOP2("OSM::amenity/shop", "Bakery") },
    { "bank", I18N_NOOP2("OSM::amenity/shop", "Bank") },
    { "bar", I18N_NOOP2("OSM::amenity/shop", "Bar") },
    { "beauty", I18N_NOOP2("OSM::amenity/shop", "Beauty") },
    { "bed", I18N_NOOP2("OSM::amenity/shop", "Bed") },
    { "beverages", I18N_NOOP2("OSM::amenity/shop", "Beverages") },
    { "bicycle", I18N_NOOP2("OSM::amenity/shop", "Bicycle") },
    { "bicycle_parking", I18N_NOOP2("OSM::amenity/shop", "Bicycle Parking") },
    { "bicycle_rental", I18N_NOOP2("OSM::amenity/shop", "Bicycle Rental") },
    { "books", I18N_NOOP2("OSM::amenity/shop", "Books") },
    { "boutique", I18N_NOOP2("OSM::amenity/shop", "Boutique") },
    { "bureau_de_change", I18N_NOOP2("OSM::amenity/shop", "Bureau de Change") },
    { "butcher", I18N_NOOP2("OSM::amenity/shop", "Butcher") },
    { "cafe", I18N_NOOP2("OSM::amenity/shop", "Cafe") },
    { "car", I18N_NOOP2("OSM::amenity/shop", "Car") },
    { "car_rental", I18N_NOOP2("OSM::amenity/shop", "Car Rental") },
    { "car_sharing", I18N_NOOP2("OSM::amenity/shop", "Car Sharing") },
    { "charging_station", I18N_NOOP2("OSM::amenity/shop", "Charging Station") },
    { "chemist", I18N_NOOP2("OSM::amenity/shop", "Chemist") },
    { "chocolate", I18N_NOOP2("OSM::amenity/shop", "Chocolate") },
    { "cinema", I18N_NOOP2("OSM::amenity/shop", "Cinema") },
    { "clothes", I18N_NOOP2("OSM::amenity/shop", "Clothes") },
    { "coffee", I18N_NOOP2("OSM::amenity/shop", "Coffee") },
    { "computer", I18N_NOOP2("OSM::amenity/shop", "Computer") },
    { "confectionery", I18N_NOOP2("OSM::amenity/shop", "Confectionery") },
    { "convenience", I18N_NOOP2("OSM::amenity/shop", "Convenience Store") },
    { "copyshop", I18N_NOOP2("OSM::amenity/shop", "Copy Shop") },
    { "cosmetics", I18N_NOOP2("OSM::amenity/shop", "Cosmetics") },
    { "courthouse", I18N_NOOP2("OSM::amenity/shop", "Court House") },
    { "deli", I18N_NOOP2("OSM::amenity/shop", "Deli") },
    { "department_store", I18N_NOOP2("OSM::amenity/shop", "Department Store") },
    { "doctors", I18N_NOOP2("OSM::amenity/shop", "Doctor") },
    { "doityourself", I18N_NOOP2("OSM::amenity/shop", "Hardware Store") },
    { "drinking_water", I18N_NOOP2("OSM::amenity/shop", "Drinking Water") },
    { "dry_cleaning", I18N_NOOP2("OSM::amenity/shop", "Dry Cleaning") },
    { "electronics", I18N_NOOP2("OSM::amenity/shop", "Electronics") },
    { "fashion", I18N_NOOP2("OSM::amenity/shop", "Fashion") },
    { "fast_food", I18N_NOOP2("OSM::amenity/shop", "Fast Food") },
    { "ferry_terminal", I18N_NOOP2("OSM::amenity/shop", "Ferry Terminal") },
    { "florist", I18N_NOOP2("OSM::amenity/shop", "Florist") },
    { "food_court", I18N_NOOP2("OSM::amenity/shop", "Food Court") },
    { "fountain", I18N_NOOP2("OSM::amenity/shop", "Fountain") },
    { "furniture", I18N_NOOP2("OSM::amenity/shop", "Furniture") },
    { "gallery", I18N_NOOP2("OSM::amenity/shop", "Gallery") },
    { "garden_centre", I18N_NOOP2("OSM::amenity/shop", "Garden Center") },
    { "gift", I18N_NOOP2("OSM::amenity/shop", "Gift Shop") },
    { "greengrocer", I18N_NOOP2("OSM::amenity/shop", "Greengrocer") },
    { "guest_house", I18N_NOOP2("OSM::amenity/shop", "Guest House") },
    { "hairdresser", I18N_NOOP2("OSM::amenity/shop", "Hairdresser") },
    { "hearing_aids", I18N_NOOP2("OSM::amenity/shop", "Hearing Aids") },
    { "hospital", I18N_NOOP2("OSM::amenity/shop", "Hospital") },
    { "hostel", I18N_NOOP2("OSM::amenity/shop", "Hostel") },
    { "hotel", I18N_NOOP2("OSM::amenity/shop", "Hotel") },
    { "houseware", I18N_NOOP2("OSM::amenity/shop", "Houseware") },
    { "ice_cream", I18N_NOOP2("OSM::amenity/shop", "Ice Cream") },
    { "information", I18N_NOOP2("OSM::amenity/shop", "Information") },
    { "interior_decoration", I18N_NOOP2("OSM::amenity/shop", "Interior Decoration") },
    { "internet_cafe", I18N_NOOP2("OSM::amenity/shop", "Internet Cafe") },
    { "jewelry", I18N_NOOP2("OSM::amenity/shop", "Jewelry") },
    { "kiosk", I18N_NOOP2("OSM::amenity/shop", "Kiosk") },
    { "laundry", I18N_NOOP2("OSM::amenity/shop", "Laundry") },
    { "library", I18N_NOOP2("OSM::amenity/shop", "Library") },
    { "lockers", I18N_NOOP2("OSM::amenity/shop", "Locker") },
    { "locksmith", I18N_NOOP2("OSM::amenity/shop", "Locksmith") },
    { "lost_and_found", I18N_NOOP2("OSM::amenity/shop", "Lost & Found") },
    { "lost_property", I18N_NOOP2("OSM::amenity/shop", "Lost & Found") },
    { "luggage_locker", I18N_NOOP2("OSM::amenity/shop", "Locker") },
    { "mall", I18N_NOOP2("OSM::amenity/shop", "Mall") },
    { "medical_supply", I18N_NOOP2("OSM::amenity/shop", "Medical Supply") },
    { "mobile_phone", I18N_NOOP2("OSM::amenity/shop", "Mobile Phone") },
    { "money_transfer", I18N_NOOP2("OSM::amenity/shop", "Money Transfer") },
    { "motorcycle_parking", I18N_NOOP2("OSM::amenity/shop", "Motorcycle Parking") },
    { "motorcycle_rental", I18N_NOOP2("OSM::amenity/shop", "Motorcycle Rental") },
    { "museum", I18N_NOOP2("OSM::amenity/shop", "Museum") },
    { "music", I18N_NOOP2("OSM::amenity/shop", "Music") },
    { "musical_instrument", I18N_NOOP2("OSM::amenity/shop", "Musical Instruments") },
    { "newsagent", I18N_NOOP2("OSM::amenity/shop", "Newsagent") },
    { "office", I18N_NOOP2("OSM::amenity/shop", "Office") },
    { "optician", I18N_NOOP2("OSM::amenity/shop", "Optician") },
    { "outdoor", I18N_NOOP2("OSM::amenity/shop", "Outdoor") },
    { "paint", I18N_NOOP2("OSM::amenity/shop", "Paint") },
    { "parking", I18N_NOOP2("OSM::amenity/shop", "Parking") },
    { "parking_tickets", I18N_NOOP2("OSM::amenity/shop", "Parking Tickets") },
    { "pastry", I18N_NOOP2("OSM::amenity/shop", "Pastry") },
    { "perfumery", I18N_NOOP2("OSM::amenity/shop", "Perfumery") },
    { "pet", I18N_NOOP2("OSM::amenity/shop", "Pet") },
    { "pharmacy", I18N_NOOP2("OSM::amenity/shop", "Pharmacy") },
    { "photo", I18N_NOOP2("OSM::amenity/shop", "Photo") },
    { "place_of_worship", I18N_NOOP2("OSM::amenity/shop", "Place of Worship") },
    { "police", I18N_NOOP2("OSM::amenity/shop", "Police") },
    { "post_box", I18N_NOOP2("OSM::amenity/shop", "Post Box") },
    { "post_office", I18N_NOOP2("OSM::amenity/shop", "Post Office") },
    { "pub", I18N_NOOP2("OSM::amenity/shop", "Pub") },
    { "public_transport_tickets", I18N_NOOP2("OSM::amenity/shop", "Public Transport Tickets") },
    { "recycling", I18N_NOOP2("OSM::amenity/shop", "Recycling") },
    { "restaurant", I18N_NOOP2("OSM::amenity/shop", "Restaurant") },
    { "school", I18N_NOOP2("OSM::amenity/shop", "School") },
    { "scooter_rental", I18N_NOOP2("OSM::amenity/shop", "Kick Scooter Rental") },
    { "seafood", I18N_NOOP2("OSM::amenity/shop", "Seafood") },
    { "shoes", I18N_NOOP2("OSM::amenity/shop", "Shoes") },
    { "shop", I18N_NOOP2("OSM::amenity/shop", "Shop") },
    { "social_facility", I18N_NOOP2("OSM::amenity/shop", "Social Facility") },
    { "souveniers", I18N_NOOP2("OSM::amenity/shop", "Souvenirs") },
    { "sports", I18N_NOOP2("OSM::amenity/shop", "Sports") },
    { "stationery", I18N_NOOP2("OSM::amenity/shop", "Stationery") },
    { "supermarket", I18N_NOOP2("OSM::amenity/shop", "Supermarket") },
    { "tailor", I18N_NOOP2("OSM::amenity/shop", "Tailor") },
    { "tatoo", I18N_NOOP2("OSM::amenity/shop", "Tattoo") },
    { "taxi", I18N_NOOP2("OSM::amenity/shop", "Taxi") },
    { "tea", I18N_NOOP2("OSM::amenity/shop", "Tea") },
    { "theatre", I18N_NOOP2("OSM::amenity/shop", "Theatre") },
    { "ticket", I18N_NOOP2("OSM::amenity/shop", "Tickets") },
    { "tobacco", I18N_NOOP2("OSM::amenity/shop", "Tobacco") },
    { "toilets", I18N_NOOP2("OSM::amenity/shop", "Toilets") },
    { "townhall", I18N_NOOP2("OSM::amenity/shop", "Town Hall") },
    { "toys", I18N_NOOP2("OSM::amenity/shop", "Toys") },
    { "travel_agency", I18N_NOOP2("OSM::amenity/shop", "Travel Agency") },
    { "travel_agent", I18N_NOOP2("OSM::amenity/shop", "Travel Agency") },
    { "university", I18N_NOOP2("OSM::amenity/shop", "University") },
    { "variety_store", I18N_NOOP2("OSM::amenity/shop", "Variety Store") },
    { "video_games", I18N_NOOP2("OSM::amenity/shop", "Video Games") },
    { "waiting", I18N_NOOP2("OSM::amenity/shop", "Waiting Area") },
    { "waiting_area", I18N_NOOP2("OSM::amenity/shop", "Waiting Area") },
    { "waiting_room", I18N_NOOP2("OSM::amenity/shop", "Waiting Area") },
    { "wine", I18N_NOOP2("OSM::amenity/shop", "Wine") },
};
static_assert(isSortedLookupTable(amenity_map), "amenity map is not sorted!");

// TODO expand this, see https://taginfo.openstreetmap.org/keys/cuisine#values
static constexpr const ValueMapEntry cuisine_map[] = {
    { "american", I18N_NOOP2("OSM::cuisine", "American") },
    { "arab", I18N_NOOP2("OSM::cuisine", "Arab") },
    { "argentinian", I18N_NOOP2("OSM::cuisine", "Argentinian") },
    { "asian", I18N_NOOP2("OSM::cuisine", "Asian") },
    { "austrian", I18N_NOOP2("OSM::cuisine", "Austrian") },
    { "barbecue", I18N_NOOP2("OSM::cuisine", "BBQ") },
    { "bbq", I18N_NOOP2("OSM::cuisine", "BBQ") },
    { "brazilian", I18N_NOOP2("OSM::cuisine", "Brazilian") },
    { "breakfast", I18N_NOOP2("OSM::cuisine", "Breakfast") },
    { "burger", I18N_NOOP2("OSM::cuisine", "Burger") },
    { "cake", I18N_NOOP2("OSM::cuisine", "Cake") },
    { "chicken", I18N_NOOP2("OSM::cuisine", "Chicken") },
    { "chinese", I18N_NOOP2("OSM::cuisine", "Chinese") },
    { "coffee_shop", I18N_NOOP2("OSM::cuisine", "Coffee Shop") },
    { "cookies", I18N_NOOP2("OSM::cuisine", "Cookies") },
    { "crepe", I18N_NOOP2("OSM::cuisine", "Crêpe") },
    { "donut", I18N_NOOP2("OSM::cuisine", "Donut") },
    { "falafel", I18N_NOOP2("OSM::cuisine", "Falafel") },
    { "fish", I18N_NOOP2("OSM::cuisine", "Fish") },
    { "fish_and_chips", I18N_NOOP2("OSM::cuisine", "Fish & Chips") },
    { "french", I18N_NOOP2("OSM::cuisine", "French") },
    { "german", I18N_NOOP2("OSM::cuisine", "German") },
    { "greek", I18N_NOOP2("OSM::cuisine", "Greek") },
    { "ice_cream", I18N_NOOP2("OSM::cuisine", "Ice Cream") },
    { "indian", I18N_NOOP2("OSM::cuisine", "Indian") },
    { "indonesian", I18N_NOOP2("OSM::cuisine", "Indonesian") },
    { "international", I18N_NOOP2("OSM::cuisine", "International") },
    { "italian", I18N_NOOP2("OSM::cuisine", "Italian") },
    { "italian_pizza", I18N_NOOP2("OSM::cuisine", "Pizza") },
    { "japanese", I18N_NOOP2("OSM::cuisine", "Japanese") },
    { "juice", I18N_NOOP2("OSM::cuisine", "Juice") },
    { "kebab", I18N_NOOP2("OSM::cuisine", "Kebab") },
    { "korean", I18N_NOOP2("OSM::cuisine", "Korean") },
    { "lebanese", I18N_NOOP2("OSM::cuisine", "Lebanese") },
    { "local", I18N_NOOP2("OSM::cuisine", "Local") },
    { "mediterranean", I18N_NOOP2("OSM::cuisine", "Mediterranean") },
    { "mexican", I18N_NOOP2("OSM::cuisine", "Mexican") },
    { "noodle", I18N_NOOP2("OSM::cuisine", "Noodle") },
    { "pakistani", I18N_NOOP2("OSM::cuisine", "Pakistani") },
    { "pancake", I18N_NOOP2("OSM::cuisine", "Pancake") },
    { "pasta", I18N_NOOP2("OSM::cuisine", "Pasta") },
    { "pizza", I18N_NOOP2("OSM::cuisine", "Pizza") },
    { "polish", I18N_NOOP2("OSM::cuisine", "Polish") },
    { "portuguese", I18N_NOOP2("OSM::cuisine", "Portuguese") },
    { "ramen", I18N_NOOP2("OSM::cuisine", "Ramen") },
    { "regional", I18N_NOOP2("OSM::cuisine", "Regional") },
    { "salad", I18N_NOOP2("OSM::cuisine", "Salad") },
    { "sandwich", I18N_NOOP2("OSM::cuisine", "Sandwich") },
    { "sausage", I18N_NOOP2("OSM::cuisine", "Sausage") },
    { "seafood", I18N_NOOP2("OSM::cuisine", "Seafood") },
    { "soup", I18N_NOOP2("OSM::cuisine", "Soup") },
    { "spanish", I18N_NOOP2("OSM::cuisine", "Spanish") },
    { "steak", I18N_NOOP2("OSM::cuisine", "Steak") },
    { "steak_house", I18N_NOOP2("OSM::cuisine", "Steak") },
    { "sushi", I18N_NOOP2("OSM::cuisine", "Sushi") },
    { "tapas", I18N_NOOP2("OSM::cuisine", "Tapas") },
    { "thai", I18N_NOOP2("OSM::cuisine", "Thai") },
    { "turkish", I18N_NOOP2("OSM::cuisine", "Turkish") },
    { "vegetarian", I18N_NOOP2("OSM::cuisine", "Vegetarian") },
    { "vietnamese", I18N_NOOP2("OSM::cuisine", "Vietnamese") },
};
static_assert(isSortedLookupTable(cuisine_map), "cuising map is not sorted!");

// diet types offered at restaurants
struct {
    const char *keyName;
    const char *label;

    constexpr inline OSMElementInformationModel::Key key() const { return OSMElementInformationModel::Diet; }
    constexpr inline OSMElementInformationModel::KeyCategory category() const { return OSMElementInformationModel::Main; }
} static constexpr const diet_type_map[] = {
    { "diet:gluten_free", I18N_NOOP2("OSM::diet_type", "gluten free") },
    { "diet:halal", I18N_NOOP2("OSM::diet_type", "halal") },
    { "diet:kosher", I18N_NOOP2("OSM::diet_type", "kosher") },
    { "diet:lactose_free", I18N_NOOP2("OSM::diet_type", "lactose free") },
    { "diet:vegan", I18N_NOOP2("OSM::diet_type", "vegan") },
    { "diet:vegetarian", I18N_NOOP2("OSM::diet_type", "vegetarian") },
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
    const char *label;

    constexpr inline OSMElementInformationModel::Key key() const { return m_key; }
    constexpr inline OSMElementInformationModel::KeyCategory category() const { return OSMElementInformationModel::Payment; }
} static constexpr const payment_type_map[] = {
    { "payment:american_express", OSMElementInformationModel::PaymentCreditCard, I18N_NOOP2("OSM::payment_method", "American Express") },
    { "payment:apple_pay", OSMElementInformationModel::PaymentDigital, I18N_NOOP2("OSM::payment_method", "Apple Pay") },
    { "payment:diners_club", OSMElementInformationModel::PaymentCreditCard, I18N_NOOP2("OSM::payment_method", "Diners Club") },
    { "payment:discover_card", OSMElementInformationModel::PaymentCreditCard, I18N_NOOP2("OSM::payment_method", "Discover Card") },
    { "payment:jcb", OSMElementInformationModel::PaymentCreditCard, I18N_NOOP2("OSM::payment_method", "JCB") },
    { "payment:girocard", OSMElementInformationModel::PaymentDebitCard, I18N_NOOP2("OSM::payment_method", "Girocard") },
    { "payment:google_pay", OSMElementInformationModel::PaymentDigital, I18N_NOOP2("OSM::payment_method", "Google Pay") },
    { "payment:maestro", OSMElementInformationModel::PaymentDebitCard, I18N_NOOP2("OSM::payment_method", "Maestro") },
    { "payment:mastercard", OSMElementInformationModel::PaymentCreditCard, I18N_NOOP2("OSM::payment_method", "Mastercard") },
    { "payment:unionpay", OSMElementInformationModel::PaymentCreditCard, I18N_NOOP2("OSM::payment_method", "UnionPay") },
    { "payment:v_pay", OSMElementInformationModel::PaymentCreditCard, I18N_NOOP2("OSM::payment_method", "V Pay") },
    { "payment:vpay", OSMElementInformationModel::PaymentCreditCard, I18N_NOOP2("OSM::payment_method", "V Pay") },
    { "payment:visa", OSMElementInformationModel::PaymentCreditCard, I18N_NOOP2("OSM::payment_method", "Visa") },
};

static constexpr const ValueMapEntry wheelchair_map[] = {
    { "limited", I18N_NOOP2("OSM::wheelchair_access", "limited") },
    { "no", I18N_NOOP2("OSM::wheelchair_access", "no") },
    { "yes", I18N_NOOP2("OSM::wheelchair_access", "yes") },
};
static_assert(isSortedLookupTable(wheelchair_map), "wheelchair access map is not sorted!");

// socket types for charging stations
struct {
    const char *keyName;
    const char *label;

    constexpr inline OSMElementInformationModel::Key key() const { return OSMElementInformationModel::Socket; }
    constexpr inline OSMElementInformationModel::KeyCategory category() const { return OSMElementInformationModel::Main; }
} static constexpr const socket_type_map[] = {
    { "socket:chademo", I18N_NOOP2("OSM::charging_station_socket", "Chademo") },
    { "socket:schuko", I18N_NOOP2("OSM::charging_station_socket", "Schuko") },
    { "socket:tesla_standard", I18N_NOOP2("OSM::charging_station_socket", "Tesla") },
    { "socket:tesla_supercharger", I18N_NOOP2("OSM::charging_station_socket", "Tesla Supercharger") },
    { "socket:tesla_supercharger_ccs", I18N_NOOP2("OSM::charging_station_socket", "Tesla Supercharger CCS") },
    { "socket:type2", I18N_NOOP2("OSM::charging_station_socket", "Type 2") },
    { "socket:type2_cable", I18N_NOOP2("OSM::charging_station_socket", "Type 2 cable") },
    { "socket:type2_combo", I18N_NOOP2("OSM::charging_station_socket", "Type 2 CCS") },
    { "socket:typee", I18N_NOOP2("OSM::charging_station_socket", "Type E") },
};
static_assert(isSortedLookupTable(socket_type_map), "socket type map is not sorted!");

// charging station authentication methods
struct {
    const char *keyName;
    const char *label;

    constexpr inline OSMElementInformationModel::Key key() const { return OSMElementInformationModel::Authentication; }
    constexpr inline OSMElementInformationModel::KeyCategory category() const { return OSMElementInformationModel::Main; }
} static constexpr const authentication_type_map[] = {
    { "authentication:app", I18N_NOOP2("OSM::charging_station_authentication", "app") },
    { "authentication:membership_card", I18N_NOOP2("OSM::charging_station_authentication", "membership card") },
    { "authentication:nfc", I18N_NOOP2("OSM::charging_station_authentication", "NFC") },
    { "authentication:none", I18N_NOOP2("OSM::charging_station_authentication", "none") },
    { "authentication:phone_call", I18N_NOOP2("OSM::charging_station_authentication", "phone call") },
    { "authentication:short_message", I18N_NOOP2("OSM::charging_station_authentication", "SMS") },
};
static_assert(isSortedLookupTable(authentication_type_map), "authentication type map is not sorted!");

// bicycle parking values
// see https://taginfo.openstreetmap.org/keys/?key=bicycle_parking#values
static constexpr const ValueMapEntry bicycle_parking_map[] = {
    { "anchors", I18N_NOOP2("OSM::bicycle_parking", "anchors") },
    { "bollard", I18N_NOOP2("OSM::bicycle_parking", "bollard") },
    { "building", I18N_NOOP2("OSM::bicycle_parking", "building") },
    { "ground_slots", I18N_NOOP2("OSM::bicycle_parking", "ground slots") },
    { "lockers", I18N_NOOP2("OSM::bicycle_parking", "lockers") },
    { "racks", I18N_NOOP2("OSM::bicycle_parking", "racks") },
    { "shed", I18N_NOOP2("OSM::bicycle_parking", "shed") },
    { "stands", I18N_NOOP2("OSM::bicycle_parking", "stands") },
    { "wall_loops", I18N_NOOP2("OSM::bicycle_parking", "wall loops") },
    { "wide_stands", I18N_NOOP2("OSM::bicycle_parking", "wide stands") },
};
static_assert(isSortedLookupTable(bicycle_parking_map), "bicycle parking map is not sorted!");

}
