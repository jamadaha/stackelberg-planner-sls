
#ifndef STACKELBERG_UTIL_H
#define STACKELBERG_UTIL_H

bool op_ptr_name_comp(const GlobalOperator *op1, const GlobalOperator *op2)
{
    return op1->get_name() < op2->get_name();
}

string StackelbergSearch::ops_to_string(vector<const GlobalOperator *> &ops) {
    sort(ops.begin(), ops.end(), op_ptr_name_comp);
    string res = "";
    for (size_t i = 0; i < ops.size(); i++) {
        if(i > 0) {
            res += " ";
        }
        res += ops[i]->get_name();
    }
    return res;
}


#endif
