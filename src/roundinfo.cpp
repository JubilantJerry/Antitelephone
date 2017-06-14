#include "roundinfo.hpp"

using namespace roundinfo;

RoundInfo::RoundInfo(int num_players)
    :num_players_(num_players),
     location_data_(num_players),
     damage_received_data_(num_players),
     health_remaining_data_(num_players),
     active_data_(num_players),
     alliance_data_(num_players) {
    for (int i = 0; i < num_players; i++) {
        alliance_data_.SetValue(i, i, true);
    }
}

int RoundInfo::Location(int player, int viewing_player) const {
    if (player < 0 || player >= num_players_ ||
            viewing_player < kOmniscientViewer ||
            viewing_player >= num_players_) {
        throw std::out_of_range("Player ID is invalid");
    }
    return KeepIfEncounter(player, viewing_player,
                           location_data_[player]);
}

int RoundInfo::DamageReceived(int player, int viewing_player) const {
    if (player < 0 || player >= num_players_ ||
            viewing_player < kOmniscientViewer ||
            viewing_player >= num_players_) {
        throw std::out_of_range("Player ID is invalid");
    }
    return KeepIfEncounter(player, viewing_player,
                           damage_received_data_[player]);
}

int RoundInfo::HealthRemaining(int player, int viewing_player) const {
    if (player < 0 || player >= num_players_ ||
            viewing_player < kOmniscientViewer ||
            viewing_player >= num_players_) {
        throw std::out_of_range("Player ID is invalid");
    }
    return KeepIfEncounter(player, viewing_player,
                           health_remaining_data_[player]);
}

bool RoundInfo::Active(int player) const {
    if (player < 0 || player >= num_players_) {
        throw std::out_of_range("Player ID is invalid");
    }
    return active_data_.test(player);
}

void RoundInfo::SetActive(int player, bool value) {
    if (player < 0 || player >= num_players_) {
        throw std::out_of_range("Player ID is invalid");
    }
    active_data_.set(player, value);
}

int RoundInfo::KeepIfEncounter(int player, int viewing_player,
                               int value) const {
    if (viewing_player == kOmniscientViewer ||
            location_data_[player] ==
            location_data_[viewing_player]) {
        return value;
    } else {
        return kUnknown;
    }
}
