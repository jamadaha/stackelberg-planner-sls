#include "opt_order.h"

#include <algorithm>
#include <ostream>
#include "../globals.h"
#include "../causal_graph.h"

#include "../utils/debug_macros.h"

using namespace std;

namespace symbolic {
//Returns a optimized variable ordering that reorders the variables
//according to the standard causal graph criterion
void InfluenceGraph::compute_gamer_ordering(std::vector <int> &var_order) {
    if (var_order.empty()) {
        for (size_t v = 0; v < g_variable_domain.size(); v++) {
            var_order.push_back(v);
        }
    }

    InfluenceGraph ig_partitions(g_variable_domain.size());
    for (size_t v = 0; v < g_variable_domain.size(); v++) {
        for (int v2 : g_causal_graph->get_successors(v)) {
            if ((int)v != v2) {
                ig_partitions.set_influence(v, v2);
            }
        }
    }

    ig_partitions.get_ordering(var_order);

    // cout << "Var ordering: ";
    // for(int v : var_order) cout << v << " ";
    // cout  << endl;
}


vector <int> InfluenceGraph::compute_gamer_ordering_local(const std::vector<std::vector<int>> & partitions) {
    InfluenceGraph ig_partitions(g_variable_domain.size());
    for (const auto & partition : partitions) {
        for (int v : partition) {
            for (int v2 : g_causal_graph->get_successors(v)) {
                if ((int)v != v2 && std::find (partition.begin(), partition.end(), v2) != partition.end()) {
                    ig_partitions.set_influence(v, v2);
                }
            }
        }
    }

    vector<int> ordering;
 
    vector<int> partition_begin, partition_sizes;
    int i = 0;
    for (const auto & partition : partitions) {
        partition_sizes.push_back(partition.size());
        partition_begin.push_back(i);
        i += partition.size();
    }

    double value_optimization_function = std::numeric_limits<double>::max();
    
    for (int counter = 0; counter < 20; counter++) {
        vector <int> new_order;
        //Copy the order randomly into new_order respecting the partitions
        for (const auto & partition : partitions) {
            randomize(partition, new_order); 
        }
        
        double new_value = ig_partitions.optimize_variable_ordering_gamer(new_order, partition_begin, partition_sizes);

        if (new_value < value_optimization_function) {
            value_optimization_function = new_value;
            ordering.swap(new_order);
        }
    }

    return ordering;
}


vector <int> InfluenceGraph::compute_gamer_ordering(const std::vector<std::vector<int>> & partitions) {
    InfluenceGraph ig_partitions(g_variable_domain.size());
    for (size_t v = 0; v < g_variable_domain.size(); v++) {
        for (int v2 : g_causal_graph->get_successors(v)) {
            if ((int)v != v2) {
                ig_partitions.set_influence(v, v2);
            }
        }
    }

    vector<int> ordering;
 
    vector<int> partition_begin, partition_sizes;
    int i = 0;
    for (const auto & partition : partitions) {
        partition_sizes.push_back(partition.size());
        partition_begin.push_back(i);
        i += partition.size();
    }

    double value_optimization_function = std::numeric_limits<double>::max();
    
    for (int counter = 0; counter < 20; counter++) {
        vector <int> new_order;
        //Copy the order randomly into new_order respecting the partitions
        for (const auto & partition : partitions) {
            randomize(partition, new_order); 
        }
        
        double new_value = ig_partitions.optimize_variable_ordering_gamer(new_order, partition_begin, partition_sizes);

        if (new_value < value_optimization_function) {
            value_optimization_function = new_value;
            ordering.swap(new_order);
        }
    }

    return ordering;
}





void InfluenceGraph::get_ordering(vector <int> &ordering) const {
    double value_optimization_function = optimize_variable_ordering_gamer(ordering, 50000);
    for (int counter = 0; counter < 20; counter++) {
        vector <int> new_order;
        randomize(ordering, new_order); //Copy the order randomly
        double new_value = optimize_variable_ordering_gamer(new_order, 50000);

        if (new_value < value_optimization_function) {
            value_optimization_function = new_value;
            ordering.swap(new_order);
        }
    }
}


void InfluenceGraph::randomize(const vector <int> &ordering, vector<int> &new_order) {
    for (size_t i = 0; i < ordering.size(); i++) {
	int rnd_pos = g_rng.next(ordering.size() - i);
        int pos = -1;
        do {
            pos++;
            bool found;
            do {
                found = false;
                for (size_t j = 0; j < new_order.size(); j++) {
                    if (new_order[j] == ordering[pos]) {
                        found = true;
                        break;
                    }
                }
                if (found)
                    pos++;
            } while (found);
        } while (rnd_pos-- > 0);
        new_order.push_back(ordering[pos]);
    }
}



double InfluenceGraph::optimize_variable_ordering_gamer(vector <int> &order,
                                                      int iterations) const {
    double totalDistance = compute_function(order);

    double oldTotalDistance = totalDistance;
    //Repeat iterations times
    for (int counter = 0; counter < iterations; counter++) {
        //Swap variable
    int swapIndex1 = g_rng.next(order.size());
    int swapIndex2 = g_rng.next(order.size());
    if(swapIndex1 == swapIndex2) continue;

        //Compute the new value of the optimization function
        for (int i = 0; i < int(order.size()); i++) {
            if ((int)i == swapIndex1 || (int)i == swapIndex2)
                continue;

            if (influence(order[i], order[swapIndex1]))
                totalDistance += (-(i - swapIndex1) * (i - swapIndex1)
                                  + (i - swapIndex2) * (i - swapIndex2));

            if (influence(order[i], order[swapIndex2]))
                totalDistance += (-(i - swapIndex2) * (i - swapIndex2)
                                  + (i - swapIndex1) * (i - swapIndex1));
        }

        //Apply the swap if it is worthy
        if (totalDistance < oldTotalDistance) {
            int tmp = order[swapIndex1];
            order[swapIndex1] = order[swapIndex2];
            order[swapIndex2] = tmp;
            oldTotalDistance = totalDistance;

            /*if(totalDistance != compute_function(order)){
              cerr << "Error computing total distance: " << totalDistance << " " << compute_function(order) << endl;
              exit(-1);
            }else{
              cout << "Bien: " << totalDistance << endl;
            }*/
        } else {
            totalDistance = oldTotalDistance;
        }
    }
//  cout << "Total distance: " << totalDistance << endl;
    return totalDistance;
}



double InfluenceGraph::compute_function(const std::vector <int> &order) const {
    double totalDistance = 0;
    for (size_t i = 0; i < order.size() - 1; i++) {
        for (size_t j = i + 1; j < order.size(); j++) {
            if (influence(order[i], order[j])) {
                totalDistance += (j - i) * (j - i);
            }
        }
    }
    return totalDistance;
}


InfluenceGraph::InfluenceGraph(int num) {
    influence_graph.resize(num);
    for (auto &i : influence_graph) {
        i.resize(num, 0);
    }
}



double InfluenceGraph::optimize_variable_ordering_gamer(vector <int> &order,
                                                        const vector <int> &partition_begin,
                                                        const vector <int> &partition_sizes,
                                                        int iterations) const {
    double totalDistance = compute_function(order);

    double oldTotalDistance = totalDistance;
    //Repeat iterations times
    for (int counter = 0; counter < iterations; counter++) {
        //Swap variable
    int partition = g_rng.next(partition_begin.size()); 

    if(partition_sizes[partition] <= 1) continue;
    int swapIndex1 = partition_begin[partition] + g_rng.next(partition_sizes[partition]);
    int swapIndex2 = partition_begin[partition] + g_rng.next(partition_sizes[partition]);
    if(swapIndex1 == swapIndex2) continue;

        //Compute the new value of the optimization function
        for (int i = 0; i < int(order.size()); i++) {
            if ((int)i == swapIndex1 || (int)i == swapIndex2)
                continue;

            if (influence(order[i], order[swapIndex1]))
                totalDistance += (-(i - swapIndex1) * (i - swapIndex1)
                                  + (i - swapIndex2) * (i - swapIndex2));

            if (influence(order[i], order[swapIndex2]))
                totalDistance += (-(i - swapIndex2) * (i - swapIndex2)
                                  + (i - swapIndex1) * (i - swapIndex1));
        }

        //Apply the swap if it is worthy
        if (totalDistance < oldTotalDistance) {
            int tmp = order[swapIndex1];
            order[swapIndex1] = order[swapIndex2];
            order[swapIndex2] = tmp;
            oldTotalDistance = totalDistance;

            /*if(totalDistance != compute_function(order)){
              cerr << "Error computing total distance: " << totalDistance << " " << compute_function(order) << endl;
              exit(-1);
            }else{
              cout << "Bien: " << totalDistance << endl;
            }*/
        } else {
            totalDistance = oldTotalDistance;
        }
    }
    return totalDistance;
//  cout << "Total distance: " << totalDistance << endl;
}
}
