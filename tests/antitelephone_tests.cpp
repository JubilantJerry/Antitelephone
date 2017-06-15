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
#include "../src/antitelephonegame.hpp"
#include "../src/roundinfo.hpp"
#include "../src/roundinfoview.hpp"
#include "../src/itemsutil.hpp"
#include "../src/aliases.hpp"

using namespace roundinfo;
using namespace external;
using namespace item;

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

#define TEST_INTERACTIVE

#ifdef TEST_INTERACTIVE

// Output goes both to a string buffer and std::cout
using TeeDevice_ = boost::iostreams::tee_device<
                   std::stringstream, std::ostream>;
using StringAugmentedStream_ = boost::iostreams::stream<TeeDevice_>;

StringAugmentedStream_ out{TeeDevice_{out_data, std::cout}};

// Input is taken from string buffer, and from std::cin when it runs out.
struct InitializedInput {};
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

#else
// Input and output is directly tied to the input and output data
std::ifstream& in = in_data;
std::stringstream& out = out_data;
#endif

void ShowMomentOverview(MomentOverview const& overview) {
    Moment m = overview.moment();
    out << "[MOMENT (";
    out << m.parent_timeline_num() << ", " << m.time() << "): ";
    out << "PLAYER" << overview.player() << ", (";

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

void Disp(std::string data) {
    std::cout << data << std::endl;
}

MoveData CollectMoveData() {
    // Example valid string: [L3|A0|B1|O2|S3|+0-2+5]
    static std::string pattern = "\\[L([0-9])\\|A([0-9])\\|B([0-9])\\|"
                                 "O([0-9])\\|S([0-9])\\|((?:[+-][0-9])*)\\]";
    static std::regex matcher{pattern};
    std::smatch results;
    std::string input;

    do {
        in >> input;
    } while (!std::regex_match(input, results, matcher));

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

extern RoundInfo MakeRoundInfo();

TEST_CASE("Antitelephone trivial game", "[game_all]") {
    std::string in_path = test_files_path + "input0.txt";
    std::string out_path = test_files_path + "output0.txt";
    REQUIRE(SetupIO("", out_path));

    RoundInfo info{MakeRoundInfo()};
    RoundInfoView viewer{info, 2};
    Moment m{0, 0};

    ItemPtr antitelephone = std::make_unique<Antitelephone>(m);
    ItemPtr bridge = std::make_unique<Bridge>(m);
    ItemPtr oracle = std::make_unique<Oracle>(m);
    ItemPtr shield = std::make_unique<Shield>(m);
    Effect e = antitelephone->View(m) + bridge->View(m) +
               oracle->View(m) + shield->View(m);

    MomentOverview::TaggedValuesArr states {
        antitelephone->StateTaggedValues(m),
        bridge->StateTaggedValues(m),
        oracle->StateTaggedValues(m),
        shield->StateTaggedValues(m)};

    MomentOverview overview{m, e, std::move(states), viewer};

    MoveData read = CollectMoveData();
    ShowMomentOverview(overview);

    std::stringstream ref_stream;
    ref_stream << ref_data.rdbuf();
    REQUIRE(ref_stream.str() == out_data.str());
}
