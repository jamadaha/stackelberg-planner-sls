#ifndef SYMBOLIC_STACKELBERG_MANAGER_H
#define SYMBOLIC_STACKELBERG_MANAGER_H

#include "../symbolic/sym_state_space_manager.h"

class MutexGroup; 
namespace stackelberg {
    
    class SymbolicStackelbergManager : public symbolic::SymStateSpaceManager {
        
        void init_mutex (const std::vector<MutexGroup> &mutex_groups);
        void init_mutex(const std::vector<MutexGroup> &mutex_groups,
                        bool genMutexBDD, bool genMutexBDDByFluent, bool fw);

    public:
        SymbolicStackelbergManager(symbolic::SymVariables *v,
                                   const symbolic::SymParamsMgr &params,
                                   std::shared_ptr<OperatorCostFunction> cost_type_,
                                   const GlobalState & leader_state);

        //Individual TRs: Useful for shrink and plan construction
        std::map<int, std::vector <symbolic::TransitionRelation>> indTRs;

        //notMutex relative for each fluent
        std::vector<std::vector<BDD>> notMutexBDDsByFluentFw, notMutexBDDsByFluentBw;
        std::vector<std::vector<BDD>> exactlyOneBDDsByFluent;

        virtual std::string tag() const override {
            return "original";
        }

        //Methods that require of mutex initialized
        inline const BDD &getNotMutexBDDFw(int var, int val) const {
            return notMutexBDDsByFluentFw[var][val];
        }

        //Methods that require of mutex initialized
        inline const BDD &getNotMutexBDDBw(int var, int val) const {
            return notMutexBDDsByFluentBw[var][val];
        }

        //Methods that require of mutex initialized
        inline const BDD &getExactlyOneBDD(int var, int val) const {
            return exactlyOneBDDsByFluent[var][val];
        }
        
        virtual BDD shrinkExists(const BDD &bdd, int) const override {
            return bdd;
        }
        virtual BDD shrinkForall(const BDD &bdd, int) const override {
            return bdd;
        }
        virtual BDD shrinkTBDD(const BDD &bdd, int) const override  {
            return bdd;
        }

        virtual const std::map<int, std::vector <symbolic::TransitionRelation>> &getIndividualTRs() const {
            return indTRs;
        }
    };
}
#endif
