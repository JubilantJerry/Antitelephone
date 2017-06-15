#include <catch/include/catch.hpp>

#include <sstream>
#include <iostream>
#include <utility>
#include <unordered_set>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/optional/optional.hpp>

#include "../src/moment.hpp"
#include "../src/timeline.hpp"
#include "../src/timeplane.hpp"

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
    TimeLine t2{t1, 3};
    TimeLine t3{t2, 2};
    TimeLine t4{t3, 0};
    // o o o
    //     o o
    //       o
    //     o
    // o

    Moment m2a = t0.GetMoment(2);
    Moment m2b = t1.GetMoment(2);
    Moment m2c = t3.GetMoment(2);
    Moment m3a = t1.GetMoment(3);
    Moment m3b = t2.GetMoment(3);

    REQUIRE(t0.GetMoment(0) == t1.GetMoment(0));
    REQUIRE(t1.GetMoment(0) == t2.GetMoment(0));
    REQUIRE(t2.GetMoment(0) == t3.GetMoment(0));
    REQUIRE(t3.GetMoment(0) != t4.GetMoment(0));
    REQUIRE(t0.GetMoment(1) == t1.GetMoment(1));
    REQUIRE(t1.GetMoment(1) == t2.GetMoment(1));
    REQUIRE(t2.GetMoment(1) == t3.GetMoment(1));

    REQUIRE(m2a.parent_timeline_num() == 0);
    REQUIRE(m2b.parent_timeline_num() == 1);
    REQUIRE(m2c.parent_timeline_num() == 3);
    REQUIRE(m2a.time() == 2);
    REQUIRE(m2b.time() == 2);
    REQUIRE(m2c.time() == 2);
    REQUIRE(m3a.parent_timeline_num() == 1);
    REQUIRE(m3b.parent_timeline_num() == 2);
    REQUIRE(m3b.time() == 3);
    REQUIRE(m3a.time() == 3);

    REQUIRE_THROWS_AS(t0.GetMoment(-1), std::out_of_range);
    REQUIRE_THROWS_AS(t0.GetMoment(3), std::out_of_range);
    REQUIRE_THROWS_AS(t1.GetMoment(-1), std::out_of_range);
    REQUIRE_THROWS_AS(t1.GetMoment(4), std::out_of_range);
    REQUIRE_THROWS_AS(t2.GetMoment(-1), std::out_of_range);
    REQUIRE_THROWS_AS(t2.GetMoment(4), std::out_of_range);
    REQUIRE_THROWS_AS(t3.GetMoment(-1), std::out_of_range);
    REQUIRE_THROWS_AS(t3.GetMoment(3), std::out_of_range);
    REQUIRE_THROWS_AS(t4.GetMoment(-1), std::out_of_range);
    REQUIRE_THROWS_AS(t4.GetMoment(1), std::out_of_range);

    REQUIRE_THROWS_AS(TimeLine(t4, -1), std::out_of_range);
    REQUIRE_THROWS_AS(TimeLine(t4, 1), std::out_of_range);

    REQUIRE_THROWS_AS(TimeLine(t0, 0), std::invalid_argument);
    REQUIRE_THROWS_AS(TimeLine(t0, 1), std::invalid_argument);
    REQUIRE_THROWS_AS(TimeLine(t0, 2), std::invalid_argument);
    REQUIRE_THROWS_AS(TimeLine(t1, 3), std::invalid_argument);
    REQUIRE_THROWS_AS(TimeLine(t2, 3), std::invalid_argument);
    REQUIRE_THROWS_AS(TimeLine(t3, 2), std::invalid_argument);
}

TEST_CASE("TimeLine moment deletion", "[timeline, timeplane_all]") {
    std::unordered_set<Moment> items;
    MomentDeleterFn deleter =
    [&items] (MomentIterators iterators) {
        std::for_each(iterators.first, iterators.second,
        [&items] (Moment m) {
            items.erase(m);
        });
    };

    using TimeLinePtr = std::unique_ptr<TimeLine>;
    TimeLine ta{deleter};
    TimeLinePtr t0 = std::make_unique<TimeLine>(deleter);
    items.insert(t0->LatestMoment());
    items.insert(t0->MakeMoment());
    items.insert(t0->MakeMoment());
    items.insert(t0->MakeMoment());
    items.insert(t0->MakeMoment());
    Moment m0 = t0->LatestMoment();
    REQUIRE(m0.time() == 4);

    // o o o o o
    REQUIRE(items.size() == 5);

    SECTION("The lone TimeLine is destroyed") {
        t0.reset(nullptr);
        // x x x x x
        REQUIRE(items.size() == 0);
    }

    SECTION("A new TimeLine branch is made at time 2") {
        TimeLinePtr t1 = std::make_unique<TimeLine>(*t0, 2, deleter);
        items.insert(t1->LatestMoment());
        items.insert(t1->MakeMoment());
        items.insert(t1->MakeMoment());
        Moment m1 = t1->LatestMoment();
        REQUIRE(m1.time() == 4);

        SECTION("The left TimeLine is destroyed") {
            t0.reset(nullptr);
            // o o x x x
            //     o o o
            REQUIRE(items.size() == 5);
            REQUIRE(items.count(m0) == 0);
            REQUIRE(items.count(m1) == 1);
        }

        SECTION("The right TimeLine is destroyed") {
            t1.reset(nullptr);
            // o o o o o
            //     x x x
            REQUIRE(items.size() == 5);
            REQUIRE(items.count(m0) == 1);
            REQUIRE(items.count(m1) == 0);
        }

        SECTION("A second TimeLine branch is made at time 3") {
            TimeLinePtr t2 = std::make_unique<TimeLine>(*t1, 3, deleter);
            items.insert(t2->LatestMoment());
            items.insert(t2->MakeMoment());
            Moment m2 = t2->LatestMoment();
            REQUIRE(m2.time() == 4);

            SECTION("The leftmost TimeLine is destroyed") {
                t0.reset(nullptr);
                // o o x x x
                //     o o o
                //       o o
                REQUIRE(items.size() == 7);
                REQUIRE(items.count(m0) == 0);
                REQUIRE(items.count(m1) == 1);
                REQUIRE(items.count(m2) == 1);

                SECTION("The middle TimeLine is destroyed") {
                    t1.reset(nullptr);
                    // o o x x x
                    //     o x x
                    //       o o
                    REQUIRE(items.size() == 5);
                    REQUIRE(items.count(m0) == 0);
                    REQUIRE(items.count(m1) == 0);
                    REQUIRE(items.count(m2) == 1);
                }

                SECTION("The rightmost TimeLine is destroyed") {
                    t2.reset(nullptr);
                    // o o x x x
                    //     o o o
                    //       x x
                    REQUIRE(items.size() == 5);
                    REQUIRE(items.count(m0) == 0);
                    REQUIRE(items.count(m1) == 1);
                    REQUIRE(items.count(m2) == 0);
                }
            }

            SECTION("The middle TimeLine is destroyed") {
                t1.reset(nullptr);
                // o o o o o
                //     o x x
                //       o o
                REQUIRE(items.size() == 8);
                REQUIRE(items.count(m0) == 1);
                REQUIRE(items.count(m1) == 0);
                REQUIRE(items.count(m2) == 1);

                SECTION("The leftmost TimeLine is destroyed") {
                    t0.reset(nullptr);
                    // o o x x x
                    //     o x x
                    //       o o
                    REQUIRE(items.size() == 5);
                    REQUIRE(items.count(m0) == 0);
                    REQUIRE(items.count(m1) == 0);
                    REQUIRE(items.count(m2) == 1);
                }

                SECTION("The rightmost TimeLine is destroyed") {
                    t2.reset(nullptr);
                    // o o o o o
                    //     x x x
                    //       x x
                    REQUIRE(items.size() == 5);
                    REQUIRE(items.count(m0) == 1);
                    REQUIRE(items.count(m1) == 0);
                    REQUIRE(items.count(m2) == 0);
                }
            }

            SECTION("The rightmost TimeLine is destroyed") {
                t2.reset(nullptr);
                // o o o o o
                //     o o o
                //       x x
                REQUIRE(items.size() == 8);
                REQUIRE(items.count(m0) == 1);
                REQUIRE(items.count(m1) == 1);
                REQUIRE(items.count(m2) == 0);

                SECTION("The leftmost TimeLine is destroyed") {
                    t0.reset(nullptr);
                    // o o x x x
                    //     o o o
                    //       x x
                    REQUIRE(items.size() == 5);
                    REQUIRE(items.count(m0) == 0);
                    REQUIRE(items.count(m1) == 1);
                    REQUIRE(items.count(m2) == 0);
                }

                SECTION("The middle TimeLine is destroyed") {
                    t1.reset(nullptr);
                    // o o o o o
                    //     x x x
                    //       x x
                    REQUIRE(items.size() == 5);
                    REQUIRE(items.count(m0) == 1);
                    REQUIRE(items.count(m1) == 0);
                    REQUIRE(items.count(m2) == 0);
                }
            }
        }
        SECTION("A second TimeLine branch is made at time 1") {
            TimeLinePtr t2 = std::make_unique<TimeLine>(*t1, 1, deleter);
            items.insert(t2->LatestMoment());
            items.insert(t2->MakeMoment());
            items.insert(t2->MakeMoment());
            items.insert(t2->MakeMoment());
            Moment m2 = t2->LatestMoment();
            REQUIRE(m2.time() == 4);

            SECTION("The leftmost TimeLine is destroyed") {
                t0.reset(nullptr);
                // o o x x x
                //     o o o
                //   o o o o
                REQUIRE(items.size() == 9);
                REQUIRE(items.count(m0) == 0);
                REQUIRE(items.count(m1) == 1);
                REQUIRE(items.count(m2) == 1);

                SECTION("The middle TimeLine is destroyed") {
                    t1.reset(nullptr);
                    // o x x x x
                    //     x x x
                    //   o o o o
                    REQUIRE(items.size() == 5);
                    REQUIRE(items.count(m0) == 0);
                    REQUIRE(items.count(m1) == 0);
                    REQUIRE(items.count(m2) == 1);
                }

                SECTION("The rightmost TimeLine is destroyed") {
                    t2.reset(nullptr);
                    // o o x x x
                    //     o o o
                    //   x x x x
                    REQUIRE(items.size() == 5);
                    REQUIRE(items.count(m0) == 0);
                    REQUIRE(items.count(m1) == 1);
                    REQUIRE(items.count(m2) == 0);
                }
            }

            SECTION("The middle TimeLine is destroyed") {
                t1.reset(nullptr);
                // o o o o o
                //     x x x
                //   o o o o
                REQUIRE(items.size() == 9);
                REQUIRE(items.count(m0) == 1);
                REQUIRE(items.count(m1) == 0);
                REQUIRE(items.count(m2) == 1);

                SECTION("The leftmost TimeLine is destroyed") {
                    t0.reset(nullptr);
                    // o x x x x
                    //     x x x
                    //   o o o o
                    REQUIRE(items.size() == 5);
                    REQUIRE(items.count(m0) == 0);
                    REQUIRE(items.count(m1) == 0);
                    REQUIRE(items.count(m2) == 1);
                }

                SECTION("The rightmost TimeLine is destroyed") {
                    t2.reset(nullptr);
                    // o o o o o
                    //     x x x
                    //   x x x x
                    REQUIRE(items.size() == 5);
                    REQUIRE(items.count(m0) == 1);
                    REQUIRE(items.count(m1) == 0);
                    REQUIRE(items.count(m2) == 0);
                }
            }

            SECTION("The rightmost TimeLine is destroyed") {
                t2.reset(nullptr);
                // o o o o o
                //     o o o
                //   x x x x
                REQUIRE(items.size() == 8);
                REQUIRE(items.count(m0) == 1);
                REQUIRE(items.count(m1) == 1);
                REQUIRE(items.count(m2) == 0);

                SECTION("The leftmost TimeLine is destroyed") {
                    t0.reset(nullptr);
                    // o o x x x
                    //     o o o
                    //   x x x x
                    REQUIRE(items.size() == 5);
                    REQUIRE(items.count(m0) == 0);
                    REQUIRE(items.count(m1) == 1);
                    REQUIRE(items.count(m2) == 0);
                }

                SECTION("The middle TimeLine is destroyed") {
                    t1.reset(nullptr);
                    // o o o o o
                    //     x x x
                    //   x x x x
                    REQUIRE(items.size() == 5);
                    REQUIRE(items.count(m0) == 1);
                    REQUIRE(items.count(m1) == 0);
                    REQUIRE(items.count(m2) == 0);
                }
            }
        }
    }

    SECTION("Cascading moment deletion across many TimeLines") {
        TimeLinePtr tn = std::make_unique<TimeLine>(*t0, 2, deleter);
        items.insert(tn->LatestMoment());
        for (int i = 2; i <= 4; i++) {
            items.insert(tn->MakeMoment());
            tn.reset(new TimeLine{*tn, i + 1, deleter});
            items.insert(tn->LatestMoment());
        }

        items.insert(tn->MakeMoment());
        TimeLinePtr t5 = std::make_unique<TimeLine>(*tn, 6, deleter);
        items.insert(t5->LatestMoment());
        items.insert(t5->MakeMoment());
        tn.reset(new TimeLine{*t5, 7, deleter});
        items.insert(tn->LatestMoment());

        for (int i = 7; i <= 9; i++) {
            items.insert(tn->MakeMoment());
            tn.reset(new TimeLine{*tn, i + 1, deleter});
            items.insert(tn->LatestMoment());
        }

        TimeLinePtr tb = std::make_unique<TimeLine>(*tn, 1, deleter);
        items.insert(tb->LatestMoment());
        t0.reset(nullptr);

        // o o x x x
        //     o x
        //       o x
        //         o x
        //           o x
        //             o o
        //               o x
        //                 o x
        //                   o x
        //                     o x
        //   o
        REQUIRE(items.size() == 13);

        tn.reset(nullptr);
        // o o x x x
        //     o x
        //       o x
        //         o x
        //           o x
        //             o o
        //               x x
        //                 x x
        //                   x x
        //                     x x
        //   o
        REQUIRE(items.size() == 9);

        t5.reset(nullptr);
        // o x x x x
        //     x x
        //       x x
        //         x x
        //           x x
        //             x x
        //               x x
        //                 x x
        //                   x x
        //                     x x
        //   o
        REQUIRE(items.size() == 2);
    }

    SECTION("TimeLine branch edge cases") {
        TimeLinePtr t1 = std::make_unique<TimeLine>(*t0, 0, deleter);
        items.insert(t1->LatestMoment());
        t0.reset(nullptr);

        // x x x x x
        // o
        REQUIRE(items.size() == 1);

        items.insert(t1->MakeMoment());
        items.insert(t1->MakeMoment());
        TimeLinePtr t2 = std::make_unique<TimeLine>(*t1, 2, deleter);
        items.insert(t2->LatestMoment());
        items.insert(t2->MakeMoment());
        items.insert(t2->MakeMoment());
        TimeLinePtr t3 = std::make_unique<TimeLine>(*t2, 2, deleter);
        items.insert(t3->LatestMoment());
        items.insert(t3->MakeMoment());
        t1.reset(nullptr);
        t2.reset(nullptr);

        // x x x x x
        // o o x
        //     x x x
        //     o o
        REQUIRE(items.size() == 4);
    }
}

TEST_CASE("TimePlane basic functionality", "[timeplane, timeplane_all]") {
    TimePlane tp{};

    {
        boost::optional<TimeLine> const& tbad = tp.second_rightmost_timeLine();
        TimeLine& t0 = tp.rightmost_timeline();

        REQUIRE(!tbad.is_initialized());
        Moment m = t0.LatestMoment();
        REQUIRE(m.parent_timeline_num() == 0);
        REQUIRE(m.time() == 0);

        t0.MakeMoment();
        t0.MakeMoment();
    }

    tp.MakeNewTimeLine(2);

    {
        boost::optional<TimeLine> const& t0 = tp.second_rightmost_timeLine();
        TimeLine& t1 = tp.rightmost_timeline();

        REQUIRE(t0.is_initialized());
        Moment m0 = t0.get().LatestMoment();
        REQUIRE(m0.parent_timeline_num() == 0);
        REQUIRE(m0.time() == 2);
        Moment m1 = t1.LatestMoment();
        REQUIRE(m1.parent_timeline_num() == 1);
        REQUIRE(m1.time() == 2);
        REQUIRE(tp.latest_antitelephone_arrival() == 2);
    }

    tp.MakeNewTimeLine(1);

    {
        boost::optional<TimeLine> const& t1 = tp.second_rightmost_timeLine();
        TimeLine& t2 = tp.rightmost_timeline();

        REQUIRE(t1.is_initialized());
        Moment m1 = t1.get().LatestMoment();
        REQUIRE(m1.parent_timeline_num() == 1);
        REQUIRE(m1.time() == 2);
        Moment m2 = t2.LatestMoment();
        REQUIRE(m2.parent_timeline_num() == 2);
        REQUIRE(m2.time() == 1);
        REQUIRE(tp.latest_antitelephone_arrival() == 2);
    }
}

TEST_CASE("TimePlane moment deletion", "[timeplane, timeplane_all]") {
    TimePlane tp{};
    std::unordered_set<Moment> items0;
    std::unordered_set<Moment> items1;
    MomentDeleterFn deleter0 =
    [&items0] (MomentIterators iterators) {
        std::for_each(iterators.first, iterators.second,
        [&items0] (Moment m) {
            items0.erase(m);
        });
    };

    MomentDeleterFn deleter1 =
    [&items1] (MomentIterators iterators) {
        std::for_each(iterators.first, iterators.second,
        [&items1] (Moment m) {
            items1.erase(m);
        });
    };

    tp.RegisterMomentDeleter(deleter0);
    tp.RegisterMomentDeleter(deleter1);

    {
        TimeLine& t0 = tp.rightmost_timeline();
        items0.insert(t0.LatestMoment());
        items1.insert(t0.MakeMoment());
        items0.insert(t0.MakeMoment());
        items1.insert(t0.MakeMoment());
        items0.insert(t0.MakeMoment());
        items1.insert(t0.MakeMoment());
        REQUIRE(t0.LatestMoment().time() == 5);
    }

    tp.MakeNewTimeLine(3);

    {
        TimeLine& t1 = tp.rightmost_timeline();
        items0.insert(t1.LatestMoment());
        items1.insert(t1.MakeMoment());
        items0.insert(t1.MakeMoment());
        REQUIRE(t1.LatestMoment().time() == 5);
    }

    // 010101
    //    010
    REQUIRE(items0.size() == 5);
    REQUIRE(items1.size() == 4);

    tp.MakeNewTimeLine(5);
    TimeLine& t2 = tp.rightmost_timeline();
    items1.insert(t2.LatestMoment());

    // 010xxx
    //    010
    //      1
    REQUIRE(items0.size() == 4);
    REQUIRE(items1.size() == 3);
}


