(define
  (problem strips-mysty-x-18)
  (:domain no-mystery-strips)
  (:objects haltingen kleinkems brombach endingen schopfheim
      koendringen inzlingen gottenheim breisach lauchringen sexau
      kandern weil guendlingen riedlingen bahlingen tumringen
      bad-bellingen emmendingen boetzingen muellheim wollbach
      pferdetransport droschke moped kutsche motorrad trollwagen
      daemonenrikscha neujahrsbrezel karfiol kiste-bier fernseher
      fussball haehnchenbein kuechenmaschine bananenkiste donnerkiesel
      fuel-0 fuel-1 fuel-2 fuel-3 fuel-4 fuel-5 fuel-6 fuel-7
      capacity-0 capacity-1 capacity-2 capacity-3 truck-fix)
  (:init
    (at bananenkiste muellheim)
    (at daemonenrikscha wollbach)
    (at donnerkiesel wollbach)
    (at droschke endingen)
    (at fernseher sexau)
    (at fussball bad-bellingen)
    (at haehnchenbein emmendingen)
    (at karfiol breisach)
    (at kiste-bier lauchringen)
    (at kuechenmaschine boetzingen)
    (at kutsche breisach)
    (at moped schopfheim)
    (at motorrad riedlingen)
    (at neujahrsbrezel brombach)
    (at pferdetransport kleinkems)
    (at trollwagen bahlingen)
    (capacity daemonenrikscha capacity-2)
    (capacity droschke capacity-1)
    (capacity kutsche capacity-1)
    (capacity moped capacity-1)
    (capacity motorrad capacity-3)
    (capacity pferdetransport capacity-3)
    (capacity trollwagen capacity-3)
    (capacity-number capacity-0)
    (capacity-number capacity-1)
    (capacity-number capacity-2)
    (capacity-number capacity-3)
    (capacity-predecessor capacity-0 capacity-1)
    (capacity-predecessor capacity-1 capacity-2)
    (capacity-predecessor capacity-2 capacity-3)
    (connected bad-bellingen muellheim)
    (connected bad-bellingen tumringen)
    (connected bahlingen boetzingen)
    (connected bahlingen breisach)
    (connected bahlingen wollbach)
    (connected boetzingen bahlingen)
    (connected boetzingen guendlingen)
    (connected boetzingen riedlingen)
    (connected breisach bahlingen)
    (connected breisach lauchringen)
    (connected breisach sexau)
    (connected breisach weil)
    (connected brombach haltingen)
    (connected brombach inzlingen)
    (connected brombach koendringen)
    (connected emmendingen riedlingen)
    (connected emmendingen wollbach)
    (connected endingen haltingen)
    (connected endingen schopfheim)
    (connected gottenheim inzlingen)
    (connected gottenheim weil)
    (connected guendlingen boetzingen)
    (connected guendlingen muellheim)
    (connected haltingen brombach)
    (connected haltingen endingen)
    (connected inzlingen brombach)
    (connected inzlingen gottenheim)
    (connected inzlingen kandern)
    (connected kandern inzlingen)
    (connected kandern sexau)
    (connected kleinkems koendringen)
    (connected kleinkems schopfheim)
    (connected koendringen brombach)
    (connected koendringen kleinkems)
    (connected lauchringen breisach)
    (connected lauchringen sexau)
    (connected lauchringen weil)
    (connected muellheim bad-bellingen)
    (connected muellheim guendlingen)
    (connected riedlingen boetzingen)
    (connected riedlingen emmendingen)
    (connected schopfheim endingen)
    (connected schopfheim kleinkems)
    (connected sexau breisach)
    (connected sexau kandern)
    (connected sexau lauchringen)
    (connected tumringen bad-bellingen)
    (connected tumringen wollbach)
    (connected weil breisach)
    (connected weil gottenheim)
    (connected weil lauchringen)
    (connected wollbach bahlingen)
    (connected wollbach emmendingen)
    (connected wollbach tumringen)
    (fuel bad-bellingen fuel-5)
    (fuel bahlingen fuel-1)
    (fuel boetzingen fuel-7)
    (fuel breisach fuel-0)
    (fuel brombach fuel-3)
    (fuel emmendingen fuel-2)
    (fuel endingen fuel-5)
    (fuel gottenheim fuel-6)
    (fuel guendlingen fuel-3)
    (fuel haltingen fuel-4)
    (fuel inzlingen fuel-2)
    (fuel kandern fuel-7)
    (fuel kleinkems fuel-2)
    (fuel koendringen fuel-6)
    (fuel lauchringen fuel-0)
    (fuel muellheim fuel-0)
    (fuel riedlingen fuel-0)
    (fuel schopfheim fuel-5)
    (fuel sexau fuel-4)
    (fuel tumringen fuel-0)
    (fuel weil fuel-1)
    (fuel wollbach fuel-2)
    (fuel-number fuel-0)
    (fuel-number fuel-1)
    (fuel-number fuel-2)
    (fuel-number fuel-3)
    (fuel-number fuel-4)
    (fuel-number fuel-5)
    (fuel-number fuel-6)
    (fuel-number fuel-7)
    (fuel-predecessor fuel-0 fuel-1)
    (fuel-predecessor fuel-1 fuel-2)
    (fuel-predecessor fuel-2 fuel-3)
    (fuel-predecessor fuel-3 fuel-4)
    (fuel-predecessor fuel-4 fuel-5)
    (fuel-predecessor fuel-5 fuel-6)
    (fuel-predecessor fuel-6 fuel-7)
    (location bad-bellingen)
    (location bahlingen)
    (location boetzingen)
    (location breisach)
    (location brombach)
    (location emmendingen)
    (location endingen)
    (location gottenheim)
    (location guendlingen)
    (location haltingen)
    (location inzlingen)
    (location kandern)
    (location kleinkems)
    (location koendringen)
    (location lauchringen)
    (location muellheim)
    (location riedlingen)
    (location schopfheim)
    (location sexau)
    (location tumringen)
    (location weil)
    (location wollbach)
    (package bananenkiste)
    (package donnerkiesel)
    (package fernseher)
    (package fussball)
    (package haehnchenbein)
    (package karfiol)
    (package kiste-bier)
    (package kuechenmaschine)
    (package neujahrsbrezel)
    (vehicle daemonenrikscha)
    (vehicle droschke)
    (vehicle kutsche)
    (vehicle moped)
    (vehicle motorrad)
    (vehicle pferdetransport)
    (vehicle trollwagen)
    (fix_vehicle truck-fix)
    (at truck-fix haltingen))
  (:goal
    (and
      (at fussball kleinkems))))
