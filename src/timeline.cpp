#include "moment.hpp"
#include "timeline.hpp"
#include <algorithm>

using namespace timeplane;

///@cond INTERNAL
class TimeLine::Impl {
  public:
    TimeLine::Impl()
        :timeline_num_{0}, left_timeline_{},
         moments_{Moment{0, 0}}, branch_time_{0} {
    }

    TimeLine::Impl(TimeLine const& left_timeline,
                   int branch_time)
        :timeline_num_{left_timeline.pimpl_->timeline_num_ + 1},
         left_timeline_{left_timeline.pimpl_},
         moments_{Moment{timeline_num_, branch_time}},
         branch_time_{branch_time} {}

    Moment const GetMoment(int time) const {
        if (time < branch_time_ && left_timeline_) {
            return left_timeline_->GetMoment(time);
        }
        return moments_.at(time - branch_time_);
    }

    Moment const MakeMoment() {
        int end_time = branch_time_ + static_cast<int>(moments_.size());
        Moment new_moment{timeline_num_, end_time};
        moments_.push_back(new_moment);
        return new_moment;
    }

    Moment const LatestMoment() const noexcept {
        return moments_.back();
    }

    void EraseMomentsPast(int time);

  private:
    int const timeline_num_;
    ::std::shared_ptr<TimeLine::Impl> const left_timeline_;
    ::std::vector<Moment> moments_;
    int const branch_time_;
};

void TimeLine::Impl::EraseMomentsPast(int time) {
    int pos = time - branch_time_;
    if (pos < 0) {
        pos = 0;
        left_timeline_->EraseMomentsPast(time);
    }
    auto pos_iter = moments_.cbegin() + pos;
    moments_.erase(pos_iter, moments_.cend());
}

TimeLine::TimeLine(TimeLine::MomentDeleter)
    :pimpl_{new TimeLine::Impl{}} {}

TimeLine::TimeLine(const TimeLine &left_timeline, int branch_time,
                   TimeLine::MomentDeleter)
    :pimpl_{new TimeLine::Impl{left_timeline, branch_time}} {}

TimeLine::~TimeLine() {}

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

