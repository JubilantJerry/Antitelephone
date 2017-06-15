#include "catch/include/catch.hpp"

#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <boost/iostreams/tee.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/optional.hpp>

#include "../src/momentoverview.hpp"
#include "../src/movedata.hpp"

#include "../src/roundinfo.hpp"
#include "../src/roundinfoview.hpp"

#include "../src/itemsutil.hpp"

#include "../src/queryresult.hpp"
#include "../src/aliases.hpp"

#include "../src/moment.hpp"
#include "../src/timeline.hpp"
#include "../src/timeplane.hpp"

#include "../src/antitelephonegame.hpp"

using namespace roundinfo;
using namespace external;
using namespace item;
using namespace timeplane;

std::stringstream out_data{};
std::ifstream in_data{};
std::ifstream ref_data{};

bool SetupIO(std::string in_path, std::string out_path) {
    if (in_data.is_open()) {
        in_data.close();
    }
    in_data.clear();
    in_data.open(in_path);
    if (ref_data.is_open()) {
        ref_data.close();
    }
    ref_data.clear();
    ref_data.open(out_path);
    out_data.str(std::string{});
    out_data.clear();

    bool success = true;
    if (!in_data.is_open() && in_path != "") {
        std::cout << "File not found: " << in_path << std::endl;
        success = false;
    }
    if (!ref_data.is_open() && out_path != "") {
        std::cout << "File not found: " << out_path << std::endl;
        success = false;
    }
    return success;
}

std::string test_files_path = TEST_FILES_PATH;

//#define TEST_INTERACTIVE

#ifdef TEST_INTERACTIVE

// Output goes both to a string buffer and std::cout
using TeeDevice_ = boost::iostreams::tee_device<
                   std::stringstream, std::ostream>;
using StringAugmentedStream_ = boost::iostreams::stream<TeeDevice_>;

StringAugmentedStream_ out{TeeDevice_{out_data, std::cout}};

// Input is taken from string buffer, and from std::cin when it runs out.
struct InitializedInput {
    InitializedInput& ignore(std::streamsize n = 1, int delim = EOF) {
        if (!in_data.is_open() || in_data.eof()) {
            std::cin.ignore(n, delim);
        } else {
            in_data.ignore(n, delim);
        }
        return *this;
    }
};
InitializedInput in;

template<typename T>
InitializedInput& operator>>(InitializedInput&, T& item) {
    if (!in_data.is_open() || in_data.eof()) {
        std::cin >> item;
    } else {
        in_data >> item;
    }
    return in;
}

InitializedInput& getline(InitializedInput&,
                          std::string& item, char delim = '\n') {
    if (!in_data.is_open() || in_data.eof()) {
        getline(std::cin, item, delim);
    } else {
        getline(in_data, item, delim);
    }
    return in;
}

#else
// Input and output is directly tied to the input and output data
std::ifstream& in = in_data;
std::stringstream& out = out_data;
#endif

void ShowMomentOverview(MomentOverview const& overview) {
    Moment m = overview.moment();
    out << "[MOMENT (";
    out << m.parent_timeline_num() << ", " << m.time() << "): ";
    out << "PLAYER " << overview.player() << ", (";

    Effect e = overview.effect();
    out << "EFFECTS: ";
    out << "ATK " << e.attack_increase();
    out << " HPMAX " << e.max_hitpoint_increase();
    out << " SHLD " << e.shield_amount();
    out << " " << e.antitelephone_departure();
    out << e.antitelephone_dest_allowed();
    out << e.player_make_active() << "), (ITEMS:";

    for (int i = 0; i < item::ItemTypeCount; i++) {
        out << " |ITEM " << i << "|";
        for (TaggedValue tag: overview.ItemState(i)) {
            out << tag.first << "=" << tag.second << "|";
        }
    }
    out << "), (";

    RoundInfoView const& view = overview.round_info();
    out << "ROUNDINFO:";
    if (view.active()) {
        out << " ACTIVE,";
    }

    for (int i = 0; i < view.num_players(); i++) {
        out << " |P" << i << "|";
        int value = view.Location(i);
        if (value != RoundInfo::kUnknown) {
            out << "LOC=" << value << "|";
        }
        value = view.HealthRemaining(i);
        if (value != RoundInfo::kUnknown) {
            out << "HP=" << value << "|";
        }
        value = view.DamageReceived(i);
        if (value != RoundInfo::kUnknown) {
            out << "DMG=" << value << "|";
        }
        if (view.allies().test(i)) {
            out << "ALLY|";
        }
    }
    out << ")]" << std::endl;
}

void ShowQueryResult(QueryResult const& result) {
    if (result.success()) {
        out << "SUCCESS";
        if (result.response_tag() != "") {
            out << result.response_tag();
        }
        out << std::endl;
        return;
    }
    out << "FAILURE ";
    if (result.response_tag() != "") {
        out << result.response_tag();
    }
    out << std::endl;
}

MoveData CollectMoveData() {
    // Example valid string: "[L3|A0|B1|O2|S3|+0-2+5] # Optional comment"
    static std::string pattern = "\\[L([0-9]+)\\|A([0-9]+)\\|B([0-9]+)\\|"
                                 "O([0-9]+)\\|S([0-9]+)\\|"
                                 "((?:[+-][0-9])*)\\](?: #.*)?";
    // The regex assumes that there cannot be more than 10 players
    static std::regex matcher{pattern};
    std::smatch results;
    std::string input;

    bool got_move = false;
    while (!got_move) {
        getline(in, input);
        if (!std::regex_match(input, results, matcher)) {
            out << "BAD SYNTAX" << std::endl;
        } else {
            got_move = true;
        }
    }

    auto iter = results.cbegin();
    iter++; // Remove the match of the whole string
    MoveData movedata{};
    movedata.set_new_location(std::stoi(*(iter++)));
    for (int i = 0; i < ItemTypeCount; i++) {
        movedata.SetEnergyInput(i, std::stoi(*(iter++)));
    }

    std::string alliance_data = *iter;
    for (int i = 0; i < alliance_data.size(); i += 2) {
        int player = std::stoi(alliance_data.substr(i + 1, 1));
        if (alliance_data[i] == '+') {
            movedata.add_alliance(player);
        } else {
            movedata.remove_alliance(player);
        }
    }
    return movedata;
}

void RunGameEngine(int num_players) {
    bool running = true;
    int antiplayer = -1;
    static int constexpr test_game_id = 42;

    AntitelephoneGame::NewRoundHandler round_handler =
    [] (int game_id, std::vector<MomentOverview> const&& overviews) {
        REQUIRE(game_id == test_game_id);
        out << "NEW ROUND HANDLER" << std::endl;
        for (MomentOverview const& overview: overviews) {
            ShowMomentOverview(overview);
        }
    };

    AntitelephoneGame::TravelHandler travel_handler =
    [&antiplayer] (int game_id, int player) {
        REQUIRE(game_id == test_game_id);
        out << "TRAVEL HANDLER" << std::endl;
        antiplayer = player;
    };

    AntitelephoneGame::EndGameHandler end_handler =
    [&running] (int game_id) {
        REQUIRE(game_id == test_game_id);
        out << "END GAME HANDLER" << std::endl;
        running = false;
    };

    AntitelephoneGame game{42, num_players};
    game.RegisterNewRoundHandler(round_handler);
    game.RegisterTravelHandler(travel_handler);
    game.RegisterEndGameHandler(end_handler);
    TimePlane const& tp = game.time_plane();

    out << "GAME START" << std::endl;

    while (running) {
        if (antiplayer != -1) {
            out << "PLAYER " << antiplayer;
            out << " ANTITELPHONE DEST?" << std::endl;
            int dest_time;
            in >> dest_time;
            in.ignore();
            ShowQueryResult(game.MakeAntitelephoneMove(
                                antiplayer, dest_time));
            antiplayer = -1;
            continue;
        }
        std::string line;
        while(true) {
            getline(in, line);
            if (line == "" || line[0] == '#') {
                // It's an empty line or a comment
                continue;
            } else if (line == "SHOW OVERVIEW") {
                int player, time;
                bool left;
                in >> player;
                in >> time;
                in >> left;
                in.ignore();
                Moment m;
                if (!left) {
                    auto const& tl = tp.rightmost_timeline();
                    if (time >= 0 && time <= tl.LatestMoment().time()) {
                        m = tp.rightmost_timeline().GetMoment(time);
                    } else {
                        out << "BAD MOMENT" << std::endl;
                        continue;
                    }
                } else {
                    auto const& tl = tp.second_rightmost_timeLine();
                    if (tl && time >= 0 &&
                            time <= tl.get().LatestMoment().time()) {
                        m = tp.rightmost_timeline().GetMoment(time);
                    } else {
                        out << "BAD MOMENT" << std::endl;
                        continue;
                    }
                }
                auto result = game.GetOverview(player, m);
                if (result.second) {
                    ShowMomentOverview(result.second.get());
                }
                ShowQueryResult(result.first);
            } else if (line == "MAKE MOVES") {
                break;
            } else {
                out << "BAD INSTRUCTION: " << line;
            }
        }
        for (int i = 0; i < num_players; i++) {
            out << "PLAYER " << i << " MOVE?" << std::endl;
            QueryResult result{false};
            while (!result) {
                result = game.MakeRegularMove(i, CollectMoveData());
                ShowQueryResult(result);
            }
        }
    }
    out << "GAME END" << std::endl << std::endl;
}

void CompareOutputWithReference(int test_no) {
    int line_no = 1;
    std::string line_from_output;
    std::string line_from_reference;
    std::string test_case_identifier = " in test case ";
    test_case_identifier += std::to_string(test_no);

    while (!out_data.eof() && !ref_data.eof()) {
        getline(out_data, line_from_output);
        getline(ref_data, line_from_reference);
        if (line_from_output != line_from_reference) {
            FAIL(std::string("Line ") + std::to_string(line_no) +
                 test_case_identifier + " does not match");
        } else {
            REQUIRE(true);
        }
        line_no++;
    }
    if (!out_data.eof()) {
        FAIL("Extra output data" + test_case_identifier);
    }
    if (!ref_data.eof()) {
        FAIL("Missing output data" + test_case_identifier);
    }
}

TEST_CASE("Antitelephone test 0", "[game_all]") {
    std::string in_path = test_files_path + "input0.txt";
    std::string out_path = test_files_path + "output0.txt";
    REQUIRE(SetupIO(in_path, out_path));

    RunGameEngine(2);
    CompareOutputWithReference(0);
}

TEST_CASE("Antitelephone test 1", "[game_all]") {
    std::string in_path = test_files_path + "input1.txt";
    std::string out_path = test_files_path + "output1.txt";
    REQUIRE(SetupIO(in_path, out_path));

    RunGameEngine(2);
    CompareOutputWithReference(1);
}

#ifdef TEST_INTERACTIVE
TEST_CASE("Antitelephone test interactive", "[game_all]") {
    SetupIO("", "");
    out << "INTERACTIVE MODE. NUMBER OF PLAYERS?" << std::endl;
    int num_players = 0;
    in >> num_players;
    if (num_players < AntitelephoneGame::kMinNumPlayers ||
            num_players > AntitelephoneGame::kMaxNumPlayers) {
        out << "BAD NUMBER OF PLAYERS" << std::endl;
    } else {
        RunGameEngine(num_players);
    }
}
#endif
