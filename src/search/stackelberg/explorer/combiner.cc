#include "combiner.h"
#include "util.h"

Combiner::Combiner(
  const std::size_t &&min_parameters,
  const std::size_t &&max_parameters,
  const std::size_t &&min_precondition_size,
  const std::size_t &&max_precondition_size
) : 
    min_parameters(std::move(min_parameters)),
    max_parameters(std::move(max_parameters)),
    min_precondition_size(std::move(min_precondition_size)),
    max_precondition_size(std::move(max_precondition_size))
{}

void Combiner::Init(
  std::shared_ptr<symbolic::SymVariables> vars,
  const World &world
) {
    this->vars = vars;
    for (const auto &instantiation : FindInstantiations(MetaOperatorName())) {
        std::vector<std::size_t> ins;
        for (const auto &o : instantiation)
            ins.push_back(world.ObjectIndex(o));
        this->default_instantiations.push_back(ins);
    }
    std::cout << "Default instantiations: " << this->default_instantiations.size() << std::endl;
    this->default_parameters = this->default_instantiations[0].size();
    const auto meta_name = MetaOperatorName();
    const auto param_count = ActionParameters(meta_name);
    for (std::size_t i = 1; i <= this->max_parameters; i++) {
        for (std::size_t t = 0; t < world.PredicateCount(); t++) {
            const std::size_t predicate_params = world.PredicateParameters(t);
            for (auto permutation : Cartesian(predicate_params, param_count + i)) {
                bool novel = false;
                for (const auto &p : permutation)
                    if (p >= param_count) novel = true;
                if (novel) 
                    this->expansion_literals[i].push_back(Literal(t, std::move(permutation)));
            }
        }
    }
}

void Combiner::Combine(
    const World &world,
    const BDD &valid,
    const BDD &invalid,
    std::function<void (Combination)> f
) {
    const auto meta_name = MetaOperatorName();
    const auto param_count = ActionParameters(meta_name);
    printf("Parameters: %zu/%zu\n", param_count, param_count);
    const auto literals = GenerateRoot(world, param_count);
    printf("Literals: %zu\n", literals.size());
    std::vector<std::size_t> comb;
    ExploreDAG(world, literals, valid, invalid, comb, f);
}

std::vector<Literal> Combiner::GenerateRoot(
    const World &world, 
    std::size_t param_count
) {
    std::vector<Literal> literals;
    
    for (std::size_t i = 0; i < world.PredicateCount(); i++) {
        const std::size_t predicate_params = world.PredicateParameters(i);
        for (auto permutation : Cartesian(predicate_params, param_count))
            literals.push_back(Literal(i, std::move(permutation)));
    }

    return literals;
}

void Combiner::ExploreDAG(
    const World &world, 
    const std::vector<Literal> &literals,
    const BDD &valid,
    const BDD &invalid,
    std::vector<std::size_t> &comb,
    std::function<void (Combination)> f
) {
    for (std::size_t i = comb.empty() ? 0 : comb.back() + 1; i < literals.size(); i++) {
        comb.push_back(i);
        const BDD c_valid = Reduce(world, this->default_instantiations, literals, comb, valid);
        const BDD c_invalid = Reduce(world, this->default_instantiations, literals, comb, invalid);
        const BDD c_applicable = c_valid | c_invalid;
        if (vars->numStates(c_applicable) != 0) {
            std::vector<Literal> c_literals;
            for (const auto &i : comb) c_literals.push_back(literals[i]);
            f(Combination(
                this->vars->numStates(c_applicable),
                this->vars->numStates(c_invalid),
                {},
                c_literals
            ));
            if (comb.size() < this->max_precondition_size) {
                Expand(world, literals, comb, c_valid, c_invalid, f);
                ExploreDAG(world, literals, c_valid, c_invalid, comb, f);
            }
        }
        comb.pop_back();
    }
}

BDD Combiner::Reduce(
  const World &world, 
  const std::vector<std::vector<std::size_t>> &instantiations,
  const std::vector<Literal> &literals,
  const std::vector<std::size_t> &comb,
  const BDD &bdd
) {
    BDD c_bdd = this->vars->zeroBDD();
    for (const auto &instantiation : instantiations) {
        BDD i_bdd = bdd;
        for (const auto &i : comb) {
            const auto &literal = literals[i];
            const auto &predicate = literal.predicate;
            const auto &parameters = literal.params;
            std::vector<std::size_t> objects;
            for (const auto &parameter : parameters)
                objects.push_back(instantiation[parameter]);
            if (world.IsStatic(predicate)) {
                if (!world.HasStatic(predicate, objects)) {
                    i_bdd = this->vars->zeroBDD();
                    break;
                }
                continue;
            }
            const auto f_bdd = world.FactBDD(predicate, objects);
            if (f_bdd == nullptr) {
                i_bdd = this->vars->zeroBDD();
                break;
            }
            i_bdd &= *f_bdd;
        }
        c_bdd |= i_bdd;
    }
    return c_bdd;
}

void Combiner::Expand(
  const World &world, 
  const std::vector<Literal> &p_literals,
  const std::vector<std::size_t> &p_comb,
  const BDD &valid,
  const BDD &invalid,
  std::function<void (Combination)> f
) {
    for (std::size_t i = 1; i <= this->max_parameters; i++) {
        const auto type_perms = Cartesian(i, world.TypeCount());
        const auto &e_literals = this->expansion_literals.at(i);
        std::vector<Literal> literals;
        for (const auto &l : p_literals) literals.push_back(l);
        for (const auto &l : e_literals) literals.push_back(l);

        for (const auto &types : type_perms) {
            std::vector<std::vector<std::size_t>> instantiations;
            for (const auto &d_instantiation : this->default_instantiations) {
                std::vector<std::vector<std::size_t>> options;
                for (const auto &o : d_instantiation) 
                    options.push_back({o});
                for (const auto t : types) 
                    options.push_back(world.TypeObjects(t));
                for (const auto &instantiation : Cartesian(options))
                    instantiations.push_back(instantiation);
            }
            for (const auto &e_comb : Comb(e_literals.size(), 1, this->max_precondition_size - p_comb.size())) {
                std::vector<std::size_t> comb = p_comb;
                for (const auto c : e_comb) comb.push_back(c + p_comb.size());
                const BDD c_valid = Reduce(world, instantiations, literals, comb, valid);
                const BDD c_invalid = Reduce(world, instantiations, literals, comb, invalid);
                const BDD c_applicable = c_valid | c_invalid;
                if (vars->numStates(c_applicable) != 0) {
                    std::vector<Literal> c_literals;
                    for (const auto &i : comb)
                        c_literals.push_back(literals[i]);
                    f(Combination(
                        this->vars->numStates(c_applicable),
                        this->vars->numStates(c_invalid),
                        types,
                        c_literals
                    ));
                }
            }
        }
    }
}
