#include <catch/include/catch.hpp>

#include <sstream>
#include <iostream>
#include <utility>
#include <unordered_set>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "../src/aliases.hpp"
#include "../src/moment.hpp"
#include "../src/timeline.hpp"
#include "../src/timeplane.hpp"
#include "../src/roundinfo.hpp"
#include "../src/roundinfoview.hpp"
#include "../src/itemsutil.hpp"

using namespace item;
using namespace timeplane;
using namespace roundinfo;

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

    REQUIRE(empty.attack_increase() == 0);
    REQUIRE(empty.max_hitpoint_increase() == 0);
    REQUIRE(empty.shield_amount() == 0);
    REQUIRE(!empty.antitelephone_departure());
    REQUIRE(!empty.antitelephone_dest_allowed());
    REQUIRE(!empty.player_make_active());

    Effect full{2, 4, 3, true, true, true};
    REQUIRE(full.attack_increase() == 2);
    REQUIRE(full.max_hitpoint_increase() == 4);
    REQUIRE(full.shield_amount() == 3);
    REQUIRE(full.antitelephone_departure());
    REQUIRE(full.antitelephone_dest_allowed());
    REQUIRE(full.player_make_active());

    Effect full2 = empty + full;
    REQUIRE(full2.attack_increase() == 2);
    REQUIRE(full2.max_hitpoint_increase() == 4);
    REQUIRE(full2.shield_amount() == 3);
    REQUIRE(full2.antitelephone_departure());
    REQUIRE(full2.antitelephone_dest_allowed());
    REQUIRE(full2.player_make_active());

    empty += full;
    REQUIRE(empty.attack_increase() == 2);
    REQUIRE(empty.max_hitpoint_increase() == 4);
    REQUIRE(empty.shield_amount() == 3);
    REQUIRE(empty.antitelephone_departure());
    REQUIRE(empty.antitelephone_dest_allowed());
    REQUIRE(empty.player_make_active());

    Effect e1{4, 0, 0, false, false, true};
    Effect e2{};
    e2.set_attack_increase(1);
    e2.set_max_hitpoint_increase(2);
    e2.set_shield_amount(2);
    e2.set_antitelephone_departure(true);
    e2.set_antitelephone_dest_allowed(false);
    e2.set_player_make_active(true);
    Effect e3_t{0, 1, 1, false, true, false};
    Effect e3;
    std::stringstream stream{};
    boost::archive::text_oarchive output_archive{stream};
    output_archive << e3_t;

    boost::archive::text_iarchive input_archive{stream};
    input_archive >> e3;

    Effect temp = e1 + e2;
    REQUIRE(temp.attack_increase() == 5);
    REQUIRE(temp.max_hitpoint_increase() == 2);
    REQUIRE(temp.shield_amount() == 2);
    REQUIRE(temp.antitelephone_departure());
    REQUIRE(!temp.antitelephone_dest_allowed());
    REQUIRE(temp.player_make_active());

    e2 += e3;
    REQUIRE(e2.attack_increase() == 1);
    REQUIRE(e2.max_hitpoint_increase() == 3);
    REQUIRE(e2.shield_amount() == 3);
    REQUIRE(e2.antitelephone_departure());
    REQUIRE(e2.antitelephone_dest_allowed());
    REQUIRE(e2.player_make_active());

    temp = e1 + e3;
    REQUIRE(temp.attack_increase() == 4);
    REQUIRE(temp.max_hitpoint_increase() == 1);
    REQUIRE(temp.shield_amount() == 1);
    REQUIRE(!temp.antitelephone_departure());
    REQUIRE(temp.antitelephone_dest_allowed());
    REQUIRE(temp.player_make_active());
}

extern RoundInfo MakeRoundInfo();

void PrintTags(ItemPtr const& ptr, Moment m) {
    for (TaggedValue const& thing : ptr->StateTaggedValues(m)) {
        std::cout << thing.first << " " << thing.second << std::endl;
    }
    std::cout << std::endl;
}

TEST_CASE("Antitelephone item tests", "[antitelephone, item_all]") {
    TimePlane tp{};
    TimeLine* tl = &tp.rightmost_timeline();
    Moment m = tl->LatestMoment();
    Moment mn;
    ItemPtr antitelephone = std::make_unique<Antitelephone>(m);
    RoundInfo info{MakeRoundInfo()};
    RoundInfoView viewer{info, 0};

    Effect e = antitelephone->View(m);
    REQUIRE(e.attack_increase() == Item::kBasicAttack);
    REQUIRE(e.max_hitpoint_increase() == Item::kBasicMaxHitpoints);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    // Activation energy 5, time 0
    mn = tl->MakeMoment();
    e = antitelephone->Step(m, viewer, 0);
    antitelephone->ConfirmPending(mn);
    REQUIRE(e.attack_increase() == Item::kBasicAttack);
    REQUIRE(e.max_hitpoint_increase() == Item::kBasicMaxHitpoints);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    // Activation energy 5, time 1
    m = mn;
    mn = tl->MakeMoment();
    e = antitelephone->Step(m, viewer, 3);
    antitelephone->ConfirmPending(mn);
    REQUIRE(e.attack_increase() == Item::kBasicAttack);
    REQUIRE(e.max_hitpoint_increase() == Item::kBasicMaxHitpoints);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    // Activation energy 3, time 2
    m = mn;
    mn = tl->MakeMoment();
    e = antitelephone->Step(m, viewer, 3);
    antitelephone->ConfirmPending(mn);

    // Activation energy 1, time 3
    m = mn;
    mn = tl->MakeMoment();
    e = antitelephone->Step(m, viewer, 0);
    antitelephone->ConfirmPending(mn);
    REQUIRE(e.attack_increase() == Item::kBasicAttack);
    REQUIRE(e.max_hitpoint_increase() == Item::kBasicMaxHitpoints);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    // Activation energy 2, time 4
    m = mn;
    // The antitelephone activates
    e = antitelephone->Step(m, viewer, 3);
    REQUIRE(e.attack_increase() == Item::kBasicAttack);
    REQUIRE(e.max_hitpoint_increase() == Item::kBasicMaxHitpoints);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(e.antitelephone_departure()); // !!!
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    e = antitelephone->View(tl->GetMoment(2));
    REQUIRE(e.attack_increase() == Item::kBasicAttack);
    REQUIRE(e.max_hitpoint_increase() == Item::kBasicMaxHitpoints);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    REQUIRE(m.time() == 4);
    e = antitelephone->Branch(m, tl->GetMoment(1));
    REQUIRE(e.attack_increase() == Item::kBasicAttack);
    REQUIRE(e.max_hitpoint_increase() == Item::kBasicMaxHitpoints);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    tl = &tp.MakeNewTimeLine(1);
    mn = tl->LatestMoment();
    antitelephone->ConfirmPending(mn);

    // Debt 3, activation energy 5, time 1
    m = mn;
    mn = tl->MakeMoment();
    e = antitelephone->Step(m, viewer, 2);
    REQUIRE(!e.antitelephone_departure());
    antitelephone->ConfirmPending(mn);

    // Debt 1, activation energy 5, time 2
    m = mn;
    mn = tl->MakeMoment();
    e = antitelephone->Step(m, viewer, 3);
    REQUIRE(!e.antitelephone_departure());
    antitelephone->ConfirmPending(mn);

    // Activation energy 4, time 3
    m = mn;
    mn = tl->MakeMoment();
    e = antitelephone->Step(m, viewer, 3);
    REQUIRE(!e.antitelephone_departure());
    antitelephone->ConfirmPending(mn);

    // Activation energy 2, time 4
    m = mn;
    mn = tl->MakeMoment();
    e = antitelephone->Step(m, viewer, 3);
    REQUIRE(e.antitelephone_departure()); // !!!
    // Let's not confirm that step
    e = antitelephone->Step(m, viewer, 2);
    REQUIRE(!e.antitelephone_departure());
    antitelephone->ConfirmPending(mn);

    // Activation energy 1, time 5
    m = mn;
    // The antitelephone activates
    e = antitelephone->Step(m, viewer, 2);
    REQUIRE(e.antitelephone_departure()); // !!!

    REQUIRE(m.time() == 5);
    e = antitelephone->Branch(m, tl->GetMoment(2));
    REQUIRE(!e.antitelephone_departure());

    tl = &tp.MakeNewTimeLine(2);
    mn = tl->LatestMoment();
    antitelephone->ConfirmPending(mn);

    // Debt 4, activation energy 5, time 2
    m = mn;
    mn = tl->MakeMoment();
    e = antitelephone->Step(m, viewer, 3);
    REQUIRE(!e.antitelephone_departure());
    antitelephone->ConfirmPending(mn);

    // Debt 1, activation energy 5, time 3
    m = mn;
    mn = tl->MakeMoment();
    e = antitelephone->Step(m, viewer, 3);
    REQUIRE(!e.antitelephone_departure());
    antitelephone->ConfirmPending(mn);

    // Activation energy 4, time 4
    m = mn;
    mn = tl->MakeMoment();
    e = antitelephone->Step(m, viewer, 2); // !!!
    REQUIRE(!e.antitelephone_departure());
    antitelephone->ConfirmPending(mn);

    // Activation energy 3, time 5
    m = mn;
    mn = tl->MakeMoment();
    e = antitelephone->Step(m, viewer, 3);
    REQUIRE(!e.antitelephone_departure());
    antitelephone->ConfirmPending(mn);

    // Activation energy 1, time 6
    m = mn;
    // The antitelephone activates for input 2 or 3
    e = antitelephone->Step(m, viewer, 1);
    REQUIRE(!e.antitelephone_departure());
    // Let's not confirm that step
    e = antitelephone->Step(m, viewer, 2);
    REQUIRE(e.antitelephone_departure()); // !!!
    // Let's not confirm that step
    e = antitelephone->Step(m, viewer, 3);
    REQUIRE(e.antitelephone_departure()); // !!!
    // Let's not confirm that step

    // Player dies without being able to use the Antitelephone.
    info.HealthRemainingIterator()[0] = 0;
    RoundInfoView viewer2{info, 0};
    e = antitelephone->Step(m, viewer2, 3);
    REQUIRE(!e.antitelephone_departure());
    // Let's not confirm that step
}

TEST_CASE("Bridge item tests", "[bridge, item_all]") {
    TimePlane tp{};
    TimeLine* tl = &tp.rightmost_timeline();
    Moment m = tl->LatestMoment();
    Moment mn;
    ItemPtr bridge = std::make_unique<Bridge>(m);
    RoundInfo info = MakeRoundInfo();
    RoundInfoView viewer{info, 0};

    Effect e = bridge->View(m);
    REQUIRE(e.attack_increase() == 0);
    REQUIRE(e.max_hitpoint_increase() == 0);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    // Time 0
    // Take big steps towards unlocking it
    int half = Bridge::kUnlockRequirement / 2;
    mn = tl->MakeMoment();
    e = bridge->Step(m, viewer, half);
    bridge->ConfirmPending(mn);
    REQUIRE(e.attack_increase() == 0);
    REQUIRE(e.max_hitpoint_increase() == 0);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    // Time 1
    // Player dies without being able to use the Bridge.
    m = mn;
    mn = tl->MakeMoment();
    info.HealthRemainingIterator()[0] = 0;
    RoundInfoView viewer2{info, 0};
    e = bridge->Step(m, viewer2, 3);
    REQUIRE(e.attack_increase() == 0);
    REQUIRE(e.max_hitpoint_increase() == 0);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());
    // Let's not confirm that step

    e = bridge->Step(m, viewer, Bridge::kUnlockRequirement - half);
    bridge->ConfirmPending(mn);
    // Now it should be unlocked
    REQUIRE(e.attack_increase() == Bridge::kAttackIncrement);
    REQUIRE(e.max_hitpoint_increase() == Bridge::kMaxHitpointIncrement);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    e = bridge->View(m);
    REQUIRE(e.attack_increase() == 0);
    REQUIRE(e.max_hitpoint_increase() == 0);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    e = bridge->View(mn);
    REQUIRE(e.attack_increase() == Bridge::kAttackIncrement);
    REQUIRE(e.max_hitpoint_increase() == Bridge::kMaxHitpointIncrement);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    // Activation energy 5, time 2
    REQUIRE(mn.time() == 2);
    m = mn;
    mn = tl->MakeMoment();
    e = bridge->Step(m, viewer, 3);
    bridge->ConfirmPending(mn);
    REQUIRE(e.attack_increase() == Bridge::kAttackIncrement);
    REQUIRE(e.max_hitpoint_increase() == Bridge::kMaxHitpointIncrement);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    // Activation energy 3, time 3
    m = mn;
    mn = tl->MakeMoment();
    e = bridge->Step(m, viewer, 3);
    bridge->ConfirmPending(mn);
    REQUIRE(!e.antitelephone_dest_allowed());

    // Activation energy 1, time 4
    m = mn;
    mn = tl->MakeMoment();
    e = bridge->Step(m, viewer, 2);
    bridge->ConfirmPending(mn);
    // Now the bridge is activated
    REQUIRE(!e.antitelephone_dest_allowed());

    // Charge remaining 4, activated since 5, time 5
    m = mn;
    mn = tl->MakeMoment();
    e = bridge->Step(m, viewer, 1);
    bridge->ConfirmPending(mn);
    REQUIRE(e.attack_increase() == Bridge::kAttackIncrement);
    REQUIRE(e.max_hitpoint_increase() == Bridge::kMaxHitpointIncrement);
    REQUIRE(!e.antitelephone_dest_allowed());

    // Charge remaining 4, activated since 5, time 6
    m = mn;
    mn = tl->MakeMoment();
    e = bridge->Step(m, viewer, 0);
    bridge->ConfirmPending(mn);
    REQUIRE(!e.antitelephone_dest_allowed());

    // Charge remaining 3, activated since 5, time 7
    m = mn;
    mn = tl->MakeMoment();
    e = bridge->Step(m, viewer, 0);
    bridge->ConfirmPending(mn);
    REQUIRE(!e.antitelephone_dest_allowed());

    // Charge remaining 2, activated since 5, time 8
    m = mn;
    mn = tl->MakeMoment();
    e = bridge->Step(m, viewer, 0);
    bridge->ConfirmPending(mn);
    REQUIRE(!e.antitelephone_dest_allowed());

    // Charge remaining 1, activated since 5, time 9
    REQUIRE(mn.time() == 9);
    m = mn;
    // We will make a branch somewhere below
    e = bridge->Branch(m, tl->GetMoment(1));
    REQUIRE(e.attack_increase() == 0);
    REQUIRE(e.max_hitpoint_increase() == 0);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());
    // Let's decide not to branch

    e = bridge->Branch(m, tl->GetMoment(2));
    REQUIRE(e.attack_increase() == Bridge::kAttackIncrement);
    REQUIRE(e.max_hitpoint_increase() == Bridge::kMaxHitpointIncrement);
    REQUIRE(!e.antitelephone_dest_allowed());
    // Let's decide not to branch

    e = bridge->Branch(m, tl->GetMoment(4));
    REQUIRE(!e.antitelephone_dest_allowed());
    // Let's decide not to branch

    e = bridge->Branch(m, tl->GetMoment(5));
    REQUIRE(e.antitelephone_dest_allowed()); // !!!
    // Let's decide not to branch

    e = bridge->Branch(m, tl->GetMoment(8));
    REQUIRE(e.antitelephone_dest_allowed()); // !!!

    tl = &tp.MakeNewTimeLine(8);
    mn = tl->LatestMoment();
    bridge->ConfirmPending(mn);

    // Charge remaining 2, activated since 8, time 8
    m = mn;
    mn = tl->MakeMoment();
    e = bridge->Step(m, viewer, 0);
    bridge->ConfirmPending(mn);
    REQUIRE(!e.antitelephone_dest_allowed());

    // Charge remaining 1, activated since 8, time 9
    m = mn;
    // We won't branch, just testing things
    e = bridge->Branch(m, tl->GetMoment(4));
    REQUIRE(!e.antitelephone_dest_allowed());
    e = bridge->Branch(m, tl->GetMoment(5));
    REQUIRE(!e.antitelephone_dest_allowed());
    e = bridge->Branch(m, tl->GetMoment(7));
    REQUIRE(!e.antitelephone_dest_allowed());
    e = bridge->Branch(m, tl->GetMoment(8));
    REQUIRE(e.antitelephone_dest_allowed()); // !!!
    mn = tl->MakeMoment();
    e = bridge->Step(m, viewer, 0);
    bridge->ConfirmPending(mn);
    REQUIRE(!e.antitelephone_dest_allowed());

    // Activation energy 5, time 10
    REQUIRE(mn.time() == 10);
    m = mn;
    e = bridge->Branch(m, tl->GetMoment(7));
    REQUIRE(!e.antitelephone_dest_allowed());
    e = bridge->Branch(m, tl->GetMoment(8));
    REQUIRE(!e.antitelephone_dest_allowed());
}

TEST_CASE("Oracle item tests", "[oracle, item_all]") {
    TimePlane tp{};
    TimeLine* tl = &tp.rightmost_timeline();
    Moment m = tl->LatestMoment();
    Moment mn;
    ItemPtr oracle = std::make_unique<Oracle>(m);
    RoundInfo info = MakeRoundInfo();
    RoundInfoView viewer{info, 0};

    Effect e = oracle->View(m);
    REQUIRE(e.attack_increase() == 0);
    REQUIRE(e.max_hitpoint_increase() == 0);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    // Time 0
    // Take big steps towards unlocking it
    int half = Oracle::kUnlockRequirement / 2;
    mn = tl->MakeMoment();
    e = oracle->Step(m, viewer, half);
    oracle->ConfirmPending(mn);
    REQUIRE(e.attack_increase() == 0);
    REQUIRE(e.max_hitpoint_increase() == 0);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    // Time 1
    m = mn;
    mn = tl->MakeMoment();
    e = oracle->Step(m, viewer, Oracle::kUnlockRequirement - half);
    oracle->ConfirmPending(mn);
    // Now it should be unlocked
    REQUIRE(e.attack_increase() == Oracle::kAttackIncrement);
    REQUIRE(e.max_hitpoint_increase() == Oracle::kMaxHitpointIncrement);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    e = oracle->View(m);
    REQUIRE(e.attack_increase() == 0);
    REQUIRE(e.max_hitpoint_increase() == 0);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    e = oracle->View(mn);
    REQUIRE(e.attack_increase() == Oracle::kAttackIncrement);
    REQUIRE(e.max_hitpoint_increase() == Oracle::kMaxHitpointIncrement);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    // Activation energy 5, time 2
    REQUIRE(mn.time() == 2);
    m = mn;
    mn = tl->MakeMoment();
    e = oracle->Step(m, viewer, 3);
    oracle->ConfirmPending(mn);
    REQUIRE(e.attack_increase() == Oracle::kAttackIncrement);
    REQUIRE(e.max_hitpoint_increase() == Oracle::kMaxHitpointIncrement);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    // Activation energy 3, time 3
    m = mn;
    mn = tl->MakeMoment();
    e = oracle->Step(m, viewer, 3);
    oracle->ConfirmPending(mn);
    REQUIRE(!e.player_make_active());

    // Activation energy 1, time 4
    m = mn;
    mn = tl->MakeMoment();
    e = oracle->Step(m, viewer, 2);
    oracle->ConfirmPending(mn);
    // Now the oracle is activated
    REQUIRE(e.player_make_active()); // !!!

    // Activation energy 5, time 5
    m = mn;
    mn = tl->MakeMoment();
    e = oracle->Step(m, viewer, 3);
    oracle->ConfirmPending(mn);
    REQUIRE(!e.player_make_active());

    // Activation energy 3, time 6
    m = mn;
    mn = tl->MakeMoment();
    e = oracle->Step(m, viewer, 3);
    oracle->ConfirmPending(mn);
    REQUIRE(!e.player_make_active());

    // Activation energy 1, time 7
    m = mn;
    mn = tl->MakeMoment();
    e = oracle->Step(m, viewer, 3);
    oracle->ConfirmPending(mn);
    REQUIRE(e.player_make_active()); // !!!

    e = oracle->View(tl->GetMoment(5));
    REQUIRE(e.player_make_active()); // !!!
    e = oracle->View(tl->GetMoment(6));
    REQUIRE(!e.player_make_active());
    e = oracle->View(tl->GetMoment(8));
    REQUIRE(e.player_make_active()); // !!!

    REQUIRE(mn.time() == 8);
    m = mn;
    // We will make a branch
    e = oracle->Branch(m, tl->GetMoment(5));
    REQUIRE(e.player_make_active()); // !!!
    // Let's decide not to make the branch
    e = oracle->Branch(m, tl->GetMoment(7));
    REQUIRE(!e.player_make_active());

    tl = &tp.MakeNewTimeLine(7);
    mn = tl->LatestMoment();
    oracle->ConfirmPending(mn);

    // Activation energy 1, time 7
    m = mn;
    mn = tl->MakeMoment();
    e = oracle->Step(m, viewer, 2);
    REQUIRE(e.player_make_active()); // !!!
    // Let's not confirm that step

    // Player dies without being able to use the Oracle.
    info.HealthRemainingIterator()[0] = 0;
    RoundInfoView viewer2{info, 0};
    e = oracle->Step(m, viewer2, 3);
    REQUIRE(!e.player_make_active());
}

TEST_CASE("Shield item tests", "[shield, item_all]") {
    TimePlane tp{};
    TimeLine* tl = &tp.rightmost_timeline();
    Moment m = tl->LatestMoment();
    Moment mn;
    ItemPtr shield = std::make_unique<Shield>(m);
    RoundInfo info = MakeRoundInfo();
    RoundInfoView viewer{info, 0};

    Effect e = shield->View(m);
    REQUIRE(e.attack_increase() == 0);
    REQUIRE(e.max_hitpoint_increase() == 0);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    // Time 0
    // Take big steps towards unlocking it
    int half = Shield::kUnlockRequirement / 2;
    mn = tl->MakeMoment();
    e = shield->Step(m, viewer, half);
    shield->ConfirmPending(mn);
    REQUIRE(e.attack_increase() == 0);
    REQUIRE(e.max_hitpoint_increase() == 0);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    // Time 1
    m = mn;
    mn = tl->MakeMoment();
    e = shield->Step(m, viewer, Shield::kUnlockRequirement - half);
    shield->ConfirmPending(mn);
    // Now it should be unlocked
    REQUIRE(e.attack_increase() == Shield::kAttackIncrement);
    REQUIRE(e.max_hitpoint_increase() == Shield::kMaxHitpointIncrement);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    e = shield->View(m);
    REQUIRE(e.attack_increase() == 0);
    REQUIRE(e.max_hitpoint_increase() == 0);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    e = shield->View(mn);
    REQUIRE(e.attack_increase() == Shield::kAttackIncrement);
    REQUIRE(e.max_hitpoint_increase() == Shield::kMaxHitpointIncrement);
    REQUIRE(e.shield_amount() == 0);
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    // Shield strength 0, time 2
    REQUIRE(mn.time() == 2);
    m = mn;
    mn = tl->MakeMoment();
    e = shield->Step(m, viewer, 3);
    shield->ConfirmPending(mn);
    REQUIRE(e.attack_increase() == Shield::kAttackIncrement);
    REQUIRE(e.max_hitpoint_increase() == Shield::kMaxHitpointIncrement);
    REQUIRE(e.shield_amount() == 2); // !!!
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());

    // Shield strength 2, time 3
    m = mn;
    mn = tl->MakeMoment();
    e = shield->Step(m, viewer, 3);
    shield->ConfirmPending(mn);
    REQUIRE(e.shield_amount() == 4);

    // Shield strength 4, time 4
    m = mn;
    mn = tl->MakeMoment();
    e = shield->Step(m, viewer, 2);
    shield->ConfirmPending(mn);
    // Shouldn't be able to get past 4
    REQUIRE(e.shield_amount() == 4);

    // Shield strength 4, time 5
    m = mn;
    mn = tl->MakeMoment();
    e = shield->Step(m, viewer, 0);
    shield->ConfirmPending(mn);
    REQUIRE(e.shield_amount() == 3);

    // Shield strength 4, time 6
    e = shield->View(tl->GetMoment(3));
    REQUIRE(e.shield_amount() == 2);
    e = shield->View(tl->GetMoment(4));
    REQUIRE(e.shield_amount() == 4);
    e = shield->View(tl->GetMoment(6));
    REQUIRE(e.shield_amount() == 3);

    REQUIRE(mn.time() == 6);
    m = mn;
    // We will make a branch
    e = shield->Branch(m, tl->GetMoment(3));
    REQUIRE(e.shield_amount() == 2 + 3);
    // Let's decide not to make the branch
    e = shield->Branch(m, tl->GetMoment(4));
    REQUIRE(e.shield_amount() == 4 + 3);

    tl = &tp.MakeNewTimeLine(4);
    mn = tl->LatestMoment();
    shield->ConfirmPending(mn);

    e = shield->View(mn);
    REQUIRE(e.shield_amount() == 4 + 3);

    // Shield strength 4, phantom shield strength 3, time 4
    m = mn;
    mn = tl->MakeMoment();
    e = shield->Step(m, viewer, 2);
    shield->ConfirmPending(mn);
    REQUIRE(e.shield_amount() == 4 + 3);

    // Shield strength 4, phantom shield strength 3, time 5
    m = mn;
    mn = tl->MakeMoment();
    e = shield->Step(m, viewer, 1);
    shield->ConfirmPending(mn);
    REQUIRE(e.shield_amount() == 4 + 3);

    // Shield strength 4, phantom shield strength 3, time 6
    m = mn;
    mn = tl->MakeMoment();
    e = shield->Step(m, viewer, 1);
    shield->ConfirmPending(mn);
    REQUIRE(e.shield_amount() == 4 + 3);

    // Shield strength 4, phantom shield strength 3, time 7
    REQUIRE(mn.time() == 7);
    m = mn;
    // We will make a branch
    e = shield->Branch(m, tl->GetMoment(5));
    REQUIRE(e.shield_amount() == 4 + 4 + 3 / 2);

    tl = &tp.MakeNewTimeLine(5);
    mn = tl->LatestMoment();
    shield->ConfirmPending(mn);

    // Shield strength 4, phantom shield strength 5, time 5
    m = mn;
    mn = tl->MakeMoment();
    e = shield->Step(m, viewer, 0);
    shield->ConfirmPending(mn);
    REQUIRE(e.shield_amount() == 3 + 5);

    // Shield strength 3, phantom shield strength 5, time 6
    m = mn;
    mn = tl->MakeMoment();
    e = shield->Step(m, viewer, 0);
    shield->ConfirmPending(mn);
    REQUIRE(e.shield_amount() == 2 + 5);

    // Shield strength 2, phantom shield strength 5, time 7
    REQUIRE(mn.time() == 7);
    m = mn;
    // We will make a branch
    e = shield->Branch(m, tl->GetMoment(4));
    REQUIRE(e.shield_amount() == 4 + 2 + 5 / 2);
    // Let's decide not to make the branch
    e = shield->Branch(m, tl->GetMoment(5));
    REQUIRE(e.shield_amount() == 4 + 5);
    // Let's decide not to make the branch
    e = shield->Branch(m, tl->GetMoment(1));
    REQUIRE(e.shield_amount() == 0);

    tl = &tp.MakeNewTimeLine(1);
    mn = tl->LatestMoment();
    shield->ConfirmPending(mn);

    // Time 1
    m = mn;
    mn = tl->MakeMoment();
    e = shield->Step(m, viewer, Shield::kUnlockRequirement - half);
    shield->ConfirmPending(mn);
    // Now it should be unlocked
    REQUIRE(e.attack_increase() == Shield::kAttackIncrement);
    REQUIRE(e.max_hitpoint_increase() == Shield::kMaxHitpointIncrement);
    REQUIRE(e.shield_amount() == 0);

    // Shield strength 0, time 2
    m = mn;
    mn = tl->MakeMoment();
    e = shield->Step(m, viewer, 3);
    shield->ConfirmPending(mn);
    REQUIRE(e.shield_amount() == 2);

    // Shield strength 2, time 3
    m = mn;
    mn = tl->MakeMoment();
    e = shield->Step(m, viewer, 3);
    shield->ConfirmPending(mn);
    REQUIRE(e.shield_amount() == 4);

    // Shield strength 4, time 4
    // Player gets hit while charging the Shield
    m = mn;
    mn = tl->MakeMoment();
    info.DamageReceivedIterator()[0] = 3;
    RoundInfoView viewer2{info, 0};
    e = shield->Step(m, viewer2, 3);
    shield->ConfirmPending(mn);
    REQUIRE(e.shield_amount() == 3);
    e = shield->View(mn);
    REQUIRE(e.shield_amount() == 3);

    // Shield strength 3, time 5
    m = mn;
    mn = tl->MakeMoment();
    info.DamageReceivedIterator()[0] = 2;
    viewer2 = RoundInfoView{info, 0};
    e = shield->Step(m, viewer2, 3);
    shield->ConfirmPending(mn);
    REQUIRE(e.shield_amount() == 3);
    e = shield->View(mn);
    REQUIRE(e.shield_amount() == 3);

    // Shield strength 3, time 6
    m = mn;
    mn = tl->MakeMoment();
    info.DamageReceivedIterator()[0] = 1;
    viewer2 = RoundInfoView{info, 0};
    e = shield->Step(m, viewer2, 3);
    shield->ConfirmPending(mn);
    REQUIRE(e.shield_amount() == 4);
    e = shield->View(mn);
    REQUIRE(e.shield_amount() == 4);

    // Shield strength 4, time 7
    REQUIRE(mn.time() == 7);
    m = mn;
    e = shield->Branch(m, tl->GetMoment(4));
    REQUIRE(e.shield_amount() == 4 + 4);

    tl = &tp.MakeNewTimeLine(4);
    mn = tl->LatestMoment();
    shield->ConfirmPending(mn);

    // Shield strength 4, phantom shield strength 4, time 4
    m = mn;
    mn = tl->MakeMoment();
    info.DamageReceivedIterator()[0] = 2;
    viewer2 = RoundInfoView{info, 0};
    e = shield->Step(m, viewer2, 3);
    shield->ConfirmPending(mn);
    REQUIRE(e.shield_amount() == 6);


    // Shield strength 4, phantom shield strength 2, time 4
    m = mn;
    mn = tl->MakeMoment();
    info.DamageReceivedIterator()[0] = 6;
    viewer2 = RoundInfoView{info, 0};
    e = shield->Step(m, viewer2, 3);
    shield->ConfirmPending(mn);
    REQUIRE(e.shield_amount() == 2);

    // Shield strength 2, time 4
    // Player dies without being able to use the Shield.
    m = mn;
    info.DamageReceivedIterator()[0] = 2;
    info.HealthRemainingIterator()[0] = 0;
    viewer2 = RoundInfoView{info, 0};
    e = shield->Step(m, viewer2, 3);
    REQUIRE(e.shield_amount() == 0);

    // Passive branching to the past
    shield->Duplicate(tl->GetMoment(3));

    tl = &tp.MakeNewTimeLine(3);
    mn = tl->LatestMoment();
    shield->ConfirmPending(mn);
    e = shield->View(mn);
    REQUIRE(e.attack_increase() == Shield::kAttackIncrement);
    REQUIRE(e.max_hitpoint_increase() == Shield::kMaxHitpointIncrement);
    REQUIRE(e.shield_amount() == 2); // !!!
    REQUIRE(!e.antitelephone_departure());
    REQUIRE(!e.antitelephone_dest_allowed());
    REQUIRE(!e.player_make_active());
}
