#include "roundinfo.hpp"
#include "roundinfoview.hpp"

using namespace roundinfo;

RoundInfoView::RoundInfoView() {}

RoundInfoView::RoundInfoView(RoundInfo const& source, int player,
                             bool location_omniscience)
    :player_{player},
     location_data_{},
     damage_received_data_{},
     health_remaining_data_{},
     active_{source.Active(player)},
     allies_{source.num_players()} {
    SymmetricBitMatrix const& allies_matrix = source.alliance_data();
    int num_players = source.num_players();
    int loc_viewer = location_omniscience ?
                     RoundInfo::kOmniscientViewer : player;

    for (int i = 0; i < num_players; i++) {
        int location = source.Location(i, loc_viewer);
        if (location != RoundInfo::kUnknown) {
            location_data_.emplace(i, location);
        }

        int damage_received = source.DamageReceived(i, player);
        if (damage_received != RoundInfo::kUnknown) {
            damage_received_data_.emplace(i, damage_received);
        }

        int health_remaining = source.HealthRemaining(i, player);
        if (health_remaining != RoundInfo::kUnknown) {
            health_remaining_data_.emplace(i, health_remaining);
        }
        allies_.set(i, allies_matrix.Value(player, i));
    }
}

int RoundInfoView::TryFindValue(IntIntMap const& source, int key) const {
    IntIntMap::const_iterator pos = source.find(key);
    if (pos != source.end()) {
        return pos->second;
    } else {
        return RoundInfo::kUnknown;
    }
}
