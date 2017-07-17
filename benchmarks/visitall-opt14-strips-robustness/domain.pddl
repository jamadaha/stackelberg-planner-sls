(define (domain grid-visit-all)
(:requirements :typing)
(:types        place - object)
(:predicates (connected ?x ?y - place)
	     (at-robot ?x - place)
	     (visited ?x - place)
)
	
(:action attack_move
:parameters (?curpos ?nextpos - place)
:precondition (and (at-robot ?curpos) (connected ?curpos ?nextpos))
:effect (and (at-robot ?nextpos) (not (at-robot ?curpos)) (visited ?nextpos))
)

(:action fix_remove_connection
:parameters (?pos1 ?pos2 - place)
:precondition (and (connected ?pos1 ?pos2))
:effect (and (not (connected ?pos1 ?pos2)))
)

)

