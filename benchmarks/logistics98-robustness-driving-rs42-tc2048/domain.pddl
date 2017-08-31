;; Basically logistics98 domain with additional fix truck which can remove a connection in both directions at once between locations in the same city, if it is in one of both locations.
;; Only connections for which the allowed_to_remove predicate is set, can be removed.
;; The fix truck can drive between locations in the same city and fly from one aiport to another.
;; All fix actions have cost of 1

(define (domain logistics-strips)
  (:requirements :strips) 
  (:predicates 	(OBJ ?obj)
	       	(TRUCK ?truck)
                (FIX_TRUCK ?truck)
               	(LOCATION ?loc)
		(AIRPLANE ?airplane)
                (CITY ?city)
                (AIRPORT ?airport)
		(at ?obj ?loc)
		(in ?obj1 ?obj2)
		(in-city ?obj ?city)
    (removed-connection ?loc1 ?loc2)
    (allowed_to_remove ?loc1 ?loc2))
 
  ; (:types )		; default object

(:action attack_LOAD-TRUCK
  :parameters
   (?obj
    ?truck
    ?loc)
  :precondition
   (and (OBJ ?obj) (TRUCK ?truck) (LOCATION ?loc)
   (at ?truck ?loc) (at ?obj ?loc))
  :effect
   (and (not (at ?obj ?loc)) (in ?obj ?truck)))

(:action attack_LOAD-AIRPLANE
  :parameters
   (?obj
    ?airplane
    ?loc)
  :precondition
   (and (OBJ ?obj) (AIRPLANE ?airplane) (LOCATION ?loc)
   (at ?obj ?loc) (at ?airplane ?loc))
  :effect
   (and (not (at ?obj ?loc)) (in ?obj ?airplane)))

(:action attack_UNLOAD-TRUCK
  :parameters
   (?obj
    ?truck
    ?loc)
  :precondition
   (and (OBJ ?obj) (TRUCK ?truck) (LOCATION ?loc)
        (at ?truck ?loc) (in ?obj ?truck))
  :effect
   (and (not (in ?obj ?truck)) (at ?obj ?loc)))

(:action attack_UNLOAD-AIRPLANE
  :parameters
   (?obj
    ?airplane
    ?loc)
  :precondition
   (and (OBJ ?obj) (AIRPLANE ?airplane) (LOCATION ?loc)
        (in ?obj ?airplane) (at ?airplane ?loc))
  :effect
   (and (not (in ?obj ?airplane)) (at ?obj ?loc)))

(:action attack_DRIVE-TRUCK
  :parameters
   (?truck
    ?loc-from
    ?loc-to
    ?city)
  :precondition
   (and (TRUCK ?truck) (LOCATION ?loc-from) (LOCATION ?loc-to) (CITY ?city)
   (at ?truck ?loc-from)
   (in-city ?loc-from ?city)
   (in-city ?loc-to ?city)
   (not (removed-connection ?loc-from ?loc-to)))
  :effect
   (and (not (at ?truck ?loc-from)) (at ?truck ?loc-to)))

(:action attack_FLY-AIRPLANE
  :parameters
   (?airplane
    ?loc-from
    ?loc-to)
  :precondition
   (and (AIRPLANE ?airplane) (AIRPORT ?loc-from) (AIRPORT ?loc-to)
	(at ?airplane ?loc-from))
  :effect
   (and (not (at ?airplane ?loc-from)) (at ?airplane ?loc-to)))

(:action fix_DRIVE-TRUCK
  :parameters
   (?truck
    ?loc-from
    ?loc-to
    ?city)
  :precondition
   (and (FIX_TRUCK ?truck) (LOCATION ?loc-from) (LOCATION ?loc-to) (CITY ?city)
   (at ?truck ?loc-from)
   (in-city ?loc-from ?city)
   (in-city ?loc-to ?city)
   (not (removed-connection ?loc-from ?loc-to)))
  :effect
   (and (not (at ?truck ?loc-from)) (at ?truck ?loc-to)))

(:action fix_FLY-TRUCK
  :parameters
   (?truck
    ?loc-from
    ?loc-to)
  :precondition
   (and (FIX_TRUCK ?truck) (AIRPORT ?loc-from) (AIRPORT ?loc-to)
  (at ?truck ?loc-from))
  :effect
   (and (not (at ?truck ?loc-from)) (at ?truck ?loc-to)))   

(:action fix_REMOVE-CONNECTION_1
  :parameters
   (?truck
    ?loc-1
    ?loc-2
    ?city)
  :precondition
   (and (LOCATION ?loc-1) (LOCATION ?loc-2) (CITY ?city)
   (FIX_TRUCK ?truck)
   (at ?truck ?loc-1)
   (in-city ?loc-1 ?city)
   (in-city ?loc-2 ?city)
   (allowed_to_remove ?loc-1 ?loc-2)
   (not (removed-connection ?loc-1 ?loc-2))
   (not (removed-connection ?loc-2 ?loc-1)))
  :effect
   (and (removed-connection ?loc-1 ?loc-2)
        (removed-connection ?loc-2 ?loc-1)))

(:action fix_REMOVE-CONNECTION_2
  :parameters
   (?truck
    ?loc-1
    ?loc-2
    ?city)
  :precondition
   (and (LOCATION ?loc-1) (LOCATION ?loc-2) (CITY ?city)
   (FIX_TRUCK ?truck)
   (at ?truck ?loc-2)
   (in-city ?loc-1 ?city)
   (in-city ?loc-2 ?city)
   (allowed_to_remove ?loc-1 ?loc-2)
   (not (removed-connection ?loc-1 ?loc-2))
   (not (removed-connection ?loc-2 ?loc-1)))
  :effect
   (and (removed-connection ?loc-1 ?loc-2)
        (removed-connection ?loc-2 ?loc-1)))         
)
