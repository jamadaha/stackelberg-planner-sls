; Transport city-sequential-22nodes-1000size-5degree-100mindistance-2trucks-4packages-42seed

(define (problem transport-city-sequential-22nodes-1000size-5degree-100mindistance-2trucks-4packages-42seed)
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
  city-loc-11 - location
  city-loc-12 - location
  city-loc-13 - location
  city-loc-14 - location
  city-loc-15 - location
  city-loc-16 - location
  city-loc-17 - location
  city-loc-18 - location
  city-loc-19 - location
  city-loc-20 - location
  city-loc-21 - location
  city-loc-22 - location
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
  (= (total-cost) 0)
  (capacity-predecessor capacity-0 capacity-1)
  (capacity-predecessor capacity-1 capacity-2)
  (capacity-predecessor capacity-2 capacity-3)
  (capacity-predecessor capacity-3 capacity-4)
  ; 228,142 -> 281,250
  (road city-loc-4 city-loc-3)
  (= (road-length city-loc-4 city-loc-3) 12)
  ; 281,250 -> 228,142
  (road city-loc-3 city-loc-4)
  (= (road-length city-loc-3 city-loc-4) 12)
  ; 754,104 -> 654,114
  (road city-loc-5 city-loc-1)
  (= (road-length city-loc-5 city-loc-1) 10)
  ; 654,114 -> 754,104
  (road city-loc-1 city-loc-5)
  (= (road-length city-loc-1 city-loc-5) 10)
  ; 913,558 -> 692,758
  (road city-loc-7 city-loc-6)
  (= (road-length city-loc-7 city-loc-6) 30)
  ; 692,758 -> 913,558
  (road city-loc-6 city-loc-7)
  (= (road-length city-loc-6 city-loc-7) 30)
  ; 89,604 -> 25,759
  (road city-loc-8 city-loc-2)
  (= (road-length city-loc-8 city-loc-2) 17)
  ; 25,759 -> 89,604
  (road city-loc-2 city-loc-8)
  (= (road-length city-loc-2 city-loc-8) 17)
  ; 432,32 -> 654,114
  (road city-loc-9 city-loc-1)
  (= (road-length city-loc-9 city-loc-1) 24)
  ; 654,114 -> 432,32
  (road city-loc-1 city-loc-9)
  (= (road-length city-loc-1 city-loc-9) 24)
  ; 432,32 -> 281,250
  (road city-loc-9 city-loc-3)
  (= (road-length city-loc-9 city-loc-3) 27)
  ; 281,250 -> 432,32
  (road city-loc-3 city-loc-9)
  (= (road-length city-loc-3 city-loc-9) 27)
  ; 432,32 -> 228,142
  (road city-loc-9 city-loc-4)
  (= (road-length city-loc-9 city-loc-4) 24)
  ; 228,142 -> 432,32
  (road city-loc-4 city-loc-9)
  (= (road-length city-loc-4 city-loc-9) 24)
  ; 30,95 -> 281,250
  (road city-loc-10 city-loc-3)
  (= (road-length city-loc-10 city-loc-3) 30)
  ; 281,250 -> 30,95
  (road city-loc-3 city-loc-10)
  (= (road-length city-loc-3 city-loc-10) 30)
  ; 30,95 -> 228,142
  (road city-loc-10 city-loc-4)
  (= (road-length city-loc-10 city-loc-4) 21)
  ; 228,142 -> 30,95
  (road city-loc-4 city-loc-10)
  (= (road-length city-loc-4 city-loc-10) 21)
  ; 517,616 -> 692,758
  (road city-loc-11 city-loc-6)
  (= (road-length city-loc-11 city-loc-6) 23)
  ; 692,758 -> 517,616
  (road city-loc-6 city-loc-11)
  (= (road-length city-loc-6 city-loc-11) 23)
  ; 203,733 -> 25,759
  (road city-loc-12 city-loc-2)
  (= (road-length city-loc-12 city-loc-2) 18)
  ; 25,759 -> 203,733
  (road city-loc-2 city-loc-12)
  (= (road-length city-loc-2 city-loc-12) 18)
  ; 203,733 -> 89,604
  (road city-loc-12 city-loc-8)
  (= (road-length city-loc-12 city-loc-8) 18)
  ; 89,604 -> 203,733
  (road city-loc-8 city-loc-12)
  (= (road-length city-loc-8 city-loc-12) 18)
  ; 558,429 -> 517,616
  (road city-loc-13 city-loc-11)
  (= (road-length city-loc-13 city-loc-11) 20)
  ; 517,616 -> 558,429
  (road city-loc-11 city-loc-13)
  (= (road-length city-loc-11 city-loc-13) 20)
  ; 225,459 -> 281,250
  (road city-loc-14 city-loc-3)
  (= (road-length city-loc-14 city-loc-3) 22)
  ; 281,250 -> 225,459
  (road city-loc-3 city-loc-14)
  (= (road-length city-loc-3 city-loc-14) 22)
  ; 225,459 -> 228,142
  (road city-loc-14 city-loc-4)
  (= (road-length city-loc-14 city-loc-4) 32)
  ; 228,142 -> 225,459
  (road city-loc-4 city-loc-14)
  (= (road-length city-loc-4 city-loc-14) 32)
  ; 225,459 -> 89,604
  (road city-loc-14 city-loc-8)
  (= (road-length city-loc-14 city-loc-8) 20)
  ; 89,604 -> 225,459
  (road city-loc-8 city-loc-14)
  (= (road-length city-loc-8 city-loc-14) 20)
  ; 225,459 -> 203,733
  (road city-loc-14 city-loc-12)
  (= (road-length city-loc-14 city-loc-12) 28)
  ; 203,733 -> 225,459
  (road city-loc-12 city-loc-14)
  (= (road-length city-loc-12 city-loc-14) 28)
  ; 603,284 -> 654,114
  (road city-loc-15 city-loc-1)
  (= (road-length city-loc-15 city-loc-1) 18)
  ; 654,114 -> 603,284
  (road city-loc-1 city-loc-15)
  (= (road-length city-loc-1 city-loc-15) 18)
  ; 603,284 -> 754,104
  (road city-loc-15 city-loc-5)
  (= (road-length city-loc-15 city-loc-5) 24)
  ; 754,104 -> 603,284
  (road city-loc-5 city-loc-15)
  (= (road-length city-loc-5 city-loc-15) 24)
  ; 603,284 -> 432,32
  (road city-loc-15 city-loc-9)
  (= (road-length city-loc-15 city-loc-9) 31)
  ; 432,32 -> 603,284
  (road city-loc-9 city-loc-15)
  (= (road-length city-loc-9 city-loc-15) 31)
  ; 603,284 -> 558,429
  (road city-loc-15 city-loc-13)
  (= (road-length city-loc-15 city-loc-13) 16)
  ; 558,429 -> 603,284
  (road city-loc-13 city-loc-15)
  (= (road-length city-loc-13 city-loc-15) 16)
  ; 828,890 -> 692,758
  (road city-loc-16 city-loc-6)
  (= (road-length city-loc-16 city-loc-6) 19)
  ; 692,758 -> 828,890
  (road city-loc-6 city-loc-16)
  (= (road-length city-loc-6 city-loc-16) 19)
  ; 714,432 -> 913,558
  (road city-loc-17 city-loc-7)
  (= (road-length city-loc-17 city-loc-7) 24)
  ; 913,558 -> 714,432
  (road city-loc-7 city-loc-17)
  (= (road-length city-loc-7 city-loc-17) 24)
  ; 714,432 -> 517,616
  (road city-loc-17 city-loc-11)
  (= (road-length city-loc-17 city-loc-11) 27)
  ; 517,616 -> 714,432
  (road city-loc-11 city-loc-17)
  (= (road-length city-loc-11 city-loc-17) 27)
  ; 714,432 -> 558,429
  (road city-loc-17 city-loc-13)
  (= (road-length city-loc-17 city-loc-13) 16)
  ; 558,429 -> 714,432
  (road city-loc-13 city-loc-17)
  (= (road-length city-loc-13 city-loc-17) 16)
  ; 714,432 -> 603,284
  (road city-loc-17 city-loc-15)
  (= (road-length city-loc-17 city-loc-15) 19)
  ; 603,284 -> 714,432
  (road city-loc-15 city-loc-17)
  (= (road-length city-loc-15 city-loc-17) 19)
  ; 159,220 -> 281,250
  (road city-loc-18 city-loc-3)
  (= (road-length city-loc-18 city-loc-3) 13)
  ; 281,250 -> 159,220
  (road city-loc-3 city-loc-18)
  (= (road-length city-loc-3 city-loc-18) 13)
  ; 159,220 -> 228,142
  (road city-loc-18 city-loc-4)
  (= (road-length city-loc-18 city-loc-4) 11)
  ; 228,142 -> 159,220
  (road city-loc-4 city-loc-18)
  (= (road-length city-loc-4 city-loc-18) 11)
  ; 159,220 -> 30,95
  (road city-loc-18 city-loc-10)
  (= (road-length city-loc-18 city-loc-10) 18)
  ; 30,95 -> 159,220
  (road city-loc-10 city-loc-18)
  (= (road-length city-loc-10 city-loc-18) 18)
  ; 159,220 -> 225,459
  (road city-loc-18 city-loc-14)
  (= (road-length city-loc-18 city-loc-14) 25)
  ; 225,459 -> 159,220
  (road city-loc-14 city-loc-18)
  (= (road-length city-loc-14 city-loc-18) 25)
  ; 980,781 -> 692,758
  (road city-loc-19 city-loc-6)
  (= (road-length city-loc-19 city-loc-6) 29)
  ; 692,758 -> 980,781
  (road city-loc-6 city-loc-19)
  (= (road-length city-loc-6 city-loc-19) 29)
  ; 980,781 -> 913,558
  (road city-loc-19 city-loc-7)
  (= (road-length city-loc-19 city-loc-7) 24)
  ; 913,558 -> 980,781
  (road city-loc-7 city-loc-19)
  (= (road-length city-loc-7 city-loc-19) 24)
  ; 980,781 -> 828,890
  (road city-loc-19 city-loc-16)
  (= (road-length city-loc-19 city-loc-16) 19)
  ; 828,890 -> 980,781
  (road city-loc-16 city-loc-19)
  (= (road-length city-loc-16 city-loc-19) 19)
  ; 344,104 -> 654,114
  (road city-loc-20 city-loc-1)
  (= (road-length city-loc-20 city-loc-1) 31)
  ; 654,114 -> 344,104
  (road city-loc-1 city-loc-20)
  (= (road-length city-loc-1 city-loc-20) 31)
  ; 344,104 -> 281,250
  (road city-loc-20 city-loc-3)
  (= (road-length city-loc-20 city-loc-3) 16)
  ; 281,250 -> 344,104
  (road city-loc-3 city-loc-20)
  (= (road-length city-loc-3 city-loc-20) 16)
  ; 344,104 -> 228,142
  (road city-loc-20 city-loc-4)
  (= (road-length city-loc-20 city-loc-4) 13)
  ; 228,142 -> 344,104
  (road city-loc-4 city-loc-20)
  (= (road-length city-loc-4 city-loc-20) 13)
  ; 344,104 -> 432,32
  (road city-loc-20 city-loc-9)
  (= (road-length city-loc-20 city-loc-9) 12)
  ; 432,32 -> 344,104
  (road city-loc-9 city-loc-20)
  (= (road-length city-loc-9 city-loc-20) 12)
  ; 344,104 -> 30,95
  (road city-loc-20 city-loc-10)
  (= (road-length city-loc-20 city-loc-10) 32)
  ; 30,95 -> 344,104
  (road city-loc-10 city-loc-20)
  (= (road-length city-loc-10 city-loc-20) 32)
  ; 344,104 -> 603,284
  (road city-loc-20 city-loc-15)
  (= (road-length city-loc-20 city-loc-15) 32)
  ; 603,284 -> 344,104
  (road city-loc-15 city-loc-20)
  (= (road-length city-loc-15 city-loc-20) 32)
  ; 344,104 -> 159,220
  (road city-loc-20 city-loc-18)
  (= (road-length city-loc-20 city-loc-18) 22)
  ; 159,220 -> 344,104
  (road city-loc-18 city-loc-20)
  (= (road-length city-loc-18 city-loc-20) 22)
  ; 94,389 -> 281,250
  (road city-loc-21 city-loc-3)
  (= (road-length city-loc-21 city-loc-3) 24)
  ; 281,250 -> 94,389
  (road city-loc-3 city-loc-21)
  (= (road-length city-loc-3 city-loc-21) 24)
  ; 94,389 -> 228,142
  (road city-loc-21 city-loc-4)
  (= (road-length city-loc-21 city-loc-4) 29)
  ; 228,142 -> 94,389
  (road city-loc-4 city-loc-21)
  (= (road-length city-loc-4 city-loc-21) 29)
  ; 94,389 -> 89,604
  (road city-loc-21 city-loc-8)
  (= (road-length city-loc-21 city-loc-8) 22)
  ; 89,604 -> 94,389
  (road city-loc-8 city-loc-21)
  (= (road-length city-loc-8 city-loc-21) 22)
  ; 94,389 -> 30,95
  (road city-loc-21 city-loc-10)
  (= (road-length city-loc-21 city-loc-10) 31)
  ; 30,95 -> 94,389
  (road city-loc-10 city-loc-21)
  (= (road-length city-loc-10 city-loc-21) 31)
  ; 94,389 -> 225,459
  (road city-loc-21 city-loc-14)
  (= (road-length city-loc-21 city-loc-14) 15)
  ; 225,459 -> 94,389
  (road city-loc-14 city-loc-21)
  (= (road-length city-loc-14 city-loc-21) 15)
  ; 94,389 -> 159,220
  (road city-loc-21 city-loc-18)
  (= (road-length city-loc-21 city-loc-18) 19)
  ; 159,220 -> 94,389
  (road city-loc-18 city-loc-21)
  (= (road-length city-loc-18 city-loc-21) 19)
  ; 867,352 -> 654,114
  (road city-loc-22 city-loc-1)
  (= (road-length city-loc-22 city-loc-1) 32)
  ; 654,114 -> 867,352
  (road city-loc-1 city-loc-22)
  (= (road-length city-loc-1 city-loc-22) 32)
  ; 867,352 -> 754,104
  (road city-loc-22 city-loc-5)
  (= (road-length city-loc-22 city-loc-5) 28)
  ; 754,104 -> 867,352
  (road city-loc-5 city-loc-22)
  (= (road-length city-loc-5 city-loc-22) 28)
  ; 867,352 -> 913,558
  (road city-loc-22 city-loc-7)
  (= (road-length city-loc-22 city-loc-7) 22)
  ; 913,558 -> 867,352
  (road city-loc-7 city-loc-22)
  (= (road-length city-loc-7 city-loc-22) 22)
  ; 867,352 -> 558,429
  (road city-loc-22 city-loc-13)
  (= (road-length city-loc-22 city-loc-13) 32)
  ; 558,429 -> 867,352
  (road city-loc-13 city-loc-22)
  (= (road-length city-loc-13 city-loc-22) 32)
  ; 867,352 -> 603,284
  (road city-loc-22 city-loc-15)
  (= (road-length city-loc-22 city-loc-15) 28)
  ; 603,284 -> 867,352
  (road city-loc-15 city-loc-22)
  (= (road-length city-loc-15 city-loc-22) 28)
  ; 867,352 -> 714,432
  (road city-loc-22 city-loc-17)
  (= (road-length city-loc-22 city-loc-17) 18)
  ; 714,432 -> 867,352
  (road city-loc-17 city-loc-22)
  (= (road-length city-loc-17 city-loc-22) 18)
  (at package-1 city-loc-20)
  (at package-2 city-loc-9)
  (at package-3 city-loc-2)
  (at package-4 city-loc-15)
  (at truck-1 city-loc-4)
  (capacity truck-1 capacity-4)
  (at truck-2 city-loc-3)
  (capacity truck-2 capacity-3)
 )
 (:goal (and
  (at package-1 city-loc-18)
  (at package-2 city-loc-10)
  (at package-3 city-loc-21)
  (at package-4 city-loc-20)
 ))
 (:metric minimize (total-cost))
)
