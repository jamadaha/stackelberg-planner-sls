    // void StackelbergSearch::compute_op_dominance_relation(const
    //     						  vector<GlobalOperator> &ops, vector<vector<int>> &dominated_op_ids)
    // {
    //     cout << "Begin compute_op_dominance_relation()..." << endl;
    //     dominated_op_ids.assign(ops.size(), vector<int>());

    //     for (size_t op_no1 = 0; op_no1 < ops.size(); op_no1++) {
    //         for (size_t op_no2 = 0; op_no2 < ops.size(); op_no2++) {
    //     	if (op_no1 == op_no2) {
    //     	    continue;
    //     	}

    //     	bool dominated_or_equivalent = true;

    //     	const GlobalOperator &op1 = ops[op_no1];
    //     	const GlobalOperator &op2 = ops[op_no2];

    //     	//cout << "Checking dominacce of op1 with id " << op_no1 << ":" << endl;
    //     	//op1.dump();
    //     	//cout << "to op2 with id " << op_no2 << ":" << endl;
    //     	//op2.dump();

    //     	if (op1.get_cost() > op2.get_cost() || op1.get_cost2() > op2.get_cost2()) {
    //     	    continue;
    //     	}

    //     	const vector<GlobalCondition> &conditions1 = op1.get_preconditions();
    //     	const vector<GlobalCondition> &conditions2 = op2.get_preconditions();
    //     	const vector<GlobalEffect> &effects1 = op1.get_effects();
    //     	const vector<GlobalEffect> &effects2 = op2.get_effects();

    //     	if (conditions1.size() > conditions2.size()
    //     	    || effects1.size() != effects2.size()) {
    //     	    continue;
    //     	}

    //     	// The effects need to be the same. We already know that effects1.size() == effects2.size()
    //     	for (size_t i_eff = 0; i_eff < effects1.size(); i_eff++) {
    //     	    if (effects1[i_eff].var != effects2[i_eff].var
    //     		|| effects1[i_eff].val != effects2[i_eff].val) {
    //     		dominated_or_equivalent = false;
    //     		break;
    //     	    }
    //     	}

    //     	if (!dominated_or_equivalent) {
    //     	    continue;
    //     	}

    //     	// Regarding preconditions, every precond of op1 needs to be a precond of op2
    //     	int i_cond2 = 0;
    //     	for (int i_cond1 = 0; i_cond1 < (int) conditions1.size(); i_cond1++) {
    //     	    int var = conditions1[i_cond1].var;
    //     	    int val = conditions1[i_cond1].val;

    //     	    while (i_cond2 < ((int) conditions2.size() - 1)
    //     		   && conditions2[i_cond2].var < var) {
    //     		i_cond2++;
    //     	    }

    //     	    if (i_cond2 >= (int) conditions2.size() || conditions2[i_cond2].var != var
    //     		|| conditions2[i_cond2].val != val) {
    //     		dominated_or_equivalent = false;
    //     		break;
    //     	    }
    //     	}

    //     	/*#ifdef LEADER_SEARCH_DEBUG
    //     	  cout << "op1 dominates op2?: " << dominated_or_equivalent << endl;
    //     	  //#endif*/
    //     	if (dominated_or_equivalent) {
    //     	    dominated_op_ids[op_no1].push_back(op_no2);
    //     	}
    //         }
    //     }
    // }



// void StackelbergSearch::prune_dominated_ops(vector<const GlobalOperator *> &ops,
//                                             vector<vector<int>> dominated_op_ids)
// {
//     vector<bool> marked_for_erase(dominated_op_ids.size(), false);
//     vector<int> dominated_by(dominated_op_ids.size(), -1);

//     //cout << "follower_ops:" << endl;

//     for (size_t op_no = 0; op_no < ops.size(); op_no++) {
//         //follower_ops[op_no]->dump();
//         if (!marked_for_erase[ops[op_no]->get_op_id()]) {
//             const vector<int> dominated_ops = dominated_op_ids[ops[op_no]->get_op_id()];
//             for (size_t dom_op_no = 0; dom_op_no < dominated_ops.size(); dom_op_no++) {
//                 marked_for_erase[dominated_ops[dom_op_no]] = true;
//                 dominated_by[dominated_ops[dom_op_no]] = ops[op_no]->get_op_id();
//             }
//         }
//     }

//     vector<const GlobalOperator *>::iterator it = ops.begin();
//     for (; it != ops.end();) {
//         if (marked_for_erase[(*it)->get_op_id()]) {
//             /*cout << "op: " << endl;
//               (*it)->dump();
//               cout << "pruned because it was dominated by:" << endl;
//               follower_operators[dominated_by[(*it)->get_op_id()]].dump();*/

//             it = ops.erase(it);
//         } else {
//             ++it;
//         }
//     }
// }
