;; Basically logistics domain with additional fix truck which can remove a connection in both directions at once between locations in the same city, if it is in one of both locations.
;; Only connections for which the allowed_to_remove predicate is set, can be removed.
;; The fix truck can drive between locations in the same city and fly from one aiport to another.
;; All fix actions have cost of 1
;; The initial fix truck location is l00

(define (domain logistics)
  (:requirements :strips) 
  (:predicates 	(package ?obj)
	       	(truck ?truck)
          (fix_truck ?truck)
		(airplane ?airplane)
                (airport ?airport)
               	(location ?loc)
		(in-city ?obj ?city)
                (city ?city)
		(at ?obj ?loc)
		(in ?obj ?obj)
    (removed-connection ?loc1 ?loc2)
    (allowed_to_remove ?loc1 ?loc2))

 
(:action attack_load-truck
  :parameters
   (?obj
    ?truck
    ?loc)
  :precondition
   (and (package ?obj) (truck ?truck) (location ?loc)
   (at ?truck ?loc) (at ?obj ?loc))
  :effect
   (and (not (at ?obj ?loc)) (in ?obj ?truck)))

(:action attack_load-airplane
  :parameters
   (?obj
    ?airplane
    ?loc)
  :precondition
   (and (package ?obj) (airplane ?airplane) (location ?loc)
   (at ?obj ?loc) (at ?airplane ?loc))
  :effect
   (and (not (at ?obj ?loc)) (in ?obj ?airplane)))

(:action attack_unload-truck
  :parameters
   (?obj
    ?truck
    ?loc)
  :precondition
   (and (package ?obj) (truck ?truck) (location ?loc)
        (at ?truck ?loc) (in ?obj ?truck))
  :effect
   (and (not (in ?obj ?truck)) (at ?obj ?loc)))

(:action attack_unload-airplane
  :parameters
   (?obj
    ?airplane
    ?loc)
  :precondition
   (and (package ?obj) (airplane ?airplane) (location ?loc)
        (in ?obj ?airplane) (at ?airplane ?loc))
  :effect
   (and (not (in ?obj ?airplane)) (at ?obj ?loc)))

(:action attack_drive-truck
  :parameters
   (?truck
    ?loc-from
    ?loc-to
    ?city)
  :precondition
   (and (truck ?truck) (location ?loc-from) (location ?loc-to) (city ?city)
   (at ?truck ?loc-from)
   (in-city ?loc-from ?city)
   (in-city ?loc-to ?city)
   (not (removed-connection ?loc-from ?loc-to)))
  :effect
   (and (not (at ?truck ?loc-from)) (at ?truck ?loc-to)))

(:action attack_fly-airplane
  :parameters
   (?airplane
    ?loc-from
    ?loc-to)
  :precondition
   (and (airplane ?airplane) (airport ?loc-from) (airport ?loc-to)
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
   (and (increase (total-cost) 1) (not (at ?truck ?loc-from)) (at ?truck ?loc-to)))

(:action fix_FLY-TRUCK
  :parameters
   (?truck
    ?loc-from
    ?loc-to)
  :precondition
   (and (FIX_TRUCK ?truck) (AIRPORT ?loc-from) (AIRPORT ?loc-to)
  (at ?truck ?loc-from))
  :effect
   (and (increase (total-cost) 1) (not (at ?truck ?loc-from)) (at ?truck ?loc-to)))   

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
   (and (increase (total-cost) 1) 
        (removed-connection ?loc-1 ?loc-2)
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
   (and (increase (total-cost) 1)
        (removed-connection ?loc-1 ?loc-2)
        (removed-connection ?loc-2 ?loc-1)))            
)
