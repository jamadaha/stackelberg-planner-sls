;; Basically rovers domain with additional fix actions which can remove connections between waypoints.
;; A connection can be removed if a normal rover can traverse it
;; With a single action, a connection for all rovers and in both directions is removed at once
;; Only connections for which the allowed_to_remove predicate is set, can be removed.
;; The cost for for removing a segment is 1
;; The initial fix rover location is always waypoint0

(define (domain Rover)
(:requirements :typing)
(:types rover waypoint store camera mode lander objective fix_rover)

(:predicates (at ?x - rover ?y - waypoint) 
             (at_lander ?x - lander ?y - waypoint)
             (can_traverse ?r - rover ?x - waypoint ?y - waypoint)
	     (equipped_for_soil_analysis ?r - rover)
             (equipped_for_rock_analysis ?r - rover)
             (equipped_for_imaging ?r - rover)
             (empty ?s - store)
             (have_rock_analysis ?r - rover ?w - waypoint)
             (have_soil_analysis ?r - rover ?w - waypoint)
             (full ?s - store)
	     (calibrated ?c - camera ?r - rover) 
	     (supports ?c - camera ?m - mode)
             (available ?r - rover)
             (visible ?w - waypoint ?p - waypoint)
             (have_image ?r - rover ?o - objective ?m - mode)
             (communicated_soil_data ?w - waypoint)
             (communicated_rock_data ?w - waypoint)
             (communicated_image_data ?o - objective ?m - mode)
	     (at_soil_sample ?w - waypoint)
	     (at_rock_sample ?w - waypoint)
             (visible_from ?o - objective ?w - waypoint)
	     (store_of ?s - store ?r - rover)
	     (calibration_target ?i - camera ?o - objective)
	     (on_board ?i - camera ?r - rover)
	     (channel_free ?l - lander)
       (removed-connection ?loc1 - waypoint ?loc2 - waypoint)
       (allowed_to_remove ?loc1 - waypoint ?loc2 - waypoint)
       (at_fix_rover ?x - fix_rover ?y - waypoint)

)

	
(:action attack_navigate
:parameters (?x - rover ?y - waypoint ?z - waypoint) 
:precondition (and (can_traverse ?x ?y ?z) (available ?x) (at ?x ?y) 
                (visible ?y ?z)
                (not (removed-connection ?y ?z))
	    )
:effect (and (not (at ?x ?y)) (at ?x ?z)
		)
)

(:action attack_sample_soil
:parameters (?x - rover ?s - store ?p - waypoint)
:precondition (and (at ?x ?p) (at_soil_sample ?p) (equipped_for_soil_analysis ?x) (store_of ?s ?x) (empty ?s)
		)
:effect (and (not (empty ?s)) (full ?s) (have_soil_analysis ?x ?p) (not (at_soil_sample ?p))
		)
)

(:action attack_sample_rock
:parameters (?x - rover ?s - store ?p - waypoint)
:precondition (and (at ?x ?p) (at_rock_sample ?p) (equipped_for_rock_analysis ?x) (store_of ?s ?x)(empty ?s)
		)
:effect (and (not (empty ?s)) (full ?s) (have_rock_analysis ?x ?p) (not (at_rock_sample ?p))
		)
)

(:action attack_drop
:parameters (?x - rover ?y - store)
:precondition (and (store_of ?y ?x) (full ?y)
		)
:effect (and (not (full ?y)) (empty ?y)
	)
)

(:action attack_calibrate
 :parameters (?r - rover ?i - camera ?t - objective ?w - waypoint)
 :precondition (and (equipped_for_imaging ?r) (calibration_target ?i ?t) (at ?r ?w) (visible_from ?t ?w)(on_board ?i ?r)
		)
 :effect (calibrated ?i ?r) 
)




(:action attack_take_image
 :parameters (?r - rover ?p - waypoint ?o - objective ?i - camera ?m - mode)
 :precondition (and (calibrated ?i ?r)
			 (on_board ?i ?r)
                      (equipped_for_imaging ?r)
                      (supports ?i ?m)
			  (visible_from ?o ?p)
                     (at ?r ?p)
               )
 :effect (and (have_image ?r ?o ?m)(not (calibrated ?i ?r))
		)
)


(:action attack_communicate_soil_data
 :parameters (?r - rover ?l - lander ?p - waypoint ?x - waypoint ?y - waypoint)
 :precondition (and (at ?r ?x)(at_lander ?l ?y)(have_soil_analysis ?r ?p) 
                   (visible ?x ?y)(available ?r)(channel_free ?l)
            )
 :effect (and (not (available ?r))(not (channel_free ?l))(channel_free ?l)
		(communicated_soil_data ?p)(available ?r)
	)
)

(:action attack_communicate_rock_data
 :parameters (?r - rover ?l - lander ?p - waypoint ?x - waypoint ?y - waypoint)
 :precondition (and (at ?r ?x)(at_lander ?l ?y)(have_rock_analysis ?r ?p)
                   (visible ?x ?y)(available ?r)(channel_free ?l)
            )
 :effect (and (not (available ?r))(not (channel_free ?l))(channel_free ?l)(communicated_rock_data ?p)(available ?r)
          )
)


(:action attack_communicate_image_data
 :parameters (?r - rover ?l - lander ?o - objective ?m - mode ?x - waypoint ?y - waypoint)
 :precondition (and (at ?r ?x)(at_lander ?l ?y)(have_image ?r ?o ?m)(visible ?x ?y)(available ?r)(channel_free ?l)
            )
 :effect (and (not (available ?r))(not (channel_free ?l))(channel_free ?l)(communicated_image_data ?o ?m)(available ?r)
          )
)


(:action fix_navigate
:parameters (?x - fix_rover ?y - waypoint ?z - waypoint) 
:precondition (and (at_fix_rover ?x ?y) 
                (visible ?y ?z)
                (not (removed-connection ?y ?z))
      )
:effect (and (increase (total-cost) 1) (not (at_fix_rover ?x ?y)) (at_fix_rover ?x ?z)
    )
)

(:action fix_remove_connection_1
:parameters (?x - fix_rover ?y - waypoint ?z - waypoint) 
:precondition (and (at_fix_rover ?x ?y) 
                (visible ?y ?z)
                (allowed_to_remove ?y ?z)
                (not (removed-connection ?y ?z))
                (not (removed-connection ?z ?y))
      )
:effect (and (increase (total-cost) 1)
             (removed-connection ?y ?z)
             (removed-connection ?z ?y)
    )
)

)
