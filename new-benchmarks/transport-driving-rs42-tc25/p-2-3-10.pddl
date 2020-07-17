; Transport two-cities-sequential-10nodes-1000size-5degree-100mindistance-2trucks-3packages-42seed

(define (problem transport-two-cities-sequential-10nodes-1000size-5degree-100mindistance-2trucks-3packages-42seed)
 (:domain transport)
 (:objects
  city-1-loc-1 - location
  city-2-loc-1 - location
  city-1-loc-2 - location
  city-2-loc-2 - location
  city-1-loc-3 - location
  city-2-loc-3 - location
  city-1-loc-4 - location
  city-2-loc-4 - location
  city-1-loc-5 - location
  city-2-loc-5 - location
  city-1-loc-6 - location
  city-2-loc-6 - location
  city-1-loc-7 - location
  city-2-loc-7 - location
  city-1-loc-8 - location
  city-2-loc-8 - location
  city-1-loc-9 - location
  city-2-loc-9 - location
  city-1-loc-10 - location
  city-2-loc-10 - location
  truck-1 - vehicle
  truck-2 - vehicle
  package-1 - package
  package-2 - package
  package-3 - package
  capacity-0 - capacity-number
  capacity-1 - capacity-number
  capacity-2 - capacity-number
  capacity-3 - capacity-number
  capacity-4 - capacity-number
  truck_f - fix_vehicle
 )
 (:init
(allowed_to_remove city-1-loc-1 city-1-loc-2)
(allowed_to_remove city-1-loc-1 city-1-loc-3)
(allowed_to_remove city-1-loc-1 city-1-loc-7)
(allowed_to_remove city-1-loc-10 city-1-loc-4)
(allowed_to_remove city-1-loc-10 city-1-loc-7)
(allowed_to_remove city-1-loc-2 city-1-loc-4)
(allowed_to_remove city-1-loc-2 city-1-loc-6)
(allowed_to_remove city-1-loc-2 city-1-loc-8)
(allowed_to_remove city-1-loc-4 city-1-loc-5)
(allowed_to_remove city-1-loc-4 city-1-loc-6)
(allowed_to_remove city-1-loc-4 city-1-loc-7)
(allowed_to_remove city-1-loc-6 city-1-loc-7)
(allowed_to_remove city-1-loc-7 city-1-loc-8)
(allowed_to_remove city-2-loc-1 city-2-loc-2)
(allowed_to_remove city-2-loc-1 city-2-loc-9)
(allowed_to_remove city-2-loc-10 city-2-loc-2)
(allowed_to_remove city-2-loc-10 city-2-loc-9)
(allowed_to_remove city-2-loc-2 city-2-loc-3)
(allowed_to_remove city-2-loc-2 city-2-loc-8)
(allowed_to_remove city-2-loc-2 city-2-loc-9)
(allowed_to_remove city-2-loc-3 city-2-loc-4)
(allowed_to_remove city-2-loc-3 city-2-loc-9)
(allowed_to_remove city-2-loc-4 city-2-loc-6)
(allowed_to_remove city-2-loc-6 city-2-loc-9)
(allowed_to_remove city-2-loc-8 city-2-loc-9)
  (= (total-cost) 0)
  (capacity-predecessor capacity-0 capacity-1)
  (capacity-predecessor capacity-1 capacity-2)
  (capacity-predecessor capacity-2 capacity-3)
  (capacity-predecessor capacity-3 capacity-4)
  ; 517,616 -> 223,238
  (road city-1-loc-2 city-1-loc-1)
  (= (road-length city-1-loc-2 city-1-loc-1) 48)
  ; 223,238 -> 517,616
  (road city-1-loc-1 city-1-loc-2)
  (= (road-length city-1-loc-1 city-1-loc-2) 48)
  ; 27,574 -> 223,238
  (road city-1-loc-3 city-1-loc-1)
  (= (road-length city-1-loc-3 city-1-loc-1) 39)
  ; 223,238 -> 27,574
  (road city-1-loc-1 city-1-loc-3)
  (= (road-length city-1-loc-1 city-1-loc-3) 39)
  ; 203,733 -> 517,616
  (road city-1-loc-4 city-1-loc-2)
  (= (road-length city-1-loc-4 city-1-loc-2) 34)
  ; 517,616 -> 203,733
  (road city-1-loc-2 city-1-loc-4)
  (= (road-length city-1-loc-2 city-1-loc-4) 34)
  ; 203,733 -> 27,574
  (road city-1-loc-4 city-1-loc-3)
  (= (road-length city-1-loc-4 city-1-loc-3) 24)
  ; 27,574 -> 203,733
  (road city-1-loc-3 city-1-loc-4)
  (= (road-length city-1-loc-3 city-1-loc-4) 24)
  ; 665,718 -> 517,616
  (road city-1-loc-5 city-1-loc-2)
  (= (road-length city-1-loc-5 city-1-loc-2) 18)
  ; 517,616 -> 665,718
  (road city-1-loc-2 city-1-loc-5)
  (= (road-length city-1-loc-2 city-1-loc-5) 18)
  ; 665,718 -> 203,733
  (road city-1-loc-5 city-1-loc-4)
  (= (road-length city-1-loc-5 city-1-loc-4) 47)
  ; 203,733 -> 665,718
  (road city-1-loc-4 city-1-loc-5)
  (= (road-length city-1-loc-4 city-1-loc-5) 47)
  ; 558,429 -> 223,238
  (road city-1-loc-6 city-1-loc-1)
  (= (road-length city-1-loc-6 city-1-loc-1) 39)
  ; 223,238 -> 558,429
  (road city-1-loc-1 city-1-loc-6)
  (= (road-length city-1-loc-1 city-1-loc-6) 39)
  ; 558,429 -> 517,616
  (road city-1-loc-6 city-1-loc-2)
  (= (road-length city-1-loc-6 city-1-loc-2) 20)
  ; 517,616 -> 558,429
  (road city-1-loc-2 city-1-loc-6)
  (= (road-length city-1-loc-2 city-1-loc-6) 20)
  ; 558,429 -> 203,733
  (road city-1-loc-6 city-1-loc-4)
  (= (road-length city-1-loc-6 city-1-loc-4) 47)
  ; 203,733 -> 558,429
  (road city-1-loc-4 city-1-loc-6)
  (= (road-length city-1-loc-4 city-1-loc-6) 47)
  ; 558,429 -> 665,718
  (road city-1-loc-6 city-1-loc-5)
  (= (road-length city-1-loc-6 city-1-loc-5) 31)
  ; 665,718 -> 558,429
  (road city-1-loc-5 city-1-loc-6)
  (= (road-length city-1-loc-5 city-1-loc-6) 31)
  ; 225,459 -> 223,238
  (road city-1-loc-7 city-1-loc-1)
  (= (road-length city-1-loc-7 city-1-loc-1) 23)
  ; 223,238 -> 225,459
  (road city-1-loc-1 city-1-loc-7)
  (= (road-length city-1-loc-1 city-1-loc-7) 23)
  ; 225,459 -> 517,616
  (road city-1-loc-7 city-1-loc-2)
  (= (road-length city-1-loc-7 city-1-loc-2) 34)
  ; 517,616 -> 225,459
  (road city-1-loc-2 city-1-loc-7)
  (= (road-length city-1-loc-2 city-1-loc-7) 34)
  ; 225,459 -> 27,574
  (road city-1-loc-7 city-1-loc-3)
  (= (road-length city-1-loc-7 city-1-loc-3) 23)
  ; 27,574 -> 225,459
  (road city-1-loc-3 city-1-loc-7)
  (= (road-length city-1-loc-3 city-1-loc-7) 23)
  ; 225,459 -> 203,733
  (road city-1-loc-7 city-1-loc-4)
  (= (road-length city-1-loc-7 city-1-loc-4) 28)
  ; 203,733 -> 225,459
  (road city-1-loc-4 city-1-loc-7)
  (= (road-length city-1-loc-4 city-1-loc-7) 28)
  ; 225,459 -> 558,429
  (road city-1-loc-7 city-1-loc-6)
  (= (road-length city-1-loc-7 city-1-loc-6) 34)
  ; 558,429 -> 225,459
  (road city-1-loc-6 city-1-loc-7)
  (= (road-length city-1-loc-6 city-1-loc-7) 34)
  ; 603,284 -> 223,238
  (road city-1-loc-8 city-1-loc-1)
  (= (road-length city-1-loc-8 city-1-loc-1) 39)
  ; 223,238 -> 603,284
  (road city-1-loc-1 city-1-loc-8)
  (= (road-length city-1-loc-1 city-1-loc-8) 39)
  ; 603,284 -> 517,616
  (road city-1-loc-8 city-1-loc-2)
  (= (road-length city-1-loc-8 city-1-loc-2) 35)
  ; 517,616 -> 603,284
  (road city-1-loc-2 city-1-loc-8)
  (= (road-length city-1-loc-2 city-1-loc-8) 35)
  ; 603,284 -> 665,718
  (road city-1-loc-8 city-1-loc-5)
  (= (road-length city-1-loc-8 city-1-loc-5) 44)
  ; 665,718 -> 603,284
  (road city-1-loc-5 city-1-loc-8)
  (= (road-length city-1-loc-5 city-1-loc-8) 44)
  ; 603,284 -> 558,429
  (road city-1-loc-8 city-1-loc-6)
  (= (road-length city-1-loc-8 city-1-loc-6) 16)
  ; 558,429 -> 603,284
  (road city-1-loc-6 city-1-loc-8)
  (= (road-length city-1-loc-6 city-1-loc-8) 16)
  ; 603,284 -> 225,459
  (road city-1-loc-8 city-1-loc-7)
  (= (road-length city-1-loc-8 city-1-loc-7) 42)
  ; 225,459 -> 603,284
  (road city-1-loc-7 city-1-loc-8)
  (= (road-length city-1-loc-7 city-1-loc-8) 42)
  ; 828,890 -> 517,616
  (road city-1-loc-9 city-1-loc-2)
  (= (road-length city-1-loc-9 city-1-loc-2) 42)
  ; 517,616 -> 828,890
  (road city-1-loc-2 city-1-loc-9)
  (= (road-length city-1-loc-2 city-1-loc-9) 42)
  ; 828,890 -> 665,718
  (road city-1-loc-9 city-1-loc-5)
  (= (road-length city-1-loc-9 city-1-loc-5) 24)
  ; 665,718 -> 828,890
  (road city-1-loc-5 city-1-loc-9)
  (= (road-length city-1-loc-5 city-1-loc-9) 24)
  ; 6,777 -> 27,574
  (road city-1-loc-10 city-1-loc-3)
  (= (road-length city-1-loc-10 city-1-loc-3) 21)
  ; 27,574 -> 6,777
  (road city-1-loc-3 city-1-loc-10)
  (= (road-length city-1-loc-3 city-1-loc-10) 21)
  ; 6,777 -> 203,733
  (road city-1-loc-10 city-1-loc-4)
  (= (road-length city-1-loc-10 city-1-loc-4) 21)
  ; 203,733 -> 6,777
  (road city-1-loc-4 city-1-loc-10)
  (= (road-length city-1-loc-4 city-1-loc-10) 21)
  ; 6,777 -> 225,459
  (road city-1-loc-10 city-1-loc-7)
  (= (road-length city-1-loc-10 city-1-loc-7) 39)
  ; 225,459 -> 6,777
  (road city-1-loc-7 city-1-loc-10)
  (= (road-length city-1-loc-7 city-1-loc-10) 39)
  ; 2714,432 -> 2825,163
  (road city-2-loc-2 city-2-loc-1)
  (= (road-length city-2-loc-2 city-2-loc-1) 30)
  ; 2825,163 -> 2714,432
  (road city-2-loc-1 city-2-loc-2)
  (= (road-length city-2-loc-1 city-2-loc-2) 30)
  ; 2348,284 -> 2714,432
  (road city-2-loc-3 city-2-loc-2)
  (= (road-length city-2-loc-3 city-2-loc-2) 40)
  ; 2714,432 -> 2348,284
  (road city-2-loc-2 city-2-loc-3)
  (= (road-length city-2-loc-2 city-2-loc-3) 40)
  ; 2159,220 -> 2348,284
  (road city-2-loc-4 city-2-loc-3)
  (= (road-length city-2-loc-4 city-2-loc-3) 20)
  ; 2348,284 -> 2159,220
  (road city-2-loc-3 city-2-loc-4)
  (= (road-length city-2-loc-3 city-2-loc-4) 20)
  ; 2980,781 -> 2714,432
  (road city-2-loc-5 city-2-loc-2)
  (= (road-length city-2-loc-5 city-2-loc-2) 44)
  ; 2714,432 -> 2980,781
  (road city-2-loc-2 city-2-loc-5)
  (= (road-length city-2-loc-2 city-2-loc-5) 44)
  ; 2344,104 -> 2348,284
  (road city-2-loc-6 city-2-loc-3)
  (= (road-length city-2-loc-6 city-2-loc-3) 18)
  ; 2348,284 -> 2344,104
  (road city-2-loc-3 city-2-loc-6)
  (= (road-length city-2-loc-3 city-2-loc-6) 18)
  ; 2344,104 -> 2159,220
  (road city-2-loc-6 city-2-loc-4)
  (= (road-length city-2-loc-6 city-2-loc-4) 22)
  ; 2159,220 -> 2344,104
  (road city-2-loc-4 city-2-loc-6)
  (= (road-length city-2-loc-4 city-2-loc-6) 22)
  ; 2094,389 -> 2348,284
  (road city-2-loc-7 city-2-loc-3)
  (= (road-length city-2-loc-7 city-2-loc-3) 28)
  ; 2348,284 -> 2094,389
  (road city-2-loc-3 city-2-loc-7)
  (= (road-length city-2-loc-3 city-2-loc-7) 28)
  ; 2094,389 -> 2159,220
  (road city-2-loc-7 city-2-loc-4)
  (= (road-length city-2-loc-7 city-2-loc-4) 19)
  ; 2159,220 -> 2094,389
  (road city-2-loc-4 city-2-loc-7)
  (= (road-length city-2-loc-4 city-2-loc-7) 19)
  ; 2094,389 -> 2344,104
  (road city-2-loc-7 city-2-loc-6)
  (= (road-length city-2-loc-7 city-2-loc-6) 38)
  ; 2344,104 -> 2094,389
  (road city-2-loc-6 city-2-loc-7)
  (= (road-length city-2-loc-6 city-2-loc-7) 38)
  ; 2867,352 -> 2825,163
  (road city-2-loc-8 city-2-loc-1)
  (= (road-length city-2-loc-8 city-2-loc-1) 20)
  ; 2825,163 -> 2867,352
  (road city-2-loc-1 city-2-loc-8)
  (= (road-length city-2-loc-1 city-2-loc-8) 20)
  ; 2867,352 -> 2714,432
  (road city-2-loc-8 city-2-loc-2)
  (= (road-length city-2-loc-8 city-2-loc-2) 18)
  ; 2714,432 -> 2867,352
  (road city-2-loc-2 city-2-loc-8)
  (= (road-length city-2-loc-2 city-2-loc-8) 18)
  ; 2867,352 -> 2980,781
  (road city-2-loc-8 city-2-loc-5)
  (= (road-length city-2-loc-8 city-2-loc-5) 45)
  ; 2980,781 -> 2867,352
  (road city-2-loc-5 city-2-loc-8)
  (= (road-length city-2-loc-5 city-2-loc-8) 45)
  ; 2618,270 -> 2825,163
  (road city-2-loc-9 city-2-loc-1)
  (= (road-length city-2-loc-9 city-2-loc-1) 24)
  ; 2825,163 -> 2618,270
  (road city-2-loc-1 city-2-loc-9)
  (= (road-length city-2-loc-1 city-2-loc-9) 24)
  ; 2618,270 -> 2714,432
  (road city-2-loc-9 city-2-loc-2)
  (= (road-length city-2-loc-9 city-2-loc-2) 19)
  ; 2714,432 -> 2618,270
  (road city-2-loc-2 city-2-loc-9)
  (= (road-length city-2-loc-2 city-2-loc-9) 19)
  ; 2618,270 -> 2348,284
  (road city-2-loc-9 city-2-loc-3)
  (= (road-length city-2-loc-9 city-2-loc-3) 27)
  ; 2348,284 -> 2618,270
  (road city-2-loc-3 city-2-loc-9)
  (= (road-length city-2-loc-3 city-2-loc-9) 27)
  ; 2618,270 -> 2159,220
  (road city-2-loc-9 city-2-loc-4)
  (= (road-length city-2-loc-9 city-2-loc-4) 47)
  ; 2159,220 -> 2618,270
  (road city-2-loc-4 city-2-loc-9)
  (= (road-length city-2-loc-4 city-2-loc-9) 47)
  ; 2618,270 -> 2344,104
  (road city-2-loc-9 city-2-loc-6)
  (= (road-length city-2-loc-9 city-2-loc-6) 32)
  ; 2344,104 -> 2618,270
  (road city-2-loc-6 city-2-loc-9)
  (= (road-length city-2-loc-6 city-2-loc-9) 32)
  ; 2618,270 -> 2867,352
  (road city-2-loc-9 city-2-loc-8)
  (= (road-length city-2-loc-9 city-2-loc-8) 27)
  ; 2867,352 -> 2618,270
  (road city-2-loc-8 city-2-loc-9)
  (= (road-length city-2-loc-8 city-2-loc-9) 27)
  ; 2826,44 -> 2825,163
  (road city-2-loc-10 city-2-loc-1)
  (= (road-length city-2-loc-10 city-2-loc-1) 12)
  ; 2825,163 -> 2826,44
  (road city-2-loc-1 city-2-loc-10)
  (= (road-length city-2-loc-1 city-2-loc-10) 12)
  ; 2826,44 -> 2714,432
  (road city-2-loc-10 city-2-loc-2)
  (= (road-length city-2-loc-10 city-2-loc-2) 41)
  ; 2714,432 -> 2826,44
  (road city-2-loc-2 city-2-loc-10)
  (= (road-length city-2-loc-2 city-2-loc-10) 41)
  ; 2826,44 -> 2867,352
  (road city-2-loc-10 city-2-loc-8)
  (= (road-length city-2-loc-10 city-2-loc-8) 32)
  ; 2867,352 -> 2826,44
  (road city-2-loc-8 city-2-loc-10)
  (= (road-length city-2-loc-8 city-2-loc-10) 32)
  ; 2826,44 -> 2618,270
  (road city-2-loc-10 city-2-loc-9)
  (= (road-length city-2-loc-10 city-2-loc-9) 31)
  ; 2618,270 -> 2826,44
  (road city-2-loc-9 city-2-loc-10)
  (= (road-length city-2-loc-9 city-2-loc-10) 31)
  ; 828,890 <-> 2094,389
  (road city-1-loc-9 city-2-loc-7)
  (= (road-length city-1-loc-9 city-2-loc-7) 137)
  (road city-2-loc-7 city-1-loc-9)
  (= (road-length city-2-loc-7 city-1-loc-9) 137)
  (at package-1 city-1-loc-8)
  (at package-2 city-1-loc-9)
  (at package-3 city-1-loc-2)
  (at truck-1 city-2-loc-7)
  (capacity truck-1 capacity-2)
  (at truck-2 city-2-loc-9)
  (capacity truck-2 capacity-3)
  (at truck_f city-1-loc-1)
 )
 (:goal (and
  (at package-1 city-2-loc-10)
  (at package-2 city-2-loc-6)
  (at package-3 city-2-loc-10)
 ))
 (:metric minimize (total-cost))
)
