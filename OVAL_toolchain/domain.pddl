;;; Bla
(define (domain coresec)
; (:requirements strips)
(:types host zone port protocol vul compromised_type probability)
(:predicates
    (target0_under_control)
    (compromised ?h - host ?ct - compromised_type)
    (haclz ?src ?dest - zone ?po - port ?pr - protocol)
    (subnet ?z - zone ?h - host)
    (vul_exists ?v - vul ?h - host ?po - port ?pr - protocol ?prob - probability)
    (applied ?v - vul ?h - host)
)
(:functions (total-cost))
(:constants
    10.9.59.118 internet - host
    p64704 p63496 p60988 p60165 p59667 p57697 p55296 p54544 p53966 p53686 p50503 p8834 p5353 p138 p137 p123 pnone - port
    CVE-2017-2925 CVE-2017-2926 CVE-2017-2927 CVE-2017-2928 CVE-2017-2930 CVE-2017-2931 CVE-2017-2932 CVE-2017-2933 CVE-2017-2934 CVE-2017-2935 CVE-2017-2936 CVE-2017-2937 CVE-2017-2938 - vul
    confidentiality integrity availability - compromised_type
    tcp - protocol
    prob0.4 prob0.3 - probability
    subnet1 subnet2 - zone
)
(:action ATTACK_set_target0_under_control_10.9.59.118_1/1
    :parameters ()
    :precondition (and (not (target0_under_control)) (compromised 10.9.59.118 integrity))
    :effect (and (target0_under_control) (increase (total-cost) 0))
)
(:action ATTACK_exploit_CVE-2017-2925_2/5
    :parameters (?src ?t - host ?z1 ?z2 - zone ?po - port ?pr - protocol)
    :precondition (and (compromised ?src integrity) (subnet ?z1 ?src) (subnet ?z2 ?t) (haclz ?z1 ?z2 ?po ?pr) (vul_exists CVE-2017-2925 ?t ?po ?pr prob0.4) (not (applied CVE-2017-2925 ?t)))
    :effect (and (increase (total-cost) 1) (compromised ?t confidentiality) (compromised ?t integrity) (compromised ?t availability) (applied CVE-2017-2925 ?t))
)
(:action ATTACK_exploit_CVE-2017-2926_2/5
    :parameters (?src ?t - host ?z1 ?z2 - zone ?po - port ?pr - protocol)
    :precondition (and (compromised ?src integrity) (subnet ?z1 ?src) (subnet ?z2 ?t) (haclz ?z1 ?z2 ?po ?pr) (vul_exists CVE-2017-2926 ?t ?po ?pr prob0.4) (not (applied CVE-2017-2926 ?t)))
    :effect (and (increase (total-cost) 1) (compromised ?t confidentiality) (compromised ?t integrity) (compromised ?t availability) (applied CVE-2017-2926 ?t))
)
(:action ATTACK_exploit_CVE-2017-2927_2/5
    :parameters (?src ?t - host ?z1 ?z2 - zone ?po - port ?pr - protocol)
    :precondition (and (compromised ?src integrity) (subnet ?z1 ?src) (subnet ?z2 ?t) (haclz ?z1 ?z2 ?po ?pr) (vul_exists CVE-2017-2927 ?t ?po ?pr prob0.4) (not (applied CVE-2017-2927 ?t)))
    :effect (and (increase (total-cost) 1) (compromised ?t confidentiality) (compromised ?t integrity) (compromised ?t availability) (applied CVE-2017-2927 ?t))
)
(:action ATTACK_exploit_CVE-2017-2928_2/5
    :parameters (?src ?t - host ?z1 ?z2 - zone ?po - port ?pr - protocol)
    :precondition (and (compromised ?src integrity) (subnet ?z1 ?src) (subnet ?z2 ?t) (haclz ?z1 ?z2 ?po ?pr) (vul_exists CVE-2017-2928 ?t ?po ?pr prob0.4) (not (applied CVE-2017-2928 ?t)))
    :effect (and (increase (total-cost) 1) (compromised ?t confidentiality) (compromised ?t integrity) (compromised ?t availability) (applied CVE-2017-2928 ?t))
)
(:action ATTACK_exploit_CVE-2017-2930_2/5
    :parameters (?src ?t - host ?z1 ?z2 - zone ?po - port ?pr - protocol)
    :precondition (and (compromised ?src integrity) (subnet ?z1 ?src) (subnet ?z2 ?t) (haclz ?z1 ?z2 ?po ?pr) (vul_exists CVE-2017-2930 ?t ?po ?pr prob0.4) (not (applied CVE-2017-2930 ?t)))
    :effect (and (increase (total-cost) 1) (compromised ?t confidentiality) (compromised ?t integrity) (compromised ?t availability) (applied CVE-2017-2930 ?t))
)
(:action ATTACK_exploit_CVE-2017-2931_2/5
    :parameters (?src ?t - host ?z1 ?z2 - zone ?po - port ?pr - protocol)
    :precondition (and (compromised ?src integrity) (subnet ?z1 ?src) (subnet ?z2 ?t) (haclz ?z1 ?z2 ?po ?pr) (vul_exists CVE-2017-2931 ?t ?po ?pr prob0.4) (not (applied CVE-2017-2931 ?t)))
    :effect (and (increase (total-cost) 1) (compromised ?t confidentiality) (compromised ?t integrity) (compromised ?t availability) (applied CVE-2017-2931 ?t))
)
(:action ATTACK_exploit_CVE-2017-2932_2/5
    :parameters (?src ?t - host ?z1 ?z2 - zone ?po - port ?pr - protocol)
    :precondition (and (compromised ?src integrity) (subnet ?z1 ?src) (subnet ?z2 ?t) (haclz ?z1 ?z2 ?po ?pr) (vul_exists CVE-2017-2932 ?t ?po ?pr prob0.4) (not (applied CVE-2017-2932 ?t)))
    :effect (and (increase (total-cost) 1) (compromised ?t confidentiality) (compromised ?t integrity) (compromised ?t availability) (applied CVE-2017-2932 ?t))
)
(:action ATTACK_exploit_CVE-2017-2933_2/5
    :parameters (?src ?t - host ?z1 ?z2 - zone ?po - port ?pr - protocol)
    :precondition (and (compromised ?src integrity) (subnet ?z1 ?src) (subnet ?z2 ?t) (haclz ?z1 ?z2 ?po ?pr) (vul_exists CVE-2017-2933 ?t ?po ?pr prob0.4) (not (applied CVE-2017-2933 ?t)))
    :effect (and (increase (total-cost) 1) (compromised ?t confidentiality) (compromised ?t integrity) (compromised ?t availability) (applied CVE-2017-2933 ?t))
)
(:action ATTACK_exploit_CVE-2017-2934_2/5
    :parameters (?src ?t - host ?z1 ?z2 - zone ?po - port ?pr - protocol)
    :precondition (and (compromised ?src integrity) (subnet ?z1 ?src) (subnet ?z2 ?t) (haclz ?z1 ?z2 ?po ?pr) (vul_exists CVE-2017-2934 ?t ?po ?pr prob0.4) (not (applied CVE-2017-2934 ?t)))
    :effect (and (increase (total-cost) 1) (compromised ?t confidentiality) (compromised ?t integrity) (compromised ?t availability) (applied CVE-2017-2934 ?t))
)
(:action ATTACK_exploit_CVE-2017-2935_2/5
    :parameters (?src ?t - host ?z1 ?z2 - zone ?po - port ?pr - protocol)
    :precondition (and (compromised ?src integrity) (subnet ?z1 ?src) (subnet ?z2 ?t) (haclz ?z1 ?z2 ?po ?pr) (vul_exists CVE-2017-2935 ?t ?po ?pr prob0.4) (not (applied CVE-2017-2935 ?t)))
    :effect (and (increase (total-cost) 1) (compromised ?t confidentiality) (compromised ?t integrity) (compromised ?t availability) (applied CVE-2017-2935 ?t))
)
(:action ATTACK_exploit_CVE-2017-2936_2/5
    :parameters (?src ?t - host ?z1 ?z2 - zone ?po - port ?pr - protocol)
    :precondition (and (compromised ?src integrity) (subnet ?z1 ?src) (subnet ?z2 ?t) (haclz ?z1 ?z2 ?po ?pr) (vul_exists CVE-2017-2936 ?t ?po ?pr prob0.4) (not (applied CVE-2017-2936 ?t)))
    :effect (and (increase (total-cost) 1) (compromised ?t confidentiality) (compromised ?t integrity) (compromised ?t availability) (applied CVE-2017-2936 ?t))
)
(:action ATTACK_exploit_CVE-2017-2937_2/5
    :parameters (?src ?t - host ?z1 ?z2 - zone ?po - port ?pr - protocol)
    :precondition (and (compromised ?src integrity) (subnet ?z1 ?src) (subnet ?z2 ?t) (haclz ?z1 ?z2 ?po ?pr) (vul_exists CVE-2017-2937 ?t ?po ?pr prob0.4) (not (applied CVE-2017-2937 ?t)))
    :effect (and (increase (total-cost) 1) (compromised ?t confidentiality) (compromised ?t integrity) (compromised ?t availability) (applied CVE-2017-2937 ?t))
)
(:action ATTACK_exploit_CVE-2017-2938_2/5
    :parameters (?src ?t - host ?z1 ?z2 - zone ?po - port ?pr - protocol)
    :precondition (and (compromised ?src integrity) (subnet ?z1 ?src) (subnet ?z2 ?t) (haclz ?z1 ?z2 ?po ?pr) (vul_exists CVE-2017-2938 ?t ?po ?pr prob0.4) (not (applied CVE-2017-2938 ?t)))
    :effect (and (increase (total-cost) 1) (compromised ?t confidentiality) (compromised ?t integrity) (compromised ?t availability) (applied CVE-2017-2938 ?t))
)
(:action ATTACK_exploit_CVE-2017-2938_3/10
    :parameters (?src ?t - host ?z1 ?z2 - zone ?po - port ?pr - protocol)
    :precondition (and (compromised ?src integrity) (subnet ?z1 ?src) (subnet ?z2 ?t) (haclz ?z1 ?z2 ?po ?pr) (vul_exists CVE-2017-2938 ?t ?po ?pr prob0.3) (not (applied CVE-2017-2938 ?t)))
    :effect (and (increase (total-cost) 1) (compromised ?t confidentiality) (compromised ?t integrity) (compromised ?t availability) (applied CVE-2017-2938 ?t))
)
(:action FIX_exploit_prob0.0_1#0
    :parameters (?v - vul ?h - host ?po - port ?pr - protocol ?old_prob - probability)
    :precondition (and (vul_exists ?v ?h ?po ?pr ?old_prob))
    :effect (and (increase (total-cost) 1) (not (vul_exists ?v ?h ?po ?pr ?old_prob)))
)
(:action FIX_exploit_CVE-2017-2938_prob0.3_1#1
    :parameters (?h - host ?po - port ?pr - protocol ?old_prob - probability)
    :precondition (and (vul_exists CVE-2017-2938 ?h ?po ?pr ?old_prob))
    :effect (and (increase (total-cost) 1) (not (vul_exists CVE-2017-2938 ?h ?po ?pr ?old_prob)) (vul_exists CVE-2017-2938 ?h ?po ?pr prob0.3))
)
(:action FIX_install_firewall_1#2
    :parameters (?src - zone ?dest - zone ?po - port ?pr - protocol)
    :precondition (and (haclz ?src ?dest ?po ?pr))
    :effect (and (increase (total-cost) 1) (not (haclz ?src ?dest ?po ?pr)))
)
)
