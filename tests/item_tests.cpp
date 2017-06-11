#include <catch/include/catch.hpp>

#include <sstream>
#include <iostream>
#include <utility>
#include <unordered_set>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "../src/itemproperties.hpp"
#include "../src/effect.hpp"

using namespace item;

TEST_CASE("ItemProperties overall", "[itemproperties, item_all]") {
    ItemProperties ip;
    ip.set_lockdown(10);
    REQUIRE(ip.lockdown() == 10);

    ip.set_cooldown(9);
    REQUIRE(ip.cooldown() == 9);

    int prop0_id = 0, prop1_id = 1;
    ip.set_custom(prop0_id, 8);
    ip.set_custom(prop1_id, 7);

    REQUIRE(ip.custom(prop0_id) == 8);
    REQUIRE(ip.custom(prop1_id) == 7);
    REQUIRE_THROWS_AS(ip.custom(2), std::out_of_range);

    ItemProperties copy = ip;
    ItemProperties move = std::move(ip);

    REQUIRE(copy.lockdown() == 10);
    REQUIRE(move.lockdown() == 10);
    REQUIRE(copy.cooldown() == 9);
    REQUIRE(move.cooldown() == 9);
    REQUIRE(copy.custom(prop0_id) == 8);
    REQUIRE(copy.custom(prop1_id) == 7);
    REQUIRE(move.custom(prop0_id) == 8);
    REQUIRE(move.custom(prop1_id) == 7);
}

TEST_CASE("Effect overall", "[effect, item_all]") {
    Effect empty{};

    REQUIRE(empty.attack_boost() == 0);
    REQUIRE(empty.shield_amount() == 0);
    REQUIRE(!empty.antitelephone_dest_allowed());
    REQUIRE(!empty.player_make_active());

    Effect full{2, 3, true, true};
    REQUIRE(full.attack_boost() == 2);
    REQUIRE(full.shield_amount() == 3);
    REQUIRE(full.antitelephone_dest_allowed());
    REQUIRE(full.player_make_active());

    Effect full2 = empty + full;
    REQUIRE(full2.attack_boost() == 2);
    REQUIRE(full2.shield_amount() == 3);
    REQUIRE(full2.antitelephone_dest_allowed());
    REQUIRE(full2.player_make_active());

    Effect e1{4, 0, false, true};
    Effect e2{1, 2, false, true};
    Effect e3{0, 1, true, false};

    Effect temp = e1 + e2;
    REQUIRE(temp.attack_boost() == 5);
    REQUIRE(temp.shield_amount() == 2);
    REQUIRE(!temp.antitelephone_dest_allowed());
    REQUIRE(temp.player_make_active());

    temp = e2 + e3;
    REQUIRE(temp.attack_boost() == 1);
    REQUIRE(temp.shield_amount() == 3);
    REQUIRE(temp.antitelephone_dest_allowed());
    REQUIRE(temp.player_make_active());

    temp = e1 + e3;
    REQUIRE(temp.attack_boost() == 4);
    REQUIRE(temp.shield_amount() == 1);
    REQUIRE(temp.antitelephone_dest_allowed());
    REQUIRE(temp.player_make_active());
}
