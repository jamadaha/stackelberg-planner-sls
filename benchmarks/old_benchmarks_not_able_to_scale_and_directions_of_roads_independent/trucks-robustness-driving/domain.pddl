; IPC5 Domain: Trucks Propositional
; Authors: Yannis Dimopoulos, Alfonso Gerevini and Alessandro Saetti 

;; Basically trucks domain with additional fix truck driving on the map which can remove a connection if it is in an adjacent location.
;; The cost for this truck to drive is currently 1 and for removing a connection it is also 1
;; The initial location of the fix truck always is l1
;; In contrast to normal trucks, fix truck actions do not consume time

(define (domain Trucks) 
(:requirements :typing :adl)  

(:types truckarea time location locatable - object
        truck fix_truck package - locatable) 

(:predicates (at ?x - locatable ?l - location) 	     
             (in ?p - package ?t - truck ?a - truckarea) 	     
             (connected ?x ?y - location)
             (free ?a - truckarea ?t - truck)
  	     (time-now ?t - time)
 	     (next ?t1 - time ?t2 - time)
	     (le ?t1 - time ?t2 - time)
 	     (delivered ?p - package ?l - location ?t - time)
	     (at-destination ?p - package ?l - location)
 	     (closer ?a1 - truckarea ?a2 - truckarea))

(:action attack_load
 :parameters (?p - package ?t - truck ?a1 - truckarea ?l - location)
 :precondition (and (at ?t ?l) (at ?p ?l) (free ?a1 ?t)
  		    (forall (?a2 - truckarea)
  			    (imply (closer ?a2 ?a1) (free ?a2 ?t))))
 :effect (and (not (at ?p ?l)) (not (free ?a1 ?t)) (in ?p ?t ?a1)))

(:action attack_unload
 :parameters (?p - package ?t - truck ?a1 - truckarea ?l - location)
 :precondition (and (at ?t ?l) (in ?p ?t ?a1)
  		    (forall (?a2 - truckarea)
  			    (imply (closer ?a2 ?a1) (free ?a2 ?t))))
 :effect (and (not (in ?p ?t ?a1)) (free ?a1 ?t) (at ?p ?l)))

(:action attack_drive
 :parameters (?t - truck ?from ?to - location ?t1 ?t2 - time)
 :precondition (and (at ?t ?from) (connected ?from ?to) 
		    (time-now ?t1) (next ?t1 ?t2))
 :effect (and (not (at ?t ?from)) (not (time-now ?t1)) 
	      (time-now ?t2) (at ?t ?to)))

(:action attack_deliver
 :parameters (?p - package ?l - location ?t1 ?t2 - time)
 :precondition (and (at ?p ?l) (time-now ?t1) (le ?t1 ?t2))
 :effect (and (not (at ?p ?l)) (delivered ?p ?l ?t2) (at-destination ?p ?l)))


(:action fix_drive
 :parameters (?t - fix_truck ?from ?to - location)
 :precondition (and (at ?t ?from) (connected ?from ?to) )
 :effect (and (not (at ?t ?from)) (at ?t ?to)))

 (:action fix_remove_connection
 :parameters (?t - fix_truck ?from ?to - location)
 :precondition (and (at ?t ?from) (connected ?from ?to) )
 :effect (and (not (connected ?from ?to)) ))

) 

