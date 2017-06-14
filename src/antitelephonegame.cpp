#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <boost/optional.hpp>
#include "pcg_random.hpp"

#include "moment.hpp"
#include "timeplane.hpp"

#include "roundinfo.hpp"
#include "roundinfoview.hpp"

#include "itemsutil.hpp"
#include "queryresult.hpp"

#include "momentoverview.hpp"
#include "movedata.hpp"

#include "antitelephonegame.hpp"
#include "aliases.hpp"

using namespace timeplane;
using namespace roundinfo;
using namespace item;
using namespace external;

///@cond INTERNAL
class AntitelephoneGame::Impl {
  public:
    Impl(int game_id, int num_players, uint64_t random_seed);

    TimePlane const& time_plane() const noexcept {
        return timeplane_;
    }

    AntitelephoneGame::MomentOverviewQueryResult GetOverview(
        int player, Moment m, bool from_rightmost) const;

    QueryResult MakeRegularMove(int player, MoveData const& move);

    QueryResult MakeAntitelephoneMove(int player, Moment dest);

    void RegisterNewRoundHandler(
        std::function<void (MomentOverview)>&& handler);

    void RegisterEndGameHandler(
        std::function<void ()>&& handler);

    Impl(Impl const&) = delete;
    Impl& operator=(Impl const&) = delete;
    Impl(Impl&&) = delete;
    Impl& operator=(Impl&&) = delete;

  private:
    int game_id_;
    int num_players_;
    pcg32 rand_;
    std::function<void(MomentOverview)> new_round_handler_;
    std::function<void(void)> end_game_handler_;
    TimePlane timeplane_;
    std::unordered_map<Moment, RoundInfo> round_info_;
    std::vector<std::vector<ItemPtr>> items_;
    std::unordered_map<int, MoveData> moves_made_;

    void MomentDeleter(MomentIterators m);
};

#define AG_ AntitelephoneGame
#define AI_ AntitelephoneGame::Impl

AI_::Impl(int game_id, int num_players, uint64_t random_seed)
    :game_id_{game_id},
     num_players_{num_players},
     rand_{random_seed},
     items_() {

    Moment first_moment = timeplane_.rightmost_timeline().LatestMoment();
    items_.reserve(num_players);

    RoundInfo initial_info{num_players};
    IntIterator location_data = initial_info.LocationIterator();
    IntIterator damage_received_data = initial_info.LocationIterator();
    IntIterator health_remaining_data = initial_info.LocationIterator();
    for (int i = 0; i < num_players; i++) {
        items_.push_back(MakeItemPtrs(first_moment));
        location_data[i] = RoundInfo::kUnknown;
        damage_received_data[i] = 0;
        health_remaining_data[i] = Item::kBasicMaxHitpoints / 2;
        initial_info.SetActive(i, true);
    }
    round_info_.emplace(first_moment, std::move(initial_info));
}

AG_::MomentOverviewQueryResult AI_::GetOverview(
    int player, Moment m, bool from_rightmost) const {

    auto finder = round_info_.find(m);
    if (player < 0 || player >= num_players_ ||
            finder == round_info_.cend()) {
        return std::make_pair(QueryResult{false, "bad_request"},
                              boost::none);
    }

    RoundInfoView view{finder->second, player, !from_rightmost};
    std::vector<ItemPtr> const& pitems = items_[player];

    std::vector<TaggedValues> item_state_data;
    item_state_data.reserve(ItemTypeCount);
    Effect effect{};

    for (int i = 0; i < ItemTypeCount; i++) {
        ItemPtr const& item = pitems[i];
        effect += item->View(m);
        item_state_data.push_back(item->StateTaggedValues(m));
    }

    return std::make_pair(QueryResult{true}, MomentOverview{
        m, effect, std::move(item_state_data), std::move(view)});
}

void AI_::RegisterNewRoundHandler(
    std::function<void (MomentOverview)>&& handler) {
    new_round_handler_ = handler;
}

void AI_::RegisterEndGameHandler(std::function<void ()>&& handler) {
    end_game_handler_ = handler;
}

AG_::AntitelephoneGame(int game_id, int num_players,
                       uint64_t random_seed)
    :pimpl_{std::make_unique<Impl>(game_id, num_players, random_seed)} {}

AG_::MomentOverviewQueryResult AG_::GetOverview(
    int player, Moment m, bool from_rightmost) const {
    return pimpl_->GetOverview(player, m, from_rightmost);
}

//QueryResult AG_::MakeRegularMove(int player, MoveData const& move) {
//    return pimpl_->MakeRegularMove(player, move);
//}

//QueryResult AG_::MakeAntitelephoneMove(int player, Moment dest) {
//    return pimpl_->MakeAntitelephoneMove(player, dest);
//}

void AG_::RegisterNewRoundHandler(
    std::function<void (MomentOverview)> handler) {
    pimpl_->RegisterNewRoundHandler(std::move(handler));
}

void AG_::RegisterEndGameHandler(std::function<void ()> handler) {
    pimpl_->RegisterEndGameHandler(std::move(handler));
}

AG_::~AntitelephoneGame() = default;

AG_::AntitelephoneGame(AntitelephoneGame&&) = default;

AntitelephoneGame& AG_::operator=(AntitelephoneGame&&) = default;
///@endcond
