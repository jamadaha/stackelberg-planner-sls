;; Basically no-mystery domain with fix actions to remove connections
;; A connection is always removed in both directions at once. Only connections for which the allowed_to_remove predicate is set, can be removed.
;; The cost for removing a segment is also 1.

(define (domain transport-strips)
(:requirements :typing :action-costs)

(:types location fuellevel locatable - object 
	package truck fix_truck - locatable
)

(:predicates 
(connected ?l1 ?l2 - location)
(at ?o - locatable ?l - location)
(in ?p - package ?t - truck)
(fuel ?t - truck ?level - fuellevel)
(fuelcost ?level - fuellevel ?l1 ?l2 - location)
(sum ?a ?b ?c - fuellevel)
(allowed_to_remove ?loc1 ?loc2 - location)
)

(:functions 
(total-cost) - number)

(:action attack_LOAD
:parameters
(?p - package
?t - truck
?l - location)
:precondition
(and (at ?t ?l) (at ?p ?l))
:effect
(and (not (at ?p ?l)) (in ?p ?t) (increase (total-cost) 1))
)

(:action attack_UNLOAD
:parameters
(?p - package
?t - truck
?l - location)
:precondition
(and (at ?t ?l) (in ?p ?t))
:effect
(and (at ?p ?l) (not (in ?p ?t)) (increase (total-cost) 1))
)

(:action attack_DRIVE
:parameters
(?t - truck
?l1 - location
?l2 - location
?fuelpost - fuellevel
?fueldelta - fuellevel
?fuelpre - fuellevel)
:precondition
(and 
(connected ?l1 ?l2)
(fuelcost ?fueldelta ?l1 ?l2)
(fuel ?t ?fuelpre)
(sum ?fuelpost ?fueldelta ?fuelpre)
(at ?t ?l1)
)
:effect
(and (not (at ?t ?l1)) 
     (at ?t ?l2) 
     (not (fuel ?t ?fuelpre)) 
     (fuel ?t ?fuelpost)
     (increase (total-cost) 1))
)

(:action fix_remove_road
       :parameters (?l1 ?l2 - location)
       :precondition (and
        (connected ?l1 ?l2)
        (connected ?l2 ?l1)
        (allowed_to_remove ?l1 ?l2))
       :effect (and (increase (total-cost) 1) 
                    (not (connected ?l1 ?l2))
                    (not (connected ?l2 ?l1))
                    ))

)


