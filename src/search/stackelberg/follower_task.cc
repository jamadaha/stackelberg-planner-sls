#include "follower_task.h"

using namespace std;

namespace stackelberg {


    std::vector<const GlobalOperator *> FollowerTask::get_leader_plan() const{
        return std::vector<const GlobalOperator *>();
    }

}
