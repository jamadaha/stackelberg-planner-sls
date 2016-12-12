;;; H=10 E=[1, 1, 8] T=[1, 1, 11] Sub=3 1-per=40 Seed=324241(2140687749857675880)
(define (domain CoreSec_non_grounded)
(:requirements :strips)

(:types host port operatingsystem osversion service serviceversion)

(:predicates
    (target_under_control)
    (hacl ?src ?dest - host ?p - port)
    (controlling ?h - host)
    (has_OS ?h - host ?os - operatingsystem)
    (has_OS_version ?h - host ?v - osversion)
    (has_service ?h - host ?s - service)
    (has_service_version ?h - host ?s - service ?v - serviceversion)
    (os_upgrade ?os - operatingsystem ?osv1 - osversion ?osv2 - osversion)
    (service_upgrade ?s - service ?sv1 - serviceversion ?sv2 - serviceversion)
)

(:functions (total-cost))

(:constants
    host0 host1 host2 host3 host4 host5 host6 host7 host8 host9 internet - host
    p80 p3306 p445 - port
    winNT4_ser winNT4_ent win2000_ser win2000_adv - operatingsystem
    osv1 osv2 osv3 - osversion
    IIS4 IIS5 SQL4 MSRPC5 - service
    sv1 sv2 sv3 - serviceversion
)

(:action ATTACK_ex-host0_winNT4ser_IIS4_vuln7_17_100
    :parameters (?src - host ?t - host)
    :precondition (and (hacl ?src ?t p80) (controlling ?src) (has_OS ?t winNT4_ser) (has_OS_version ?t osv3) (has_service ?t IIS4) (has_service_version ?t IIS4 sv3))
    :effect (and (increase (total-cost) 10) (and (controlling ?t)))
)

(:action ATTACK_ex-host1_winNT4ser_IIS4_vuln0_47_100
    :parameters (?src - host ?t - host)
    :precondition (and (hacl ?src ?t p80) (controlling ?src) (has_OS ?t winNT4_ser) (has_OS_version ?t osv1) (has_service ?t IIS4) (has_service_version ?t IIS4 sv1))
    :effect (and (increase (total-cost) 5) (and (controlling ?t)))
)

(:action ATTACK_ex-host2_winNT4ser_IIS4_vuln15_73_100
    :parameters (?src - host ?t - host)
    :precondition (and (hacl ?src ?t p80) (controlling ?src) (has_OS ?t winNT4_ser) (has_OS_version ?t osv2) (has_service ?t IIS4) (has_service_version ?t IIS4 sv2))
    :effect (and (increase (total-cost) 5) (and (controlling ?t)))
)

(:action ATTACK_ex-host3_winNT4ent_SQL4_vuln60_69_100
    :parameters (?src - host ?t - host)
    :precondition (and (hacl ?src ?t p3306) (controlling ?src) (has_OS ?t winNT4_ent) (has_OS_version ?t osv1) (has_service ?t SQL4) (has_service_version ?t SQL4 sv1))
    :effect (and (increase (total-cost) 3) (and (controlling ?t)))
)

(:action ATTACK_ex-host4_win2000ser_MSRPC5_vuln23_64_100
    :parameters (?src - host ?t - host)
    :precondition (and (hacl ?src ?t p445) (controlling ?src) (has_OS ?t win2000_ser) (has_service ?t MSRPC5))
    :effect (and (increase (total-cost) 3) (and (controlling ?t)))
)

(:action ATTACK_ex-host5_win2000adv_IIS5_vuln25_71_100
    :parameters (?src - host ?t - host)
    :precondition (and (hacl ?src ?t p80) (controlling ?src) (has_OS ?t win2000_adv) (has_OS_version ?t osv2) (has_service ?t IIS5) (has_service_version ?t IIS5 sv2))
    :effect (and (increase (total-cost) 5) (and (controlling ?t)))
)

(:action ATTACK_ex-host6_winNT4ser_SQL4_vuln15_84_100
    :parameters (?src - host ?t - host)
    :precondition (and (hacl ?src ?t p3306) (controlling ?src) (has_OS ?t winNT4_ser) (has_service ?t SQL4))
    :effect (and (increase (total-cost) 4) (and (controlling ?t)))
)

(:action ATTACK_ex-host7_win2000adv_MSRPC5_vuln61_74_100
    :parameters (?src - host ?t - host)
    :precondition (and (hacl ?src ?t p445) (controlling ?src) (has_OS ?t win2000_adv) (has_service ?t MSRPC5))
    :effect (and (increase (total-cost) 2) (and (controlling ?t)))
)

(:action ATTACK_ex-host8_win2000adv_IIS5_vuln89_82_100
    :parameters (?src - host ?t - host)
    :precondition (and (hacl ?src ?t p80) (controlling ?src) (has_OS ?t win2000_adv) (has_OS_version ?t osv1) (has_service ?t IIS5) (has_service_version ?t IIS5 sv1))
    :effect (and (increase (total-cost) 1) (and (controlling ?t)))
)

(:action ATTACK_ex-host9_winNT4ent_SQL4_vuln24_82_100
    :parameters (?src - host ?t - host)
    :precondition (and (hacl ?src ?t p3306) (controlling ?src) (has_OS ?t winNT4_ent) (has_OS_version ?t osv2) (has_service ?t SQL4) (has_service_version ?t SQL4 sv2))
    :effect (and (increase (total-cost) 3) (and (controlling ?t)))
)

(:action FIX_install_firewall_0
    :parameters (?src ?dest - host ?p - port)
    :precondition (and (hacl ?src ?dest ?p))
    :effect (and (increase (total-cost) 1) (and (not (hacl ?src ?dest ?p))))
)

(:action FIX_upgrade_os_5
    :parameters (?h - host ?os - operatingsystem ?osv1 - osversion ?osv2 - osversion)
    :precondition (and (has_OS ?h ?os) (has_OS_version ?h ?osv1) (os_upgrade ?os ?osv1 ?osv2))
    :effect (and (increase (total-cost) 1) (and (not (has_OS_version ?h ?osv1)) (has_OS_version ?h ?osv2)))
)

(:action FIX_upgrade_service_5
    :parameters (?h - host ?s - service ?sv1 - serviceversion ?sv2 - serviceversion)
    :precondition (and (has_service ?h ?s) (has_service_version ?h ?s ?sv1) (service_upgrade ?s ?sv1 ?sv2))
    :effect (and (increase (total-cost) 1) (and (not (has_service_version ?h ?s ?sv1)) (has_service_version ?h ?s ?sv2)))
)

)
