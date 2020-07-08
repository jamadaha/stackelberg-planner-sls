; Transport city-sequential-10nodes-1000size-5degree-100mindistance-2trucks-4packages-42seed

(define (problem transport-city-sequential-10nodes-1000size-5degree-100mindistance-2trucks-4packages-42seed)
 (:domain transport)
 (:objects
  city-loc-1 - location
  city-loc-2 - location
  city-loc-3 - location
  city-loc-4 - location
  city-loc-5 - location
  city-loc-6 - location
  city-loc-7 - location
  city-loc-8 - location
  city-loc-9 - location
  city-loc-10 - location
  truck-1 - vehicle
  truck-2 - vehicle
  package-1 - package
  package-2 - package
  package-3 - package
  package-4 - package
  capacity-0 - capacity-number
  capacity-1 - capacity-number
  capacity-2 - capacity-number
  capacity-3 - capacity-number
  capacity-4 - capacity-number
 )
 (:init
(allowed_to_remove city-loc-1 city-loc-2)
(allowed_to_remove city-loc-3 city-loc-7)
  (= (total-cost) 0)
  (capacity-predecessor capacity-0 capacity-1)
  (capacity-predecessor capacity-1 capacity-2)
  (capacity-predecessor capacity-2 capacity-3)
  (capacity-predecessor capacity-3 capacity-4)
  ; 517,616 -> 223,238
  (road city-loc-2 city-loc-1)
  (= (road-length city-loc-2 city-loc-1) 48)
  ; 223,238 -> 517,616
  (road city-loc-1 city-loc-2)
  (= (road-length city-loc-1 city-loc-2) 48)
  ; 27,574 -> 223,238
  (road city-loc-3 city-loc-1)
  (= (road-length city-loc-3 city-loc-1) 39)
  ; 223,238 -> 27,574
  (road city-loc-1 city-loc-3)
  (= (road-length city-loc-1 city-loc-3) 39)
  ; 203,733 -> 517,616
  (road city-loc-4 city-loc-2)
  (= (road-length city-loc-4 city-loc-2) 34)
  ; 517,616 -> 203,733
  (road city-loc-2 city-loc-4)
  (= (road-length city-loc-2 city-loc-4) 34)
  ; 203,733 -> 27,574
  (road city-loc-4 city-loc-3)
  (= (road-length city-loc-4 city-loc-3) 24)
  ; 27,574 -> 203,733
  (road city-loc-3 city-loc-4)
  (= (road-length city-loc-3 city-loc-4) 24)
  ; 665,718 -> 517,616
  (road city-loc-5 city-loc-2)
  (= (road-length city-loc-5 city-loc-2) 18)
  ; 517,616 -> 665,718
  (road city-loc-2 city-loc-5)
  (= (road-length city-loc-2 city-loc-5) 18)
  ; 665,718 -> 203,733
  (road city-loc-5 city-loc-4)
  (= (road-length city-loc-5 city-loc-4) 47)
  ; 203,733 -> 665,718
  (road city-loc-4 city-loc-5)
  (= (road-length city-loc-4 city-loc-5) 47)
  ; 558,429 -> 223,238
  (road city-loc-6 city-loc-1)
  (= (road-length city-loc-6 city-loc-1) 39)
  ; 223,238 -> 558,429
  (road city-loc-1 city-loc-6)
  (= (road-length city-loc-1 city-loc-6) 39)
  ; 558,429 -> 517,616
  (road city-loc-6 city-loc-2)
  (= (road-length city-loc-6 city-loc-2) 20)
  ; 517,616 -> 558,429
  (road city-loc-2 city-loc-6)
  (= (road-length city-loc-2 city-loc-6) 20)
  ; 558,429 -> 203,733
  (road city-loc-6 city-loc-4)
  (= (road-length city-loc-6 city-loc-4) 47)
  ; 203,733 -> 558,429
  (road city-loc-4 city-loc-6)
  (= (road-length city-loc-4 city-loc-6) 47)
  ; 558,429 -> 665,718
  (road city-loc-6 city-loc-5)
  (= (road-length city-loc-6 city-loc-5) 31)
  ; 665,718 -> 558,429
  (road city-loc-5 city-loc-6)
  (= (road-length city-loc-5 city-loc-6) 31)
  ; 225,459 -> 223,238
  (road city-loc-7 city-loc-1)
  (= (road-length city-loc-7 city-loc-1) 23)
  ; 223,238 -> 225,459
  (road city-loc-1 city-loc-7)
  (= (road-length city-loc-1 city-loc-7) 23)
  ; 225,459 -> 517,616
  (road city-loc-7 city-loc-2)
  (= (road-length city-loc-7 city-loc-2) 34)
  ; 517,616 -> 225,459
  (road city-loc-2 city-loc-7)
  (= (road-length city-loc-2 city-loc-7) 34)
  ; 225,459 -> 27,574
  (road city-loc-7 city-loc-3)
  (= (road-length city-loc-7 city-loc-3) 23)
  ; 27,574 -> 225,459
  (road city-loc-3 city-loc-7)
  (= (road-length city-loc-3 city-loc-7) 23)
  ; 225,459 -> 203,733
  (road city-loc-7 city-loc-4)
  (= (road-length city-loc-7 city-loc-4) 28)
  ; 203,733 -> 225,459
  (road city-loc-4 city-loc-7)
  (= (road-length city-loc-4 city-loc-7) 28)
  ; 225,459 -> 558,429
  (road city-loc-7 city-loc-6)
  (= (road-length city-loc-7 city-loc-6) 34)
  ; 558,429 -> 225,459
  (road city-loc-6 city-loc-7)
  (= (road-length city-loc-6 city-loc-7) 34)
  ; 603,284 -> 223,238
  (road city-loc-8 city-loc-1)
  (= (road-length city-loc-8 city-loc-1) 39)
  ; 223,238 -> 603,284
  (road city-loc-1 city-loc-8)
  (= (road-length city-loc-1 city-loc-8) 39)
  ; 603,284 -> 517,616
  (road city-loc-8 city-loc-2)
  (= (road-length city-loc-8 city-loc-2) 35)
  ; 517,616 -> 603,284
  (road city-loc-2 city-loc-8)
  (= (road-length city-loc-2 city-loc-8) 35)
  ; 603,284 -> 665,718
  (road city-loc-8 city-loc-5)
  (= (road-length city-loc-8 city-loc-5) 44)
  ; 665,718 -> 603,284
  (road city-loc-5 city-loc-8)
  (= (road-length city-loc-5 city-loc-8) 44)
  ; 603,284 -> 558,429
  (road city-loc-8 city-loc-6)
  (= (road-length city-loc-8 city-loc-6) 16)
  ; 558,429 -> 603,284
  (road city-loc-6 city-loc-8)
  (= (road-length city-loc-6 city-loc-8) 16)
  ; 603,284 -> 225,459
  (road city-loc-8 city-loc-7)
  (= (road-length city-loc-8 city-loc-7) 42)
  ; 225,459 -> 603,284
  (road city-loc-7 city-loc-8)
  (= (road-length city-loc-7 city-loc-8) 42)
  ; 828,890 -> 517,616
  (road city-loc-9 city-loc-2)
  (= (road-length city-loc-9 city-loc-2) 42)
  ; 517,616 -> 828,890
  (road city-loc-2 city-loc-9)
  (= (road-length city-loc-2 city-loc-9) 42)
  ; 828,890 -> 665,718
  (road city-loc-9 city-loc-5)
  (= (road-length city-loc-9 city-loc-5) 24)
  ; 665,718 -> 828,890
  (road city-loc-5 city-loc-9)
  (= (road-length city-loc-5 city-loc-9) 24)
  ; 6,777 -> 27,574
  (road city-loc-10 city-loc-3)
  (= (road-length city-loc-10 city-loc-3) 21)
  ; 27,574 -> 6,777
  (road city-loc-3 city-loc-10)
  (= (road-length city-loc-3 city-loc-10) 21)
  ; 6,777 -> 203,733
  (road city-loc-10 city-loc-4)
  (= (road-length city-loc-10 city-loc-4) 21)
  ; 203,733 -> 6,777
  (road city-loc-4 city-loc-10)
  (= (road-length city-loc-4 city-loc-10) 21)
  ; 6,777 -> 225,459
  (road city-loc-10 city-loc-7)
  (= (road-length city-loc-10 city-loc-7) 39)
  ; 225,459 -> 6,777
  (road city-loc-7 city-loc-10)
  (= (road-length city-loc-7 city-loc-10) 39)
  (at package-1 city-loc-3)
  (at package-2 city-loc-7)
  (at package-3 city-loc-6)
  (at package-4 city-loc-5)
  (at truck-1 city-loc-4)
  (capacity truck-1 capacity-2)
  (at truck-2 city-loc-2)
  (capacity truck-2 capacity-3)
 )
 (:goal (and
  (at package-1 city-loc-2)
  (at package-2 city-loc-2)
  (at package-3 city-loc-10)
  (at package-4 city-loc-1)
 ))
 (:metric minimize (total-cost))
)
