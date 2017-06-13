#include <catch/include/catch.hpp>

#include <sstream>
#include <iostream>
#include <utility>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/dynamic_bitset.hpp>

#include "../src/aliases.hpp"
#include "../src/symmetricbitmatrix.hpp"
#include "../src/roundinfo.hpp"
#include "../src/roundinfoview.hpp"

using namespace roundinfo;

TEST_CASE("SymmetricBitMatrix overall", "[bitmatrix, misc]") {
    SymmetricBitMatrix m{8};

    REQUIRE(!m.Value(0, 0));
    m.SetValue(0, 0, true);
    REQUIRE(m.Value(0, 0));

    REQUIRE(!m.Value(7, 7));
    m.SetValue(7, 7, true);
    REQUIRE(m.Value(7, 7));

    REQUIRE(!m.Value(2, 3));
    REQUIRE(!m.Value(3, 2));
    m.SetValue(2, 3, true);
    REQUIRE(m.Value(2, 3));
    REQUIRE(m.Value(3, 2));
    m.SetValue(2, 3, false);
    REQUIRE(!m.Value(2, 3));
    REQUIRE(!m.Value(3, 2));

    REQUIRE(!m.Value(4, 7));
    REQUIRE(!m.Value(7, 4));
    m.SetValue(4, 7, true);
    REQUIRE(m.Value(4, 7));
    REQUIRE(m.Value(7, 4));
    m.SetValue(4, 7, false);
    REQUIRE(!m.Value(4, 7));
    REQUIRE(!m.Value(7, 4));

    m = SymmetricBitMatrix{3};
    m.SetValue(1, 2, true);
    m.SetValue(1, 0, true);
    m.SetValue(2, 2, true);

    // F T F
    // T F T
    // F T T
    REQUIRE(!m.Value(0, 0));
    REQUIRE( m.Value(0, 1));
    REQUIRE(!m.Value(0, 2));
    REQUIRE( m.Value(1, 0));
    REQUIRE(!m.Value(1, 1));
    REQUIRE( m.Value(1, 2));
    REQUIRE(!m.Value(2, 0));
    REQUIRE( m.Value(2, 1));
    REQUIRE( m.Value(2, 2));
}

RoundInfo MakeRoundInfo() {
    RoundInfo info{5};

    IntIterator locations = info.LocationIterator();
    locations[0] = 1;
    locations[1] = 4;
    locations[2] = 4;
    locations[3] = 2;
    locations[4] = 3;

    IntIterator damage_received = info.DamageReceivedIterator();
    damage_received[0] = 0;
    damage_received[1] = 2;
    damage_received[2] = 4;
    damage_received[3] = 0;
    damage_received[4] = 0;

    IntIterator health_remaining = info.HealthRemainingIterator();
    health_remaining[0] = 14;
    health_remaining[1] = 5;
    health_remaining[2] = 26;
    health_remaining[3] = 18;
    health_remaining[4] = 9;

    info.SetActive(0, true);
    info.SetActive(1, true);
    info.SetActive(2, true);
    info.SetActive(3, false);
    info.SetActive(4, false);

    SymmetricBitMatrix& alliances = info.alliance_data();
    alliances.SetValue(0, 2, true);
    alliances.SetValue(1, 3, true);
    alliances.SetValue(0, 1, true);

    return info;
}

TEST_CASE("RoundInfo overall", "[roundinfo, round_all]") {
    RoundInfo info = MakeRoundInfo();

    REQUIRE(info.num_players() == 5);

    REQUIRE(info.Location(0, 0) == 1);
    REQUIRE(info.Location(0, 1) == 1);
    REQUIRE(info.Location(0, 2) == 1);
    REQUIRE(info.Location(0, 3) == RoundInfo::kUnknown);
    REQUIRE(info.Location(0, 4) == RoundInfo::kUnknown);
    REQUIRE(info.Location(0, RoundInfo::kOmniscientViewer) == 1);
    REQUIRE(info.Location(1, 0) == 4);
    REQUIRE(info.Location(1, 1) == 4);
    REQUIRE(info.Location(1, 2) == 4);
    REQUIRE(info.Location(1, 3) == 4);
    REQUIRE(info.Location(1, 4) == RoundInfo::kUnknown);
    REQUIRE(info.Location(1, RoundInfo::kOmniscientViewer) == 4);
    REQUIRE(info.Location(2, 0) == 4);
    REQUIRE(info.Location(2, 1) == 4);
    REQUIRE(info.Location(2, 2) == 4);
    REQUIRE(info.Location(2, 3) == RoundInfo::kUnknown);
    REQUIRE(info.Location(2, 4) == RoundInfo::kUnknown);
    REQUIRE(info.Location(2, RoundInfo::kOmniscientViewer) == 4);
    REQUIRE(info.Location(3, 0) == RoundInfo::kUnknown);
    REQUIRE(info.Location(3, 1) == 2);
    REQUIRE(info.Location(3, 2) == RoundInfo::kUnknown);
    REQUIRE(info.Location(3, 3) == 2);
    REQUIRE(info.Location(3, 4) == RoundInfo::kUnknown);
    REQUIRE(info.Location(3, RoundInfo::kOmniscientViewer) == 2);
    REQUIRE(info.Location(4, 0) == RoundInfo::kUnknown);
    REQUIRE(info.Location(4, 1) == RoundInfo::kUnknown);
    REQUIRE(info.Location(4, 2) == RoundInfo::kUnknown);
    REQUIRE(info.Location(4, 3) == RoundInfo::kUnknown);
    REQUIRE(info.Location(4, 4) == 3);
    REQUIRE(info.Location(4, RoundInfo::kOmniscientViewer) == 3);
    REQUIRE_THROWS_AS(info.Location(4, 5), std::out_of_range);
    REQUIRE_THROWS_AS(info.Location(4, -2), std::out_of_range);
    REQUIRE_THROWS_AS(info.Location(5, 4), std::out_of_range);
    REQUIRE_THROWS_AS(info.Location(-2, 4), std::out_of_range);

    REQUIRE(info.DamageReceived(0, 0) == 0);
    REQUIRE(info.DamageReceived(0, 1) == 0);
    REQUIRE(info.DamageReceived(0, 2) == 0);
    REQUIRE(info.DamageReceived(0, 3) == RoundInfo::kUnknown);
    REQUIRE(info.DamageReceived(0, 4) == RoundInfo::kUnknown);
    REQUIRE(info.DamageReceived(0, RoundInfo::kOmniscientViewer) == 0);
    REQUIRE(info.DamageReceived(1, 0) == 2);
    REQUIRE(info.DamageReceived(1, 1) == 2);
    REQUIRE(info.DamageReceived(1, 2) == 2);
    REQUIRE(info.DamageReceived(1, 3) == 2);
    REQUIRE(info.DamageReceived(1, 4) == RoundInfo::kUnknown);
    REQUIRE(info.DamageReceived(1, RoundInfo::kOmniscientViewer) == 2);
    REQUIRE(info.DamageReceived(2, 0) == 4);
    REQUIRE(info.DamageReceived(2, 1) == 4);
    REQUIRE(info.DamageReceived(2, 2) == 4);
    REQUIRE(info.DamageReceived(2, 3) == RoundInfo::kUnknown);
    REQUIRE(info.DamageReceived(2, 4) == RoundInfo::kUnknown);
    REQUIRE(info.DamageReceived(2, RoundInfo::kOmniscientViewer) == 4);
    REQUIRE(info.DamageReceived(3, 0) == RoundInfo::kUnknown);
    REQUIRE(info.DamageReceived(3, 1) == 0);
    REQUIRE(info.DamageReceived(3, 2) == RoundInfo::kUnknown);
    REQUIRE(info.DamageReceived(3, 3) == 0);
    REQUIRE(info.DamageReceived(3, 4) == RoundInfo::kUnknown);
    REQUIRE(info.DamageReceived(3, RoundInfo::kOmniscientViewer) == 0);
    REQUIRE(info.DamageReceived(4, 0) == RoundInfo::kUnknown);
    REQUIRE(info.DamageReceived(4, 1) == RoundInfo::kUnknown);
    REQUIRE(info.DamageReceived(4, 2) == RoundInfo::kUnknown);
    REQUIRE(info.DamageReceived(4, 3) == RoundInfo::kUnknown);
    REQUIRE(info.DamageReceived(4, 4) == 0);
    REQUIRE(info.DamageReceived(4, RoundInfo::kOmniscientViewer) == 0);
    REQUIRE_THROWS_AS(info.DamageReceived(4, 5), std::out_of_range);
    REQUIRE_THROWS_AS(info.DamageReceived(4, -2), std::out_of_range);
    REQUIRE_THROWS_AS(info.DamageReceived(5, 4), std::out_of_range);
    REQUIRE_THROWS_AS(info.DamageReceived(-2, 4), std::out_of_range);


    REQUIRE(info.HealthRemaining(0, 0) == 14);
    REQUIRE(info.HealthRemaining(0, 1) == 14);
    REQUIRE(info.HealthRemaining(0, 2) == 14);
    REQUIRE(info.HealthRemaining(0, 3) == RoundInfo::kUnknown);
    REQUIRE(info.HealthRemaining(0, 4) == RoundInfo::kUnknown);
    REQUIRE(info.HealthRemaining(0, RoundInfo::kOmniscientViewer) == 14);
    REQUIRE(info.HealthRemaining(1, 0) == 5);
    REQUIRE(info.HealthRemaining(1, 1) == 5);
    REQUIRE(info.HealthRemaining(1, 2) == 5);
    REQUIRE(info.HealthRemaining(1, 3) == 5);
    REQUIRE(info.HealthRemaining(1, 4) == RoundInfo::kUnknown);
    REQUIRE(info.HealthRemaining(1, RoundInfo::kOmniscientViewer) == 5);
    REQUIRE(info.HealthRemaining(2, 0) == 26);
    REQUIRE(info.HealthRemaining(2, 1) == 26);
    REQUIRE(info.HealthRemaining(2, 2) == 26);
    REQUIRE(info.HealthRemaining(2, 3) == RoundInfo::kUnknown);
    REQUIRE(info.HealthRemaining(2, 4) == RoundInfo::kUnknown);
    REQUIRE(info.HealthRemaining(2, RoundInfo::kOmniscientViewer) == 26);
    REQUIRE(info.HealthRemaining(3, 0) == RoundInfo::kUnknown);
    REQUIRE(info.HealthRemaining(3, 1) == 18);
    REQUIRE(info.HealthRemaining(3, 2) == RoundInfo::kUnknown);
    REQUIRE(info.HealthRemaining(3, 3) == 18);
    REQUIRE(info.HealthRemaining(3, 4) == RoundInfo::kUnknown);
    REQUIRE(info.HealthRemaining(3, RoundInfo::kOmniscientViewer) == 18);
    REQUIRE(info.HealthRemaining(4, 0) == RoundInfo::kUnknown);
    REQUIRE(info.HealthRemaining(4, 1) == RoundInfo::kUnknown);
    REQUIRE(info.HealthRemaining(4, 2) == RoundInfo::kUnknown);
    REQUIRE(info.HealthRemaining(4, 3) == RoundInfo::kUnknown);
    REQUIRE(info.HealthRemaining(4, 4) == 9);
    REQUIRE(info.HealthRemaining(4, RoundInfo::kOmniscientViewer) == 9);
    REQUIRE_THROWS_AS(info.HealthRemaining(4, 5), std::out_of_range);
    REQUIRE_THROWS_AS(info.HealthRemaining(4, -2), std::out_of_range);
    REQUIRE_THROWS_AS(info.HealthRemaining(5, 4), std::out_of_range);
    REQUIRE_THROWS_AS(info.HealthRemaining(-2, 4), std::out_of_range);

    REQUIRE( info.Active(0));
    REQUIRE( info.Active(1));
    REQUIRE( info.Active(2));
    REQUIRE(!info.Active(3));
    REQUIRE(!info.Active(4));
    REQUIRE_THROWS_AS(info.Active(5), std::out_of_range);

    SymmetricBitMatrix const& calliances = info.calliance_data();
    REQUIRE( calliances.Value(0, 0));
    REQUIRE( calliances.Value(0, 1));
    REQUIRE( calliances.Value(0, 2));
    REQUIRE(!calliances.Value(0, 3));
    REQUIRE(!calliances.Value(0, 4));
    REQUIRE( calliances.Value(1, 0));
    REQUIRE( calliances.Value(1, 1));
    REQUIRE(!calliances.Value(1, 2));
    REQUIRE( calliances.Value(1, 3));
    REQUIRE(!calliances.Value(1, 4));
    REQUIRE( calliances.Value(2, 0));
    REQUIRE(!calliances.Value(2, 1));
    REQUIRE( calliances.Value(2, 2));
    REQUIRE(!calliances.Value(2, 3));
    REQUIRE(!calliances.Value(2, 4));
    REQUIRE(!calliances.Value(3, 0));
    REQUIRE( calliances.Value(3, 1));
    REQUIRE(!calliances.Value(3, 2));
    REQUIRE( calliances.Value(3, 3));
    REQUIRE(!calliances.Value(3, 4));
    REQUIRE(!calliances.Value(4, 0));
    REQUIRE(!calliances.Value(4, 1));
    REQUIRE(!calliances.Value(4, 2));
    REQUIRE(!calliances.Value(4, 3));
    REQUIRE( calliances.Value(4, 4));
}

void TestRoundInfoViewers(RoundInfoView const& viewer0,
                          RoundInfoView const& viewer1,
                          RoundInfoView const& viewer2,
                          RoundInfoView const& viewer3,
                          RoundInfoView const& viewer4) {
    REQUIRE(viewer0.player() == 0);
    REQUIRE(viewer1.player() == 1);
    REQUIRE(viewer2.player() == 2);
    REQUIRE(viewer3.player() == 3);
    REQUIRE(viewer4.player() == 4);

    REQUIRE(viewer0.Location(0) == 1);
    REQUIRE(viewer1.Location(0) == 1);
    REQUIRE(viewer2.Location(0) == 1);
    REQUIRE(viewer3.Location(0) == RoundInfo::kUnknown);
    REQUIRE(viewer4.Location(0) == 1);
    REQUIRE(viewer0.Location(1) == 4);
    REQUIRE(viewer1.Location(1) == 4);
    REQUIRE(viewer2.Location(1) == 4);
    REQUIRE(viewer3.Location(1) == 4);
    REQUIRE(viewer4.Location(1) == 4);
    REQUIRE(viewer0.Location(2) == 4);
    REQUIRE(viewer1.Location(2) == 4);
    REQUIRE(viewer2.Location(2) == 4);
    REQUIRE(viewer3.Location(2) == RoundInfo::kUnknown);
    REQUIRE(viewer4.Location(2) == 4);
    REQUIRE(viewer0.Location(3) == RoundInfo::kUnknown);
    REQUIRE(viewer1.Location(3) == 2);
    REQUIRE(viewer2.Location(3) == RoundInfo::kUnknown);
    REQUIRE(viewer3.Location(3) == 2);
    REQUIRE(viewer4.Location(3) == 2);
    REQUIRE(viewer0.Location(4) == RoundInfo::kUnknown);
    REQUIRE(viewer1.Location(4) == RoundInfo::kUnknown);
    REQUIRE(viewer2.Location(4) == RoundInfo::kUnknown);
    REQUIRE(viewer3.Location(4) == RoundInfo::kUnknown);
    REQUIRE(viewer4.Location(4) == 3);

    REQUIRE(viewer0.DamageReceived(0) == 0);
    REQUIRE(viewer1.DamageReceived(0) == 0);
    REQUIRE(viewer2.DamageReceived(0) == 0);
    REQUIRE(viewer3.DamageReceived(0) == RoundInfo::kUnknown);
    REQUIRE(viewer4.DamageReceived(0) == RoundInfo::kUnknown);
    REQUIRE(viewer0.DamageReceived(1) == 2);
    REQUIRE(viewer1.DamageReceived(1) == 2);
    REQUIRE(viewer2.DamageReceived(1) == 2);
    REQUIRE(viewer3.DamageReceived(1) == 2);
    REQUIRE(viewer4.DamageReceived(1) == RoundInfo::kUnknown);
    REQUIRE(viewer0.DamageReceived(2) == 4);
    REQUIRE(viewer1.DamageReceived(2) == 4);
    REQUIRE(viewer2.DamageReceived(2) == 4);
    REQUIRE(viewer3.DamageReceived(2) == RoundInfo::kUnknown);
    REQUIRE(viewer4.DamageReceived(2) == RoundInfo::kUnknown);
    REQUIRE(viewer0.DamageReceived(3) == RoundInfo::kUnknown);
    REQUIRE(viewer1.DamageReceived(3) == 0);
    REQUIRE(viewer2.DamageReceived(3) == RoundInfo::kUnknown);
    REQUIRE(viewer3.DamageReceived(3) == 0);
    REQUIRE(viewer4.DamageReceived(3) == RoundInfo::kUnknown);
    REQUIRE(viewer0.DamageReceived(4) == RoundInfo::kUnknown);
    REQUIRE(viewer1.DamageReceived(4) == RoundInfo::kUnknown);
    REQUIRE(viewer2.DamageReceived(4) == RoundInfo::kUnknown);
    REQUIRE(viewer3.DamageReceived(4) == RoundInfo::kUnknown);
    REQUIRE(viewer4.DamageReceived(4) == 0);

    REQUIRE(viewer0.HealthRemaining(0) == 14);
    REQUIRE(viewer1.HealthRemaining(0) == 14);
    REQUIRE(viewer2.HealthRemaining(0) == 14);
    REQUIRE(viewer3.HealthRemaining(0) == RoundInfo::kUnknown);
    REQUIRE(viewer4.HealthRemaining(0) == RoundInfo::kUnknown);
    REQUIRE(viewer0.HealthRemaining(1) == 5);
    REQUIRE(viewer1.HealthRemaining(1) == 5);
    REQUIRE(viewer2.HealthRemaining(1) == 5);
    REQUIRE(viewer3.HealthRemaining(1) == 5);
    REQUIRE(viewer4.HealthRemaining(1) == RoundInfo::kUnknown);
    REQUIRE(viewer0.HealthRemaining(2) == 26);
    REQUIRE(viewer1.HealthRemaining(2) == 26);
    REQUIRE(viewer2.HealthRemaining(2) == 26);
    REQUIRE(viewer3.HealthRemaining(2) == RoundInfo::kUnknown);
    REQUIRE(viewer4.HealthRemaining(2) == RoundInfo::kUnknown);
    REQUIRE(viewer0.HealthRemaining(3) == RoundInfo::kUnknown);
    REQUIRE(viewer1.HealthRemaining(3) == 18);
    REQUIRE(viewer2.HealthRemaining(3) == RoundInfo::kUnknown);
    REQUIRE(viewer3.HealthRemaining(3) == 18);
    REQUIRE(viewer4.HealthRemaining(3) == RoundInfo::kUnknown);
    REQUIRE(viewer0.HealthRemaining(4) == RoundInfo::kUnknown);
    REQUIRE(viewer1.HealthRemaining(4) == RoundInfo::kUnknown);
    REQUIRE(viewer2.HealthRemaining(4) == RoundInfo::kUnknown);
    REQUIRE(viewer3.HealthRemaining(4) == RoundInfo::kUnknown);
    REQUIRE(viewer4.HealthRemaining(4) == 9);

    REQUIRE( viewer0.active());
    REQUIRE( viewer1.active());
    REQUIRE( viewer2.active());
    REQUIRE(!viewer3.active());
    REQUIRE(!viewer4.active());

    BitSet const& allies0 = viewer0.allies();
    BitSet const& allies1 = viewer1.allies();
    BitSet const& allies2 = viewer2.allies();
    BitSet const& allies3 = viewer3.allies();
    BitSet const& allies4 = viewer4.allies();
    REQUIRE( allies0.test(0));
    REQUIRE( allies1.test(0));
    REQUIRE( allies2.test(0));
    REQUIRE(!allies3.test(0));
    REQUIRE(!allies4.test(0));
    REQUIRE( allies0.test(1));
    REQUIRE( allies1.test(1));
    REQUIRE(!allies2.test(1));
    REQUIRE( allies3.test(1));
    REQUIRE(!allies4.test(1));
    REQUIRE( allies0.test(2));
    REQUIRE(!allies1.test(2));
    REQUIRE( allies2.test(2));
    REQUIRE(!allies3.test(2));
    REQUIRE(!allies4.test(2));
    REQUIRE(!allies0.test(3));
    REQUIRE( allies1.test(3));
    REQUIRE(!allies2.test(3));
    REQUIRE( allies3.test(3));
    REQUIRE(!allies4.test(3));
    REQUIRE(!allies0.test(4));
    REQUIRE(!allies1.test(4));
    REQUIRE(!allies2.test(4));
    REQUIRE(!allies3.test(4));
    REQUIRE( allies4.test(4));
}

TEST_CASE("RoundInfoView overall", "[roundinfoview, round_all]") {
    RoundInfo info = MakeRoundInfo();

    SECTION("Regular construction") {
        RoundInfoView viewer0{info, 0, false};
        RoundInfoView viewer1{info, 1, false};
        RoundInfoView viewer2{info, 2};
        RoundInfoView viewer3{info, 3};
        RoundInfoView viewer4{info, 4, true};

        TestRoundInfoViewers(viewer0, viewer1, viewer2,
                             viewer3, viewer4);
    }

    SECTION("Serialization and deserialization") {
        RoundInfoView viewer0{info, 0, false};
        RoundInfoView viewer1{info, 1, false};
        RoundInfoView viewer2{info, 2};
        RoundInfoView viewer3{info, 3};
        RoundInfoView viewer4{info, 4, true};

        std::stringstream stream{};
        boost::archive::text_oarchive output_archive{stream};
        output_archive << viewer0 << viewer1 << viewer2;
        output_archive << viewer3 << viewer4;

        boost::archive::text_iarchive input_archive{stream};
        RoundInfoView cviewer0{};
        RoundInfoView cviewer1{};
        RoundInfoView cviewer2{};
        RoundInfoView cviewer3{};
        RoundInfoView cviewer4{};
        input_archive >> cviewer0 >> cviewer1 >> cviewer2;
        input_archive >> cviewer3 >> cviewer4;

        TestRoundInfoViewers(cviewer0, cviewer1, cviewer2,
                             cviewer3, cviewer4);
    }
}
