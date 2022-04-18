/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "osmelementinformationmodel.h"
#include "osmelementinformationmodel_p.h"

#include <KLazyLocalizedString>

namespace KOSMIndoorMap {

// TODO expand this, see:
// - https://taginfo.openstreetmap.org/keys/?key=amenity#values
// - https://taginfo.openstreetmap.org/keys/?key=shop#values
// - https://taginfo.openstreetmap.org/keys/?key=tourism#values
// - https://taginfo.openstreetmap.org/keys/?key=leisure#values
// - parts of https://taginfo.openstreetmap.org/keys/?key=office#values
// - parts of https://taginfo.openstreetmap.org/keys/?key=room#values
static constexpr const ValueMapEntry amenity_map[] = {
    { "alcohol", kli18nc("OSM::amenity/shop", "Alcohol") },
    { "apartment", kli18nc("OSM::amenity/shop", "Apartment") },
    { "arts_centre", kli18nc("OSM::amenity/shop", "Arts Center") },
    { "artwork", kli18nc("OSM::amenity/shop", "Artwork") },
    { "atm", kli18nc("OSM::amenity/shop", "ATM") },
    { "attraction", kli18nc("OSM::amenity/shop", "Attraction") },
    { "bag", kli18nc("OSM::amenity/shop", "Bag") },
    { "bakery", kli18nc("OSM::amenity/shop", "Bakery") },
    { "bank", kli18nc("OSM::amenity/shop", "Bank") },
    { "bar", kli18nc("OSM::amenity/shop", "Bar") },
    { "beauty", kli18nc("OSM::amenity/shop", "Beauty") },
    { "bed", kli18nc("OSM::amenity/shop", "Bed") },
    { "bench", kli18nc("OSM::amenity", "Bench") },
    { "beverages", kli18nc("OSM::amenity/shop", "Beverages") },
    { "bicycle", kli18nc("OSM::amenity/shop", "Bicycle") },
    { "bicycle_parking", kli18nc("OSM::amenity/shop", "Bicycle Parking") },
    { "bicycle_rental", kli18nc("OSM::amenity/shop", "Bicycle Rental") },
    { "books", kli18nc("OSM::amenity/shop", "Books") },
    { "boutique", kli18nc("OSM::amenity/shop", "Boutique") },
    { "bureau_de_change", kli18nc("OSM::amenity/shop", "Bureau de Change") },
    { "butcher", kli18nc("OSM::amenity/shop", "Butcher") },
    { "cafe", kli18nc("OSM::amenity/shop", "Cafe") },
    { "car", kli18nc("OSM::amenity/shop", "Car") },
    { "car_rental", kli18nc("OSM::amenity/shop", "Car Rental") },
    { "car_sharing", kli18nc("OSM::amenity/shop", "Car Sharing") },
    { "charging_station", kli18nc("OSM::amenity/shop", "Charging Station") },
    { "chemist", kli18nc("OSM::amenity/shop", "Chemist") },
    { "chocolate", kli18nc("OSM::amenity/shop", "Chocolate") },
    { "cinema", kli18nc("OSM::amenity/shop", "Cinema") },
    { "citywalls", kli18nc("OSM::historic", "Citywall") },
    { "clothes", kli18nc("OSM::amenity/shop", "Clothes") },
    { "coffee", kli18nc("OSM::amenity/shop", "Coffee") },
    { "computer", kli18nc("OSM::amenity/shop", "Computer") },
    { "confectionery", kli18nc("OSM::amenity/shop", "Confectionery") },
    { "convenience", kli18nc("OSM::amenity/shop", "Convenience Store") },
    { "copyshop", kli18nc("OSM::amenity/shop", "Copy Shop") },
    { "cosmetics", kli18nc("OSM::amenity/shop", "Cosmetics") },
    { "courthouse", kli18nc("OSM::amenity/shop", "Court House") },
    { "deli", kli18nc("OSM::amenity/shop", "Deli") },
    { "department_store", kli18nc("OSM::amenity/shop", "Department Store") },
    { "doctors", kli18nc("OSM::amenity/shop", "Doctor") },
    { "doityourself", kli18nc("OSM::amenity/shop", "Hardware Store") },
    { "drinking_water", kli18nc("OSM::amenity/shop", "Drinking Water") },
    { "dry_cleaning", kli18nc("OSM::amenity/shop", "Dry Cleaning") },
    { "electronics", kli18nc("OSM::amenity/shop", "Electronics") },
    { "fashion", kli18nc("OSM::amenity/shop", "Fashion") },
    { "fast_food", kli18nc("OSM::amenity/shop", "Fast Food") },
    { "ferry_terminal", kli18nc("OSM::amenity/shop", "Ferry Terminal") },
    { "florist", kli18nc("OSM::amenity/shop", "Florist") },
    { "food_court", kli18nc("OSM::amenity/shop", "Food Court") },
    { "fountain", kli18nc("OSM::amenity/shop", "Fountain") },
    { "furniture", kli18nc("OSM::amenity/shop", "Furniture") },
    { "gallery", kli18nc("OSM::amenity/shop", "Gallery") },
    { "garden", kli18nc("OSM::amenity/shop", "Garden") },
    { "garden_centre", kli18nc("OSM::amenity/shop", "Garden Center") },
    { "gift", kli18nc("OSM::amenity/shop", "Gift Shop") },
    { "government", kli18nc("OSM::office", "Government") },
    { "greengrocer", kli18nc("OSM::amenity/shop", "Greengrocer") },
    { "guest_house", kli18nc("OSM::amenity/shop", "Guest House") },
    { "hairdresser", kli18nc("OSM::amenity/shop", "Hairdresser") },
    { "hearing_aids", kli18nc("OSM::amenity/shop", "Hearing Aids") },
    { "hospital", kli18nc("OSM::amenity/shop", "Hospital") },
    { "hostel", kli18nc("OSM::amenity/shop", "Hostel") },
    { "hotel", kli18nc("OSM::amenity/shop", "Hotel") },
    { "houseware", kli18nc("OSM::amenity/shop", "Houseware") },
    { "ice_cream", kli18nc("OSM::amenity/shop", "Ice Cream") },
    { "information", kli18nc("OSM::amenity/shop", "Information") },
    { "interior_decoration", kli18nc("OSM::amenity/shop", "Interior Decoration") },
    { "internet_cafe", kli18nc("OSM::amenity/shop", "Internet Cafe") },
    { "jewelry", kli18nc("OSM::amenity/shop", "Jewelry") },
    { "kiosk", kli18nc("OSM::amenity/shop", "Kiosk") },
    { "laundry", kli18nc("OSM::amenity/shop", "Laundry") },
    { "library", kli18nc("OSM::amenity/shop", "Library") },
    { "lockers", kli18nc("OSM::amenity/shop", "Locker") },
    { "locksmith", kli18nc("OSM::amenity/shop", "Locksmith") },
    { "lost_and_found", kli18nc("OSM::amenity/shop", "Lost & Found") },
    { "lost_property", kli18nc("OSM::amenity/shop", "Lost & Found") },
    { "luggage_locker", kli18nc("OSM::amenity/shop", "Locker") },
    { "mall", kli18nc("OSM::amenity/shop", "Mall") },
    { "marketplace", kli18nc("OSM::amenity", "Marketplace") },
    { "medical_supply", kli18nc("OSM::amenity/shop", "Medical Supply") },
    { "memorial", kli18nc("OSM::historic", "Memorial") },
    { "mobile_phone", kli18nc("OSM::amenity/shop", "Mobile Phone") },
    { "money_transfer", kli18nc("OSM::amenity/shop", "Money Transfer") },
    { "monument", kli18nc("OSM::historic", "Monument") },
    { "motorcycle_parking", kli18nc("OSM::amenity/shop", "Motorcycle Parking") },
    { "motorcycle_rental", kli18nc("OSM::amenity/shop", "Motorcycle Rental") },
    { "museum", kli18nc("OSM::amenity/shop", "Museum") },
    { "music", kli18nc("OSM::amenity/shop", "Music") },
    { "musical_instrument", kli18nc("OSM::amenity/shop", "Musical Instruments") },
    { "newsagent", kli18nc("OSM::amenity/shop", "Newsagent") },
    { "office", kli18nc("OSM::amenity/shop", "Office") },
    { "optician", kli18nc("OSM::amenity/shop", "Optician") },
    { "outdoor", kli18nc("OSM::amenity/shop", "Outdoor") },
    { "paint", kli18nc("OSM::amenity/shop", "Paint") },
    { "park", kli18nc("outdoor recreational area", "Park") },
    { "parking", kli18nc("OSM::amenity/shop", "Parking") },
    { "parking_tickets", kli18nc("OSM::amenity/shop", "Parking Tickets") },
    { "pastry", kli18nc("OSM::amenity/shop", "Pastry") },
    { "perfumery", kli18nc("OSM::amenity/shop", "Perfumery") },
    { "pet", kli18nc("OSM::amenity/shop", "Pet") },
    { "pharmacy", kli18nc("OSM::amenity/shop", "Pharmacy") },
    { "photo", kli18nc("OSM::amenity/shop", "Photo") },
    { "place_of_worship", kli18nc("OSM::amenity/shop", "Place of Worship") },
    { "playground", kli18n("Playground") },
    { "police", kli18nc("OSM::amenity/shop", "Police") },
    { "post_box", kli18nc("OSM::amenity/shop", "Post Box") },
    { "post_office", kli18nc("OSM::amenity/shop", "Post Office") },
    { "pub", kli18nc("OSM::amenity/shop", "Pub") },
    { "public_transport_tickets", kli18nc("OSM::amenity/shop", "Public Transport Tickets") },
    { "recycling", kli18nc("OSM::amenity/shop", "Recycling") },
    { "restaurant", kli18nc("OSM::amenity/shop", "Restaurant") },
    { "school", kli18nc("OSM::amenity/shop", "School") },
    { "scooter_rental", kli18nc("OSM::amenity/shop", "Kick Scooter Rental") },
    { "seafood", kli18nc("OSM::amenity/shop", "Seafood") },
    { "shoes", kli18nc("OSM::amenity/shop", "Shoes") },
    { "shop", kli18nc("OSM::amenity/shop", "Shop") },
    { "social_facility", kli18nc("OSM::amenity/shop", "Social Facility") },
    { "souveniers", kli18nc("OSM::amenity/shop", "Souvenirs") },
    { "sports", kli18nc("OSM::amenity/shop", "Sports") },
    { "stationery", kli18nc("OSM::amenity/shop", "Stationery") },
    { "supermarket", kli18nc("OSM::amenity/shop", "Supermarket") },
    { "tailor", kli18nc("OSM::amenity/shop", "Tailor") },
    { "tatoo", kli18nc("OSM::amenity/shop", "Tattoo") },
    { "taxi", kli18nc("OSM::amenity/shop", "Taxi") },
    { "tea", kli18nc("OSM::amenity/shop", "Tea") },
    { "theatre", kli18nc("OSM::amenity/shop", "Theatre") },
    { "ticket", kli18nc("OSM::amenity/shop", "Tickets") },
    { "tobacco", kli18nc("OSM::amenity/shop", "Tobacco") },
    { "toilets", kli18nc("OSM::amenity/shop", "Toilets") },
    { "townhall", kli18nc("OSM::amenity/shop", "Town Hall") },
    { "toys", kli18nc("OSM::amenity/shop", "Toys") },
    { "travel_agency", kli18nc("OSM::amenity/shop", "Travel Agency") },
    { "travel_agent", kli18nc("OSM::amenity/shop", "Travel Agency") },
    { "university", kli18nc("OSM::amenity/shop", "University") },
    { "variety_store", kli18nc("OSM::amenity/shop", "Variety Store") },
    { "video_games", kli18nc("OSM::amenity/shop", "Video Games") },
    { "waiting", kli18nc("OSM::amenity/shop", "Waiting Area") },
    { "waiting_area", kli18nc("OSM::amenity/shop", "Waiting Area") },
    { "waiting_room", kli18nc("OSM::amenity/shop", "Waiting Area") },
    { "waste_basket", kli18nc("OSM::amenity", "Waste Basket") },
    { "wine", kli18nc("OSM::amenity/shop", "Wine") },
};
static_assert(isSortedLookupTable(amenity_map), "amenity map is not sorted!");

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

}
