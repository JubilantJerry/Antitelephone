#include "moment.hpp"
#include "timeline.hpp"

using namespace timeplane;

///@cond INTERNAL
class TimeLine::Impl {
  public:
    Impl(MomentDeleterFn moment_deleter)
        :timeline_num_{0},
         left_timeline_{},
         branch_time_{0},
         moments_{Moment{0, 0}},
         moment_deleter_{moment_deleter},
         erase_from_{-1} {}

    Impl(TimeLine const& left_timeline, int branch_time,
         MomentDeleterFn moment_deleter);

    Moment const GetMoment(int time) const {
        if (time < branch_time_ && left_timeline_) {
            return left_timeline_->GetMoment(time);
        }
        return moments_.at(time - branch_time_);
    }

    Moment const MakeMoment() {
        assert(externally_reachable_);
        Moment new_moment{timeline_num_, size()};
        moments_.push_back(new_moment);
        return new_moment;
    }

    Moment const LatestMoment() const noexcept {
        assert(externally_reachable_);
        return moments_.back();
    }

    /* Cleans up all moments owned by the instance */
    void CleanUpAllMoments() {
        externally_reachable_ = false;
        CleanUpMomentsInternal(branch_time_);
    }

    /* Cleans up all moments that can only be reached by direct
     * access from the parent TimeLine instance rather than indirectly
     * via timelines on the right. */
    void CleanUpInternallyUnreachableMoments() {
        externally_reachable_ = false;
        int time = erase_from_;
        if (erase_from_ == kInitialEraseFrom) {
            time = branch_time_;
        }
        CleanUpMomentsInternal(time);
    }

    TimeLine::Impl(TimeLine::Impl const&) = delete;
    TimeLine::Impl& operator=(TimeLine::Impl const&) = delete;

  private:
    static int constexpr kInitialEraseFrom = -1;

    int const timeline_num_;
    ::std::shared_ptr<TimeLine::Impl> const left_timeline_;
    int const branch_time_;
    ::std::vector<Moment> moments_;
    MomentDeleterFn moment_deleter_;
    int erase_from_;
    bool externally_reachable_ = true;

    /* Erases and calls the moment deleter for all moments past the
     * specified time. If the time is before the branch time, then the
     * left timeline is also told to erase its moments once possible */
    void CleanUpMomentsInternal(int time);

    int size() {
        return branch_time_ + static_cast<int>(moments_.size());
    }
};

TimeLine::Impl::Impl(TimeLine const& left_timeline, int branch_time,
                     MomentDeleterFn moment_deleter)
    :timeline_num_{left_timeline.pimpl_->timeline_num_ + 1},
     left_timeline_{left_timeline.pimpl_},
     branch_time_{branch_time},
     moments_{Moment{timeline_num_, branch_time}},
     moment_deleter_{moment_deleter},
     erase_from_{-1} {
    if (branch_time < 0 ||
            branch_time >= left_timeline.pimpl_->size()) {
        throw std::out_of_range("Branch time is not valid.");
    }
    if (left_timeline.pimpl_->erase_from_ != -1) {
        throw std::invalid_argument("Left timeline already has a branch.");
    }
    left_timeline.pimpl_->erase_from_ = branch_time;
}

void TimeLine::Impl::CleanUpMomentsInternal(int time) {
    int pos = time - branch_time_;
    if (pos < 0) {
        pos = 0;
        left_timeline_->erase_from_ = time;
        if (!externally_reachable_) {
            left_timeline_->CleanUpMomentsInternal(time);
        } else {
            return;
        }
    }
    if (moments_.size() == 0) {
        return;
    }
    if (!externally_reachable_) {
        auto pos_iter = moments_.cbegin() + pos;
        auto end = moments_.cend();
        if (moment_deleter_) {
            moment_deleter_(std::make_pair(pos_iter, end));
        }
        moments_.erase(pos_iter, end);
        if (moments_.capacity() / 2 >= moments_.size()) {
            moments_.shrink_to_fit();
        }
    }
}

void TimeLine::ImplDeleter(TimeLine::Impl* pimpl) {
    pimpl->CleanUpAllMoments();
    delete pimpl;
}

TimeLine::TimeLine(MomentDeleterFn moment_deleter)
    :pimpl_{new TimeLine::Impl{moment_deleter}, &TimeLine::ImplDeleter} {}

TimeLine::TimeLine(const TimeLine &left_timeline, int branch_time,
                   MomentDeleterFn moment_deleter)
    :pimpl_{new TimeLine::Impl{left_timeline, branch_time,
                               moment_deleter}, &TimeLine::ImplDeleter} {}

TimeLine::~TimeLine() {
    CleanUp();
}

/* Shared function to clean up moments that cannot be accessed */
void TimeLine::CleanUp() {
    if (pimpl_) {
        // The instance has not been moved away
        pimpl_->CleanUpInternallyUnreachableMoments();
    }
}

Moment const TimeLine::GetMoment(int time) const {
    return pimpl_->GetMoment(time);
}

Moment const TimeLine::MakeMoment() {
    return pimpl_->MakeMoment();
}

Moment const TimeLine::LatestMoment() const noexcept {
    return pimpl_->LatestMoment();
}
///@endcond

