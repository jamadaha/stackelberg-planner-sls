; IPC5 Domain: TPP Propositional
; Authors: Alfonso Gerevini and Alessandro Saetti 

;; Basically tpp domain with additional fix truck driving on the map which can remove a connection in both directions at once if it is in an adjacent place.
;; The cost for this truck to drive is currently 1 and for removing a connection it is also 1
;; Only connections for which the allowed_to_remove predicate is set, can be removed.
;; The initial location of the fix truck always is depot1
;; THESE INITIAL LOCATION IS PROBABLY NOT PERFECTLY CHOSEN

(define (domain TPP-Propositional)
(:requirements :strips :typing)
(:types place locatable level - object
	depot market - place
	truck fix_truck goods - locatable)

(:predicates (loaded ?g - goods ?t - truck ?l - level)
	     (ready-to-load ?g - goods ?m - market ?l - level) 
	     (stored ?g - goods ?l - level) 
	     (on-sale ?g - goods ?m -  market ?l - level)
	     (next ?l1 ?l2 - level)
	     (at ?t - truck ?p - place)
	     (connected ?p1 ?p2 - place)
	     (allowed_to_remove ?loc1 - place ?loc2 - place))

(:action attack_drive
 :parameters (?t - truck ?from ?to - place)
 :precondition (and (at ?t ?from) (connected ?from ?to))
 :effect (and (not (at ?t ?from)) (at ?t ?to)))


; ### LOAD ###
; ?l1 is the level of ?g ready to be loaded at ?m before loading
; ?l2 is the level of ?g ready to be loaded at ?m after loading
; ?l3 is the level of ?g in ?t before loading
; ?l4 is the level of ?g in ?t after loading

(:action attack_load
 :parameters (?g - goods ?t - truck ?m - market ?l1 ?l2 ?l3 ?l4 - level)
 :precondition (and (at ?t ?m) (loaded ?g ?t ?l3)
		    (ready-to-load ?g ?m ?l2) (next ?l2 ?l1) (next ?l4 ?l3))
 :effect (and (loaded ?g ?t ?l4) (not (loaded ?g ?t ?l3)) 
	      (ready-to-load ?g ?m ?l1) (not (ready-to-load ?g ?m ?l2))))


; ### UNLOAD ###
; ?l1 is the level of ?g in ?t before unloading
; ?l2 is the level of ?g in ?t after unloading
; ?l3 is the level of ?g in ?d before unloading
; ?l4 is the level of ?g in ?d after unloading

(:action attack_unload
 :parameters (?g - goods ?t - truck ?d - depot ?l1 ?l2 ?l3 ?l4 - level)
 :precondition (and (at ?t ?d) (loaded ?g ?t ?l2)
		    (stored ?g ?l3) (next ?l2 ?l1) (next ?l4 ?l3))
 :effect (and (loaded ?g ?t ?l1) (not (loaded ?g ?t ?l2)) 
	      (stored ?g ?l4) (not (stored ?g ?l3))))


; ### BUY ###
; ?l1 is the level of ?g on sale at ?m before buying
; ?l2 is the level of ?g on sale at ?m after buying 
; ?l3 is the level of ?g ready to be loaded at ?m before buying
; ?l4 is the level of ?g ready to be loaded at ?m after buying

(:action attack_buy
 :parameters (?t - truck ?g - goods ?m - market ?l1 ?l2 ?l3 ?l4 - level)
 :precondition (and (at ?t ?m) (on-sale ?g ?m ?l2) (ready-to-load ?g ?m ?l3)
		    (next ?l2 ?l1) (next ?l4 ?l3))
 :effect (and (on-sale ?g ?m ?l1) (not (on-sale ?g ?m ?l2)) 
	      (ready-to-load ?g ?m ?l4) (not (ready-to-load ?g ?m ?l3))))

(:action fix_drive
 :parameters (?t - fix_truck ?from ?to - place)
 :precondition (and (at ?t ?from) (connected ?from ?to))
 :effect (and (not (at ?t ?from)) (at ?t ?to)))	      

(:action fix_remove_connection
 :parameters (?t - fix_truck ?from ?to - place)
 :precondition (and (at ?t ?from) (connected ?from ?to)
 					(connected ?to ?from)
 					(allowed_to_remove ?to ?from))
 :effect (and (not (connected ?from ?to))
 			  (not (connected ?to ?from)) ))	  

)