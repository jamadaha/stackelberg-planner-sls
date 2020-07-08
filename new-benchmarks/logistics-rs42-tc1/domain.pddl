;; Basically logistics domain with additional fix actions which remove connection in both directions at once between locations in the same city.
;; Only connections for which the allowed_to_remove predicate is set, can be removed.
;; The cost for removing a connection is 1

(define (domain logistics)
  (:requirements :strips) 
  (:predicates 	(package ?obj)
	       	(truck ?truck)
		(airplane ?airplane)
                (airport ?airport)
               	(location ?loc)
		(in-city ?obj ?city)
                (city ?city)
		(at ?obj ?loc)
		(in ?obj ?obj)
    (removed-connection ?loc1 ?loc2)
    (allowed_to_remove ?loc1 ?loc2))

 
(:action load-truck
  :parameters
   (?obj
    ?truck
    ?loc)
  :precondition
   (and (package ?obj) (truck ?truck) (location ?loc)
   (at ?truck ?loc) (at ?obj ?loc))
  :effect
   (and (not (at ?obj ?loc)) (in ?obj ?truck)))

(:action load-airplane
  :parameters
   (?obj
    ?airplane
    ?loc)
  :precondition
   (and (package ?obj) (airplane ?airplane) (location ?loc)
   (at ?obj ?loc) (at ?airplane ?loc))
  :effect
   (and (not (at ?obj ?loc)) (in ?obj ?airplane)))

(:action unload-truck
  :parameters
   (?obj
    ?truck
    ?loc)
  :precondition
   (and (package ?obj) (truck ?truck) (location ?loc)
        (at ?truck ?loc) (in ?obj ?truck))
  :effect
   (and (not (in ?obj ?truck)) (at ?obj ?loc)))

(:action unload-airplane
  :parameters
   (?obj
    ?airplane
    ?loc)
  :precondition
   (and (package ?obj) (airplane ?airplane) (location ?loc)
        (in ?obj ?airplane) (at ?airplane ?loc))
  :effect
   (and (not (in ?obj ?airplane)) (at ?obj ?loc)))

(:action drive-truck
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

(:action fly-airplane
  :parameters
   (?airplane
    ?loc-from
    ?loc-to)
  :precondition
   (and (airplane ?airplane) (airport ?loc-from) (airport ?loc-to)
	(at ?airplane ?loc-from))
  :effect
   (and (not (at ?airplane ?loc-from)) (at ?airplane ?loc-to)))

(:action fix_REMOVE-CONNECTION
  :parameters
   (?loc-1
    ?loc-2
    ?city)
  :precondition
   (and (LOCATION ?loc-1) (LOCATION ?loc-2) (CITY ?city)
   (in-city ?loc-1 ?city)
   (in-city ?loc-2 ?city)
   (allowed_to_remove ?loc-1 ?loc-2)
   (not (removed-connection ?loc-1 ?loc-2))
   (not (removed-connection ?loc-2 ?loc-1)))
  :effect
   (and (removed-connection ?loc-1 ?loc-2)
        (removed-connection ?loc-2 ?loc-1)))    
)
