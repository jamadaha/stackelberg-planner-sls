;;; H=10 E=[1, 1, 8] T=[1, 1, 11] Sub=3 1-per=40 Seed=324241(2140687749857675880)
(define (domain CoreSec)
(:requirements :strips :probabilistic-effects)

(:types host port)

(:predicates
    (target_under_control)
    (hacl ?src ?dest - host ?p - port)
    (controlling ?h - host)
    (expl_host3_winNT4ent_SQL4_vuln60)
    (expl_host1_winNT4ser_IIS4_vuln0)
    (expl_host8_win2000adv_IIS5_vuln89)
    (expl_host7_win2000adv_MSRPC5_vuln61)
    (expl_host6_winNT4ser_SQL4_vuln15)
    (expl_host2_winNT4ser_IIS4_vuln15)
    (expl_host5_win2000adv_IIS5_vuln25)
    (expl_host4_win2000ser_MSRPC5_vuln23)
    (expl_host0_winNT4ser_IIS4_vuln7)
    (expl_host9_winNT4ent_SQL4_vuln24)
)

(:functions (total-cost))

(:constants
    host0 host1 host2 host3 host4 host5 host6 host7 host8 host9 internet - host
    p80 p3306 p445 - port
)

(:action ex-host0_winNT4ser_IIS4_vuln7
    :parameters (?src - host)
    :precondition (and (hacl ?src host0 p80) (controlling ?src) (expl_host0_winNT4ser_IIS4_vuln7))
    :effect (and (increase (total-cost) 10) (not (expl_host0_winNT4ser_IIS4_vuln7)) (probabilistic 17/100 (and (controlling host0))))
)

(:action ex-host1_winNT4ser_IIS4_vuln0
    :parameters (?src - host)
    :precondition (and (hacl ?src host1 p80) (controlling ?src) (expl_host1_winNT4ser_IIS4_vuln0))
    :effect (and (increase (total-cost) 5) (not (expl_host1_winNT4ser_IIS4_vuln0)) (probabilistic 47/100 (and (controlling host1))))
)

(:action ex-host2_winNT4ser_IIS4_vuln15
    :parameters (?src - host)
    :precondition (and (hacl ?src host2 p80) (controlling ?src) (expl_host2_winNT4ser_IIS4_vuln15))
    :effect (and (increase (total-cost) 5) (not (expl_host2_winNT4ser_IIS4_vuln15)) (probabilistic 73/100 (and (controlling host2))))
)

(:action ex-host3_winNT4ent_SQL4_vuln60
    :parameters (?src - host)
    :precondition (and (hacl ?src host3 p3306) (controlling ?src) (expl_host3_winNT4ent_SQL4_vuln60))
    :effect (and (increase (total-cost) 3) (not (expl_host3_winNT4ent_SQL4_vuln60)) (probabilistic 69/100 (and (controlling host3))))
)

(:action ex-host4_win2000ser_MSRPC5_vuln23
    :parameters (?src - host)
    :precondition (and (hacl ?src host4 p445) (controlling ?src) (expl_host4_win2000ser_MSRPC5_vuln23))
    :effect (and (increase (total-cost) 3) (not (expl_host4_win2000ser_MSRPC5_vuln23)) (probabilistic 64/100 (and (controlling host4))))
)

(:action ex-host5_win2000adv_IIS5_vuln25
    :parameters (?src - host)
    :precondition (and (hacl ?src host5 p80) (controlling ?src) (expl_host5_win2000adv_IIS5_vuln25))
    :effect (and (increase (total-cost) 5) (not (expl_host5_win2000adv_IIS5_vuln25)) (probabilistic 71/100 (and (controlling host5))))
)

(:action ex-host6_winNT4ser_SQL4_vuln15
    :parameters (?src - host)
    :precondition (and (hacl ?src host6 p3306) (controlling ?src) (expl_host6_winNT4ser_SQL4_vuln15))
    :effect (and (increase (total-cost) 4) (not (expl_host6_winNT4ser_SQL4_vuln15)) (probabilistic 84/100 (and (controlling host6))))
)

(:action ex-host7_win2000adv_MSRPC5_vuln61
    :parameters (?src - host)
    :precondition (and (hacl ?src host7 p445) (controlling ?src) (expl_host7_win2000adv_MSRPC5_vuln61))
    :effect (and (increase (total-cost) 2) (not (expl_host7_win2000adv_MSRPC5_vuln61)) (probabilistic 74/100 (and (controlling host7))))
)

(:action ex-host8_win2000adv_IIS5_vuln89
    :parameters (?src - host)
    :precondition (and (hacl ?src host8 p80) (controlling ?src) (expl_host8_win2000adv_IIS5_vuln89))
    :effect (and (increase (total-cost) 1) (not (expl_host8_win2000adv_IIS5_vuln89)) (probabilistic 82/100 (and (controlling host8))))
)

(:action ex-host9_winNT4ent_SQL4_vuln24
    :parameters (?src - host)
    :precondition (and (hacl ?src host9 p3306) (controlling ?src) (expl_host9_winNT4ent_SQL4_vuln24))
    :effect (and (increase (total-cost) 3) (not (expl_host9_winNT4ent_SQL4_vuln24)) (probabilistic 82/100 (and (controlling host9))))
)

)
