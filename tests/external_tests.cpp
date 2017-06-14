#include "catch/include/catch.hpp"

#include <sstream>
#include <iostream>
#include <unordered_set>
#include <utility>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "../src/moment.hpp"
#include "../src/roundinfo.hpp"
#include "../src/roundinfoview.hpp"
#include "../src/itemsutil.hpp"
#include "../src/aliases.hpp"
#include "../src/momentoverview.hpp"
#include "../src/movedata.hpp"

using namespace timeplane;
using namespace roundinfo;
using namespace item;
using namespace external;

extern RoundInfo MakeRoundInfo();

TEST_CASE("MomentOverview overall", "[momentoverview, external_all]") {
    RoundInfo info = MakeRoundInfo();
    RoundInfoView viewer{info, 2};
    Moment m{0, 0};

    ItemPtr antitelephone = std::make_unique<Antitelephone>(m);
    ItemPtr bridge = std::make_unique<Bridge>(m);
    ItemPtr oracle = std::make_unique<Oracle>(m);
    ItemPtr shield = std::make_unique<Shield>(m);
    Effect e = antitelephone->View(m) + bridge->View(m) +
               oracle->View(m) + shield->View(m);

    std::vector<TaggedValues> states{};
    states.push_back(antitelephone->StateTaggedValues(m));
    states.push_back(bridge->StateTaggedValues(m));
    states.push_back(oracle->StateTaggedValues(m));
    states.push_back(shield->StateTaggedValues(m));

    MomentOverview overview{};

    auto continuation = [&] {
        REQUIRE(overview.player() == 2);
        REQUIRE(overview.moment() == m);
        Effect e2 = overview.effect();
        REQUIRE(e.attack_increase() == e2.attack_increase());
        REQUIRE(e.max_hitpoint_increase() == e2.max_hitpoint_increase());
        REQUIRE(e.shield_amount() == e2.shield_amount());
        REQUIRE(e.antitelephone_departure() == e2.antitelephone_departure());
        REQUIRE(e.antitelephone_dest_allowed() ==
                e2.antitelephone_dest_allowed());
        REQUIRE(e.player_make_active() == e2.player_make_active());
        REQUIRE(overview.ItemState(ItemType::kAntitelephone) ==
                antitelephone->StateTaggedValues(m));
        REQUIRE(overview.ItemState(ItemType::kBridge) ==
                bridge->StateTaggedValues(m));
        REQUIRE(overview.ItemState(ItemType::kOracle) ==
                oracle->StateTaggedValues(m));
        REQUIRE(overview.ItemState(ItemType::kShield) ==
                shield->StateTaggedValues(m));
        RoundInfoView const& viewer2 = overview.round_info();
        REQUIRE(viewer.active() == viewer2.active());
        REQUIRE(viewer.player() == viewer2.player());
        for (int i = 0; i < 5; i++) {
            REQUIRE(viewer.Location(i) == viewer2.Location(i));
            REQUIRE(viewer.DamageReceived(i) == viewer2.DamageReceived(i));
            REQUIRE(viewer.HealthRemaining(i) == viewer2.HealthRemaining(i));
        }
    };

    SECTION("Regular construction") {
        overview = MomentOverview{m, e, std::move(states), viewer};
        continuation();
    }

    SECTION("Serialization and deserialization") {
        MomentOverview temp{m, e, std::move(states), viewer};
        std::stringstream stream{};
        boost::archive::text_oarchive output_archive{stream};
        output_archive << temp;

        boost::archive::text_iarchive input_archive{stream};
        input_archive >> overview;
        continuation();
    }
}

TEST_CASE("MoveData overall", "[movedata, external_all]") {
    MoveData data{};

    REQUIRE(data.EnergyInput(ItemType::kAntitelephone) == 0);
    REQUIRE(data.EnergyInput(ItemType::kBridge) == 0);
    REQUIRE(data.EnergyInput(ItemType::kOracle) == 0);
    REQUIRE(data.EnergyInput(ItemType::kShield) == 0);
    std::unordered_set<int> added = data.added_alliances();
    REQUIRE(!added.count(0));
    REQUIRE(!added.count(1));
    REQUIRE(!added.count(2));
    REQUIRE(!added.count(3));
    std::unordered_set<int> removed = data.removed_alliances();
    REQUIRE(!removed.count(0));
    REQUIRE(!removed.count(1));
    REQUIRE(!removed.count(2));
    REQUIRE(!removed.count(3));

    auto continuation = [&] {
        REQUIRE(data.new_location() == 4);
        REQUIRE(data.EnergyInput(ItemType::kAntitelephone) == 1);
        REQUIRE(data.EnergyInput(ItemType::kBridge) == 0);
        REQUIRE(data.EnergyInput(ItemType::kOracle) == 2);
        REQUIRE(data.EnergyInput(ItemType::kShield) == 0);
        std::unordered_set<int> added = data.added_alliances();
        REQUIRE(!added.count(0));
        REQUIRE(!added.count(1));
        REQUIRE( added.count(2));
        REQUIRE(!added.count(3));
        std::unordered_set<int> removed = data.removed_alliances();
        REQUIRE(!removed.count(0));
        REQUIRE(!removed.count(1));
        REQUIRE(!removed.count(2));
        REQUIRE( removed.count(3));
    };

    SECTION("Direct setup") {
        data.set_new_location(4);
        data.SetEnergyInput(ItemType::kAntitelephone, 1);
        data.SetEnergyInput(ItemType::kOracle, 2);
        data.add_alliance(2);
        data.remove_alliance(3);

        SECTION("No serialization") {
            continuation();
        }

        SECTION("Serialization and deserialization") {
            MoveData temp{std::move(data)};
            data = MoveData{};
            std::stringstream stream{};
            boost::archive::text_oarchive output_archive{stream};
            output_archive << temp;

            boost::archive::text_iarchive input_archive{stream};
            input_archive >> data;
            continuation();
        }
    }

    SECTION("Changing minds") {
        data.set_new_location(2);
        data.set_new_location(4);
        data.SetEnergyInput(ItemType::kAntitelephone, 2);
        data.SetEnergyInput(ItemType::kBridge, 1);
        data.SetEnergyInput(ItemType::kAntitelephone, 1);
        data.SetEnergyInput(ItemType::kBridge, 0);
        data.SetEnergyInput(ItemType::kOracle, 2);
        data.add_alliance(1);
        data.add_alliance(2);
        data.add_alliance(3);
        data.remove_alliance(1);
        data.remove_alliance(3);
        data.leave_unchanged_alliance(1);

        SECTION("No serialization") {
            continuation();
        }

        SECTION("Serialization and deserialization") {
            MoveData temp{std::move(data)};
            data = MoveData{};
            std::stringstream stream{};
            boost::archive::text_oarchive output_archive{stream};
            output_archive << temp;

            boost::archive::text_iarchive input_archive{stream};
            input_archive >> data;
            continuation();
        }
    }
}
