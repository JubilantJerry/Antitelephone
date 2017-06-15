#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <random>
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

#define AG_ AntitelephoneGame
#define AI_ AntitelephoneGame::Impl

class AntitelephoneGame::Impl {
  public:
    Impl(int game_id, int num_players, uint64_t random_seed);

    TimePlane const& time_plane() const noexcept {
        return timeplane_;
    }

    AG_::MomentOverviewQueryResult GetOverview(
        int player, Moment m, bool from_rightmost) const;

    QueryResult MakeRegularMove(int player, MoveData&& move);

    QueryResult MakeAntitelephoneMove(int player, int dest_time);

    // R-value references to avoid an extra copy operation
    void RegisterNewRoundHandler(AG_::NewRoundHandler&& handler);

    void RegisterTravelHandler(AG_::TravelHandler&& handler);

    void RegisterEndGameHandler(AG_::EndGameHandler&& handler);

    Impl(Impl const&) = delete;
    Impl& operator=(Impl const&) = delete;
    Impl(Impl&&) = delete;
    Impl& operator=(Impl&&) = delete;

  private:
    int game_id_;
    int num_players_;
    pcg32 rand_;
    AG_::NewRoundHandler new_round_handler_;
    AG_::TravelHandler travel_handler_;
    AG_::EndGameHandler end_game_handler_;
    TimePlane timeplane_;
    std::unordered_map<Moment, RoundInfo> round_info_;
    std::unordered_map<Moment, std::unordered_map<int, MoveData>> moves_info_;
    std::vector<ItemArr> items_;
    std::unordered_map<int, MoveData> moves_pending_;
    bool game_over;

    inline int NumLocations();

    bool MoveValid(MoveData const& move);

    void ProcessMoves();

    void MomentDeleter(MomentIterators m);
};

AI_::Impl(int game_id, int num_players, uint64_t random_seed)
    :game_id_{game_id},
     num_players_{num_players},
     rand_{random_seed},
     items_(),
     game_over{false} {
    assert(num_players >= kMinNumPlayers && num_players <= kMaxNumPlayers);

    // Obtain the first moment
    Moment first_moment = timeplane_.rightmost_timeline().LatestMoment();
    items_.reserve(num_players);

    // Set up the initial round information and items
    RoundInfo initial_info{num_players};
    IntIterator location_data =
        initial_info.LocationIterator();
    IntIterator damage_received_data =
        initial_info.DamageReceivedIterator();
    IntIterator health_remaining_data =
        initial_info.HealthRemainingIterator();
    for (int i = 0; i < num_players; i++) {
        items_.push_back(MakeItemPtrs(first_moment));
        location_data[i] = RoundInfo::kUnknown;
        damage_received_data[i] = 0;
        health_remaining_data[i] = Item::kBasicMaxHitpoints / 2;
        initial_info.SetActive(i, true);
    }
    round_info_.emplace(first_moment, std::move(initial_info));

    // Register the moment deleter functions
    timeplane_.RegisterMomentDeleter([this] (MomentIterators iter) {
        this->MomentDeleter(iter);
    });
    for (ItemArr const& pitems: items_) {
        for (ItemPtr const& item: pitems) {
            timeplane_.RegisterMomentDeleter([&item] (MomentIterators iter) {
                item->MomentDeleter(iter);
            });
        }
    }
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
    ItemArr const& pitems = items_[player];

    MomentOverview::TaggedValuesArr item_state_data;
    Effect effect{};

    for (int i = 0; i < ItemTypeCount; i++) {
        ItemPtr const& item = pitems[i];
        effect += item->View(m);
        item_state_data[i] = item->StateTaggedValues(m);
    }

    return std::make_pair(QueryResult{}, MomentOverview{
        m, effect, std::move(item_state_data), std::move(view)});
}

int AI_::NumLocations() {
    return kRoomsPerPlayer * num_players_;
}

bool AI_::MoveValid(MoveData const& move) {
    int location = move.new_location();
    if (location < 0 || location >= NumLocations()) {
        return false;
    }
    int total_energy = 0;
    for (int i = 0; i < ItemTypeCount; i++) {
        int energy_input = move.EnergyInput(i);
        if (energy_input < 0) {
            return false;
        }
        total_energy += energy_input;
    }
    if (total_energy > kEnergyPerRound) {
        return false;
    }
    for (int alliance: move.added_alliances()) {
        if (alliance < 0 || alliance > num_players_) {
            return false;
        }
    }
    for (int alliance: move.removed_alliances()) {
        if (alliance < 0 || alliance > num_players_) {
            return false;
        }
    }
    return true;
}

QueryResult AI_::MakeRegularMove(int player, MoveData&& move) {

    if (game_over || player < 0 || player >= num_players_ ||
            moves_pending_.count(player) || !MoveValid(move)) {
        return QueryResult{false, "bad_request"};
    }

    MoveData* move_to_use = &move;
    TimeLine const& timeline = timeplane_.rightmost_timeline();
    Moment curr = timeline.LatestMoment();
    RoundInfo const& curr_info = round_info_.at(curr);
    if (!curr_info.Active(player)) {
        TimeLine const& timeline_sec =
            timeplane_.second_rightmost_timeLine().get();
        move_to_use = &moves_info_.at(timeline_sec.GetMoment(curr.time()))
                      .at(player);
    }

    moves_pending_.emplace(player, *move_to_use);
    if (moves_pending_.size() == num_players_) {
        ProcessMoves();
    }
    return QueryResult{};
}

QueryResult AI_::MakeAntitelephoneMove(int player, int dest_time) {

    TimeLine* timeline = &timeplane_.rightmost_timeline();
    Moment curr = timeline->LatestMoment();
    if (game_over || player < 0 || player >= num_players_ ||
            dest_time < 0 || dest_time >= curr.time() ||
            moves_pending_.size() != num_players_) {
        return QueryResult{false, "bad_request"};
    }

    // Determine whether Antitelephone is allowed
    Moment dest = timeline->GetMoment(dest_time);
    Effect antiplayer_effect{};
    ItemArr const& antiplayer_items = items_[player];
    for (ItemPtr const& item: antiplayer_items) {
        antiplayer_effect += item->Branch(curr, dest);
    }
    if (dest_time > timeplane_.latest_antitelephone_arrival()
            && !antiplayer_effect.antitelephone_dest_allowed()) {
        return QueryResult{false, "antitelephone_prohibited"};
    }

    // Things to be incorporated into moment overviews
    std::vector<Effect> effects(num_players_);
    effects[player] = antiplayer_effect;
    std::vector<MomentOverview::TaggedValuesArr> item_state_data;
    item_state_data.reserve(num_players_);

    // This assumes that the second rightmost timeline is not
    // out of scope after this branch.
    timeline = &timeplane_.MakeNewTimeLine(dest_time);
    Moment new_moment = timeline->LatestMoment();

    // Update every player's item to the new moment
    for (int i = 0; i < num_players_; i++) {
        ItemArr const& pitems = items_[i];
        MomentOverview::TaggedValuesArr pitem_state_data;
        for (int j = 0; j < ItemTypeCount; j++) {
            ItemPtr const& item = pitems[i];
            // The antitelephone player has already been dealt with
            if (i != player) {
                effects[i] += item->View(dest);
                item->Duplicate(dest);
            }
            item->ConfirmPending(new_moment);
            pitem_state_data[j] = item->StateTaggedValues(new_moment);
        }
        item_state_data.push_back(pitem_state_data);
    }

    // Create a new set of round information
    RoundInfo new_info{round_info_.at(dest)};
    for (int i = 0; i < num_players_; i++) {
        new_info.SetActive(i, false);
    }
    new_info.SetActive(player, true);
    round_info_.emplace(new_moment, new_info);

    // Create moment overviews and call the new round handler
    if (new_round_handler_) {
        std::vector<MomentOverview> overviews;
        overviews.reserve(num_players_);
        for (int i = 0; i < num_players_; i++) {
            overviews.emplace_back(new_moment, effects[i],
                                   std::move(item_state_data[i]),
                                   RoundInfoView{new_info, i});
        }
        new_round_handler_(std::move(overviews));
    }
    moves_info_.emplace(curr, std::move(moves_pending_));
    moves_pending_ = std::unordered_map<int, MoveData>();
    return QueryResult{};
}

/*
 * This is one long function, which basically increments the game state
 * forward by one step. It needs to handle encounters and any resulting
 * combat, look out for Antitelephone departures and set the active status
 * of the players correctly for the next round. The function uses the
 * accumulated move data stored internally, so the caller has the
 * responsibility to make sure that these moves are correct, even in cases
 * where the player is inactive and the move is copied from past events */
void AI_::ProcessMoves() {
    assert(moves_pending_.size() == num_players_);
    TimeLine& timeline = timeplane_.rightmost_timeline();
    Moment curr = timeline.LatestMoment();
    RoundInfo new_info{round_info_.at(curr)};

    IntIterator location_data = new_info.LocationIterator();
    IntIterator damage_received = new_info.DamageReceivedIterator();
    IntIterator health_remaining = new_info.HealthRemainingIterator();
    SymmetricBitMatrix alliances = new_info.alliance_data();

    // Update alliance information
    for (int i = 0; i < num_players_; i++) {
        MoveData const& pmove = moves_pending_.at(i);
        for (int new_alliance: pmove.added_alliances()) {
            if (moves_pending_.at(new_alliance).added_alliances().count(i)) {
                // Both sides agreed to be allies
                alliances.SetValue(i, new_alliance, true);
            }
        }
        for (int broken_alliance: pmove.removed_alliances()) {
            // No agreement is needed to break an alliance
            alliances.SetValue(i, broken_alliance, false);
        }
    }

    // Compute the effects bestowed by each player's items
    // These will first contain current effects. Then they will
    // contain effects that will apply for the next round.
    std::vector<Effect> effects = std::vector<Effect>(num_players_);
    // Also apply the healing effect from energy usage.
    for (int pid = 0; pid < num_players_; pid++) {
        ItemArr const& pitems = items_[pid];
        MoveData const& pmove = moves_pending_.at(pid);
        int used_energy = 0;
        for (int iid = 0; iid < ItemTypeCount; iid++) {
            ItemPtr const& item = pitems[iid];
            effects[pid] += item->View(curr);
            used_energy += pmove.EnergyInput(iid);
        }
        // Heal only if alive, and up to the maximum health.
        if (health_remaining[pid] > 0) {
            health_remaining[pid] += (kEnergyPerRound - used_energy);
            int max_health = effects[pid].max_hitpoint_increase();
            if (health_remaining[pid] > max_health) {
                health_remaining[pid] = max_health;
            }
        }
    }

    // Fill in locations and identify encounters
    int constexpr kNoEncounter = -1;
    std::vector<int> weakest_opponent =
        std::vector<int>(num_players_, kNoEncounter);
    SymmetricBitMatrix encounters{num_players_};
    for (int newp = 0; newp < num_players_; newp++) {
        if (health_remaining[newp] == 0) {
            // Dead players can't go anywhere
            location_data[newp] = RoundInfo::kGraveyardLocation;
            // Also, dead players cannot encounter anyone else.
        } else {
            location_data[newp] = moves_pending_.at(newp).new_location();

            // Update encounter data
            for (int oldp = 0; oldp < newp; oldp++) {
                // Players must be at the same location
                if (location_data[newp] != location_data[oldp]) {
                    continue;
                }

                // Set weakest opponent of the player. They can't be allies.
                if (!alliances.Value(newp, oldp)) {
                    // Is the added player the weakest one?
                    if (weakest_opponent[oldp] == kNoEncounter ||
                            health_remaining[newp] <
                            health_remaining[weakest_opponent[oldp]]) {
                        weakest_opponent[oldp] = newp;
                    }
                    // Who is the weakest opponent of the added player?
                    if (weakest_opponent[newp] == kNoEncounter ||
                            health_remaining[oldp] <
                            health_remaining[weakest_opponent[newp]]) {
                        weakest_opponent[newp] = oldp;
                    }
                }

                // Add the encounter to the matrix
                encounters.SetValue(newp, oldp, true);
            }
        }
    }

    // Now they fight!
    // Initially they don't take damage
    for (int i = 0; i < num_players_; i++) {
        damage_received[i] = 0;
    }
    // Rack up damage from each fighter
    for (int i = 0; i < num_players_; i++) {
        damage_received[weakest_opponent[i]] += effects[i].attack_increase();
    }
    // Calculate remaining health
    for (int i = 0; i < num_players_; i++) {
        int net_damage = damage_received[i] - effects[i].shield_amount();
        if (net_damage > 0) {
            health_remaining[i] -= net_damage;
            if (health_remaining[i] < 0) {
                health_remaining[i] = 0;
            }
        }
    }

    // Make round info views, and update items / effects while at it
    // Update the effects vector to reflect effects for the next round.
    std::vector<RoundInfoView> views;
    views.reserve(num_players_);
    std::vector<int> antiplayers; // Players who activated the antitelephone
    for (int pid = 0; pid < num_players_; pid++) {
        views.emplace_back(new_info, pid);
        ItemArr const& pitems = items_[pid];
        // Don't forget to zero out the effects first
        effects[pid] = Effect{};
        MoveData const& pmove = moves_pending_.at(pid);
        for (int iid = 0; iid < ItemTypeCount; iid++) {
            ItemPtr const& item = pitems[iid];
            effects[pid] += item->Step(curr, views[pid],
                                       pmove.EnergyInput(iid));
        }
        // Any weird effects to deal with?
        if (effects[pid].antitelephone_departure()) {
            antiplayers.push_back(pid);
        }
        if (effects[pid].player_make_active() && !new_info.Active(pid)) {
            new_info.SetActive(pid, true);
            views[pid].set_active(true);
        }
    }

    int num_antiplayers = static_cast<int>(antiplayers.size());
    if (num_antiplayers > 0) {
        // Who will be the true antitelephone player?
        std::uniform_int_distribution<int> uniform(0, num_antiplayers);
        int antiplayer = antiplayers[uniform(rand_)];
        if (travel_handler_) {
            travel_handler_(antiplayer);
        }
        // Have to discard all the work done above, but then again
        // time travel tends to undo things anyway.
        return;
    }

    // Make a new moment one step into the future
    Moment new_moment = timeline.MakeMoment();

    // If there's a second-rightmost timeline and inactive players,
    // check for changed encounter data and make players active if found.
    boost::optional<TimeLine> const& timeline_sec_opt =
        timeplane_.second_rightmost_timeLine();

    // One of the few times I think goto makes things clearer...

    // *** if (I need to use the second rightmost timeline) {
    if (!timeline_sec_opt) {
        goto timeline_sec_handling_end;
    }
    TimeLine const& timeline_sec = timeline_sec_opt.get();
    int new_time = new_moment.time();
    if (timeline_sec.LatestMoment().time() < new_time) {
        goto timeline_sec_handling_end;
    }

    // *** (if statement body)
    RoundInfo const& sec_info =
        round_info_.at(timeline_sec.GetMoment(new_time));
    for (int sleeper = 0; sleeper < num_players_; sleeper++) {
        if (new_info.Active(sleeper)) {
            // No need to care about already active players
            continue;
        }
        int sleeper_location = location_data[sleeper];
        int omnv = RoundInfo::kOmniscientViewer;
        // If the sleeper is in a different location across
        // the timelines, then something has gone horribly wrong.
        assert(sleeper_location == sec_info.Location(sleeper, omnv));

        for (int other = 0; other < num_players_; other++) {
            // Checking for equality of two boolean expressions.
            // Just a heads up since it's slightly confusing.
            if ((sec_info.Location(other, omnv) == sleeper_location) !=
                    encounters.Value(sleeper, other)) {
                // Either a missed encounter, or a new encounter with
                // an active player involved.
                new_info.SetActive(sleeper, true);
                views[sleeper].set_active(true);
            }
        }
    }
timeline_sec_handling_end:
    // *** }

    // Now to finalize everything
    std::vector<MomentOverview::TaggedValuesArr> item_state_data;
    item_state_data.reserve(num_players_);
    for (ItemArr const& pitems: items_) {
        MomentOverview::TaggedValuesArr pitem_state_data;
        for (int i = 0; i < ItemTypeCount; i++) {
            ItemPtr const& item = pitems[i];
            item->ConfirmPending(new_moment);
            pitem_state_data[i] = item->StateTaggedValues(new_moment);
        }
        item_state_data.push_back(pitem_state_data);
    }

    // Create moment overviews and call the new round handler
    if (new_round_handler_) {
        std::vector<MomentOverview> overviews;
        overviews.reserve(num_players_);
        for (int i = 0; i < num_players_; i++) {
            overviews.emplace_back(new_moment, effects[i],
                                   std::move(item_state_data[i]),
                                   RoundInfoView{new_info, i});
        }
        new_round_handler_(std::move(overviews));
    }

    // Is the game over yet?
    std::vector<int> survivors;
    for(int i = 0; i < num_players_; i++) {
        if (health_remaining[i] > 0) {
            survivors.push_back(i);
        }
    }
    bool exists_pair_of_enemies = false;
    for (int survivor_left: survivors) {
        for (int survivor_right: survivors) {
            if (survivor_left < survivor_right &&
                    !alliances.Value(survivor_left, survivor_right)) {
                exists_pair_of_enemies = true;
            }
        }
    }
    if (!exists_pair_of_enemies) {
        game_over = true;
        if (end_game_handler_) {
            end_game_handler_();
        }
    }

    // No turning back, moving lots of important data
    round_info_.emplace(new_moment, std::move(new_info));

    // Note, the moves are associated with curr, not the new moment
    moves_info_.emplace(curr, std::move(moves_pending_));
    moves_pending_ = std::unordered_map<int, MoveData>();
}

void AI_::RegisterNewRoundHandler(NewRoundHandler&& handler) {
    new_round_handler_ = handler;
}

void AI_::RegisterTravelHandler(TravelHandler&& handler) {
    travel_handler_ = handler;
}

void AI_::RegisterEndGameHandler(EndGameHandler&& handler) {
    end_game_handler_ = handler;
}

void AI_::MomentDeleter(MomentIterators m) {
    std::for_each(m.first, m.second,
    [this] (Moment to_delete) {
        this->round_info_.erase(to_delete);
        this->moves_info_.erase(to_delete);
    });
}

AG_::AntitelephoneGame(int game_id, int num_players,
                       uint64_t random_seed)
    :pimpl_{std::make_unique<Impl>(game_id, num_players, random_seed)} {}

AG_::MomentOverviewQueryResult AG_::GetOverview(
    int player, Moment m, bool from_rightmost) const {
    return pimpl_->GetOverview(player, m, from_rightmost);
}

QueryResult AG_::MakeRegularMove(int player, MoveData move) {
    return pimpl_->MakeRegularMove(player, std::move(move));
}

QueryResult AG_::MakeAntitelephoneMove(int player, int dest_time) {
    return pimpl_->MakeAntitelephoneMove(player, dest_time);
}

void AG_::RegisterNewRoundHandler(NewRoundHandler handler) {
    pimpl_->RegisterNewRoundHandler(std::move(handler));
}

void AG_::RegisterTravelHandler(TravelHandler handler) {
    pimpl_->RegisterTravelHandler(std::move(handler));
}

void AG_::RegisterEndGameHandler(EndGameHandler handler) {
    pimpl_->RegisterEndGameHandler(std::move(handler));
}

AG_::~AntitelephoneGame() = default;

AG_::AntitelephoneGame(AntitelephoneGame&&) = default;

AntitelephoneGame& AG_::operator=(AntitelephoneGame&&) = default;
///@endcond
