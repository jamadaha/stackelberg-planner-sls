(define (domain 3)
(:requirements :strips :probabilistic-effects)
(:functions (total-cost))
(:constants
    user2_0
    clientapplication
    serverapplicationprotocol
    node1_0
    node3_0
    user
    node2_0
    serverapplicationport
    root
    action0
    action1
    action2
    action3
    action4
    action5
    action6
)
(:predicates
    (accessmaliciousinput ?x0 ?x1 ?x2)
    (netaccess ?x0 ?x1 ?x2)
    (execcode ?x0 ?x1)
    (applyonce ?x0)
)
(:action RULE_4_0
    :parameters ()
    :precondition (and
        (execcode node3_0 root)
        (applyonce action0)
    )
    :effect (and (increase (total-cost) 69)  (not (applyonce action0))(probabilistic 1.00000 (and
        (netaccess node1_0 serverapplicationprotocol serverapplicationport)
    )))
)
(:action RULE_1_0
    :parameters ()
    :precondition (and
        (netaccess node1_0 serverapplicationprotocol serverapplicationport)
        (applyonce action1)
    )
    :effect (and (increase (total-cost) 54)  (not (applyonce action1))(probabilistic 0.13000 (and
        (execcode node1_0 root)
    )))
)
(:action RULE_1_1
    :parameters ()
    :precondition (and
        (netaccess node3_0 serverapplicationprotocol serverapplicationport)
        (applyonce action2)
    )
    :effect (and (increase (total-cost) 17)  (not (applyonce action2))(probabilistic 0.73000 (and
        (execcode node3_0 root)
    )))
)
(:action RULE_4_1
    :parameters ()
    :precondition (and
        (execcode node2_0 user)
        (applyonce action3)
    )
    :effect (and (increase (total-cost) 4)  (not (applyonce action3))(probabilistic 1.00000 (and
        (netaccess node1_0 serverapplicationprotocol serverapplicationport)
    )))
)
(:action RULE_5_0
    :parameters ()
    :precondition (and
        (applyonce action4)
    )
    :effect (and (increase (total-cost) 1)  (not (applyonce action4))(probabilistic 1.00000 (and
        (netaccess node3_0 serverapplicationprotocol serverapplicationport)
    )))
)
(:action RULE_2_0
    :parameters ()
    :precondition (and
        (accessmaliciousinput node2_0 user2_0 clientapplication)
        (applyonce action5)
    )
    :effect (and (increase (total-cost) 6)  (not (applyonce action5))(probabilistic 0.34000 (and
        (execcode node2_0 user)
    )))
)
(:action RULE_20_0
    :parameters ()
    :precondition (and
        (applyonce action6)
    )
    :effect (and (increase (total-cost) 1)  (not (applyonce action6))(probabilistic 0.78000 (and
        (accessmaliciousinput node2_0 user2_0 clientapplication)
    )))
)
)
