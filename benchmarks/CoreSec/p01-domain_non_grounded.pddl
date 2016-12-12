;;; H=10 E=[1, 1, 8] T=[1, 1, 11] Sub=3 1-per=40 Seed=324241(2140687749857675880)
(define (domain CoreSec)
(:requirements :strips)

(:types host port operatingsystem osedition osversion service serviceversion)

(:predicates
    (target_under_control)
    (hacl ?src ?dest - host ?p - port)
    (controlling ?h - host)
    (has_OS ?h - host ?os - operatingsystem ?e - osedition ?v - osversion)
    (has_service ?h - host ?s - service ?v - serviceversion)
)

(:functions (total-cost))

(:constants
    host0 host1 host2 host3 host4 host5 host6 host7 host8 host9 internet - host
    p80 p3306 p445 - port
    winNT4 win2000 - operatingsystem
    ent ser adv - osedition
    1 2 3 - osversion
    1 2 3 - serviceversion
)

(:action ATTACK_ex-host0_winNT4ser_IIS4_vuln7_17_100
    :parameters (?src - host ?t - host)
    :precondition (and (hacl ?src host0 p80) (controlling ?src) (has_OS ?t winNT4 ser 3) (has_service ?t IIS4 3))
    :effect (and (increase (total-cost) 10) (probabilistic 17/100 (and (controlling ?t))))
)

(:action ATTACK_ex-host1_winNT4ser_IIS4_vuln0_47_100
    :parameters (?src - host ?t - host)
    :precondition (and (hacl ?src host1 p80) (controlling ?src) (has_OS ?t winNT4 ser 1) (has_service ?t IIS4 1))
    :effect (and (increase (total-cost) 5) (and (controlling ?t)))
)

(:action ATTACK_ex-host2_winNT4ser_IIS4_vuln15_73_100
    :parameters (?src - host ?t - host)
    :precondition (and (hacl ?src host2 p80) (controlling ?src) (has_OS ?t winNT4 ser 2) (has_service ?t IIS4 2))
    :effect (and (increase (total-cost) 5) (and (controlling ?t)))
)

(:action ATTACK_ex-host3_winNT4ent_SQL4_vuln60_69_100
    :parameters (?src - host ?t - host)
    :precondition (and (hacl ?src host3 p3306) (controlling ?src) (has_OS ?t winNT4 ent 1) (has_service ?t SQL4 1))
    :effect (and (increase (total-cost) 3) (and (controlling ?t)))
)

(:action ATTACK_ex-host4_win2000ser_MSRPC5_vuln23_64_100
    :parameters (?src - host ?t - host ?osv - osversion ?sv - serviceversion)
    :precondition (and (hacl ?src host4 p445) (controlling ?src) (has_OS ?t win2000 ser ?osv) (has_service ?t MSRPC5 ?sv))
    :effect (and (increase (total-cost) 3) (and (controlling ?t)))
)

(:action ATTACK_ex-host5_win2000adv_IIS5_vuln25_71_100
    :parameters (?src - host ?t - host)
    :precondition (and (hacl ?src host5 p80) (controlling ?src) (has_OS ?t win2000 adv 2) (has_service ?t IIS5 2))
    :effect (and (increase (total-cost) 5) (and (controlling ?t)))
)

(:action ATTACK_ex-host6_winNT4ser_SQL4_vuln15_84_100
    :parameters (?src - host ?t - host ?osv - osversion ?sv - serviceversion)
    :precondition (and (hacl ?src host6 p3306) (controlling ?src) (has_OS ?t winNT4 ser ?osv) (has_service ?t SQL4 ?sv))
    :effect (and (increase (total-cost) 4) (and (controlling ?t)))
)

(:action ATTACK_ex-host7_win2000adv_MSRPC5_vuln61_74_100
    :parameters (?src - host ?t - host ?osv - osversion ?sv - serviceversion)
    :precondition (and (hacl ?src host7 p445) (controlling ?src) (has_OS ?t win2000 adv ?osv) (has_service ?t MSRPC5 ?sv))
    :effect (and (increase (total-cost) 2) (and (controlling ?t)))
)

(:action ATTACK_ex-host8_win2000adv_IIS5_vuln89_82_100
    :parameters (?src - host ?t - host)
    :precondition (and (hacl ?src host8 p80) (controlling ?src) (has_OS ?t win2000 adv 1) (has_service ?t IIS5 1))
    :effect (and (increase (total-cost) 1) (and (controlling ?t)))
)

(:action ATTACK_ex-host9_winNT4ent_SQL4_vuln24_82_100
    :parameters (?src - host ?t - host)
    :precondition (and (hacl ?src host9 p3306) (controlling ?src) (has_OS ?t winNT4 ent 2) (has_service ?t SQL4 2))
    :effect (and (increase (total-cost) 3) (and (controlling ?t)))
)

)
