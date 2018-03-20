;; Transport sequential
;; Basically transport domain with additional fix truck driving on the map which can remove a road segment if it is in an adjacent city.
;; The cost for this truck to drive is currently one and for removing a segment, it is the road-length
;; THESE COSTS ARE PROBABLY NOT PERFECTLY CHOSEN
;; The initial location of the fix truck always is city-loc-1 or, starting with p07.pddl, city-1-loc-1

(define (domain transport)
  (:requirements :typing :action-costs)
  (:types
        location target locatable - object
        vehicle fix_vehicle package - locatable
        capacity-number - object
  )

  (:predicates 
     (road ?l1 ?l2 - location)
     (at ?x - locatable ?v - location)
     (in ?x - package ?v - vehicle)
     (capacity ?v - vehicle ?s1 - capacity-number)
     (capacity-predecessor ?s1 ?s2 - capacity-number)
  )

  (:functions
     (road-length ?l1 ?l2 - location) - number
     (total-cost) - number
  )

  (:action attack_drive
    :parameters (?v - vehicle ?l1 ?l2 - location)
    :precondition (and
        (at ?v ?l1)
        (road ?l1 ?l2)
      )
    :effect (and
        (not (at ?v ?l1))
        (at ?v ?l2)
        (increase (total-cost) (road-length ?l1 ?l2))
      )
  )

 (:action attack_pick-up
    :parameters (?v - vehicle ?l - location ?p - package ?s1 ?s2 - capacity-number)
    :precondition (and
        (at ?v ?l)
        (at ?p ?l)
        (capacity-predecessor ?s1 ?s2)
        (capacity ?v ?s2)
      )
    :effect (and
        (not (at ?p ?l))
        (in ?p ?v)
        (capacity ?v ?s1)
        (not (capacity ?v ?s2))
        (increase (total-cost) 1)
      )
  )

  (:action attack_drop
    :parameters (?v - vehicle ?l - location ?p - package ?s1 ?s2 - capacity-number)
    :precondition (and
        (at ?v ?l)
        (in ?p ?v)
        (capacity-predecessor ?s1 ?s2)
        (capacity ?v ?s1)
      )
    :effect (and
        (not (in ?p ?v))
        (at ?p ?l)
        (capacity ?v ?s2)
        (not (capacity ?v ?s1))
        (increase (total-cost) 1)
      )
  )

  (:action fix_drive
    :parameters (?v - fix_vehicle ?l1 ?l2 - location)
    :precondition (and
        (at ?v ?l1)
        (road ?l1 ?l2)
      )
    :effect (and
        (not (at ?v ?l1))
        (at ?v ?l2)
        (increase (total-cost) 1);;(road-length ?l1 ?l2))
      )
  )  

  (:action fix_remove_road
    :parameters (?v - fix_vehicle ?l1 ?l2 - location)
    :precondition (and
        (road ?l1 ?l2)
        (at ?v ?l1)
      )
    :effect (and
        (not (road ?l1 ?l2))
        (increase (total-cost) (road-length ?l1 ?l2))
      )
  )

)
