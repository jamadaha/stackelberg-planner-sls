(define
  (problem strips-mysty-x-25)
  (:domain no-mystery-strips)
  (:objects wittlingen sexau muellheim schallstadt ICE droschke
      neujahrsbrezel tuete-pommes fuel-0 fuel-1 fuel-2 fuel-3 fuel-4
      fuel-5 capacity-0 capacity-1 capacity-2 capacity-3 truck-fix)
  (:init
    (at ICE sexau)
    (at droschke muellheim)
    (at neujahrsbrezel wittlingen)
    (at tuete-pommes schallstadt)
    (capacity ICE capacity-1)
    (capacity droschke capacity-3)
    (capacity-number capacity-0)
    (capacity-number capacity-1)
    (capacity-number capacity-2)
    (capacity-number capacity-3)
    (capacity-predecessor capacity-0 capacity-1)
    (capacity-predecessor capacity-1 capacity-2)
    (capacity-predecessor capacity-2 capacity-3)
    (connected muellheim schallstadt)
    (connected muellheim sexau)
    (connected muellheim wittlingen)
    (connected schallstadt muellheim)
    (connected schallstadt wittlingen)
    (connected sexau muellheim)
    (connected sexau wittlingen)
    (connected wittlingen muellheim)
    (connected wittlingen schallstadt)
    (connected wittlingen sexau)
    (fuel muellheim fuel-5)
    (fuel schallstadt fuel-2)
    (fuel sexau fuel-2)
    (fuel wittlingen fuel-2)
    (fuel-number fuel-0)
    (fuel-number fuel-1)
    (fuel-number fuel-2)
    (fuel-number fuel-3)
    (fuel-number fuel-4)
    (fuel-number fuel-5)
    (fuel-predecessor fuel-0 fuel-1)
    (fuel-predecessor fuel-1 fuel-2)
    (fuel-predecessor fuel-2 fuel-3)
    (fuel-predecessor fuel-3 fuel-4)
    (fuel-predecessor fuel-4 fuel-5)
    (location muellheim)
    (location schallstadt)
    (location sexau)
    (location wittlingen)
    (package neujahrsbrezel)
    (package tuete-pommes)
    (vehicle ICE)
    (vehicle droschke)
    (fix_vehicle truck-fix)
    (at truck-fix wittlingen))
  (:goal
    (and
      (at neujahrsbrezel schallstadt))))
