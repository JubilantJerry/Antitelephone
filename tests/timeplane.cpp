#include <catch/include/catch.hpp>

#include <sstream>
#include <iostream>
#include <utility>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "../src/moment.hpp"
#include "../src/timeline.hpp"

using namespace timeplane;

TEST_CASE("Moment construction", "[moment, timeplane_all]") {
    Moment m{2, 12};
    REQUIRE(m.parent_timeline_num() == 2);
    REQUIRE(m.time() == 12);
}

TEST_CASE("Moment serialization", "[moment, timeplane_all]") {
    Moment oldm{2, 12};
    std::stringstream stream{};
    boost::archive::text_oarchive output_archive{stream};
    output_archive << oldm;

    Moment newm;
    boost::archive::text_iarchive input_archive{stream};
    input_archive >> newm;

    REQUIRE(newm.parent_timeline_num() == 2);
    REQUIRE(newm.time() == 12);
}

TEST_CASE("Moment hash and equality", "[moment, timeplane_all]") {
    Moment m1{2, 12};
    Moment m2{2, 12};
    Moment m3{2, 12};
    Moment m4{3, 12};
    Moment m5{2, 11};
    Moment m6{2, 11};
    std::hash<Moment> hasher{};

    REQUIRE(m1 == m1);
    REQUIRE(m1 == m2);
    REQUIRE(m2 == m1);
    REQUIRE(m2 == m3);
    REQUIRE(m1 == m3);
    REQUIRE(m1 != m4);
    REQUIRE(m1 != m5);
    REQUIRE(m1 != m6);
    REQUIRE(hasher(m1) == hasher(m1));
    REQUIRE(hasher(m1) == hasher(m2));
    REQUIRE(hasher(m2) == hasher(m1));
    REQUIRE(hasher(m2) == hasher(m3));
    REQUIRE(hasher(m1) == hasher(m3));
}

TEST_CASE("TimeLine default construction", "[timeline, timeplane_all]") {
    TimeLine t0{};
    Moment m = t0.GetMoment(0);

    REQUIRE(m.parent_timeline_num() == 0);
    REQUIRE(m.time() == 0);
}

TEST_CASE("Making and accessing moments in a TimeLine",
          "[timeline, timeplane_all]") {
    TimeLine t0{};
    Moment m1 = t0.MakeMoment();
    Moment m2 = t0.MakeMoment();

    REQUIRE(m1.parent_timeline_num() == 0);
    REQUIRE(m1.time() == 1);
    REQUIRE(m2.parent_timeline_num() == 0);
    REQUIRE(m2.time() == 2);
    REQUIRE(m1 == t0.GetMoment(1));
    REQUIRE(m2 == t0.GetMoment(2));
    REQUIRE_THROWS_AS(t0.GetMoment(-1), std::out_of_range);
    REQUIRE_THROWS_AS(t0.GetMoment(3), std::out_of_range);
}

TEST_CASE("TimeLine branching", "[timeline, timeplane_all]") {
    TimeLine t0{};
    t0.MakeMoment();
    t0.MakeMoment();
    TimeLine t1{t0, 2};
    t1.MakeMoment();

    Moment m2a = t0.GetMoment(2);
    Moment m2b = t1.GetMoment(2);
    Moment m3 = t1.GetMoment(3);

    REQUIRE(t0.GetMoment(0) == t1.GetMoment(0));
    REQUIRE(t0.GetMoment(1) == t1.GetMoment(1));
    REQUIRE(m2a.parent_timeline_num() == 0);
    REQUIRE(m2b.parent_timeline_num() == 1);
    REQUIRE(m2a.time() == 2);
    REQUIRE(m2b.time() == 2);
    REQUIRE(m3.parent_timeline_num() == 1);
    REQUIRE(m3.time() == 3);
    REQUIRE_THROWS_AS(t0.GetMoment(-1), std::out_of_range);
    REQUIRE_THROWS_AS(t0.GetMoment(3), std::out_of_range);
    REQUIRE_THROWS_AS(t1.GetMoment(-1), std::out_of_range);
    REQUIRE_THROWS_AS(t1.GetMoment(4), std::out_of_range);
}

