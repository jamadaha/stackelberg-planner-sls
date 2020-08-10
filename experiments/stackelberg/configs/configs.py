#!/usr/bin/env python2
# encoding: utf-8

from collections import defaultdict


PREPROCESS_REVISION_DEFAULT = "92845ada3fd61a733f99702ffcfff2b3b99260f0"

PREPROCESS_REVISION_SOFT = "6f1b5abf91f0b5a9f7dace3fea8ffbedcff3c7dc"

REVISION = "d059552e393f05e01d52bb8bd880873acf4dce78"
SERVERS = "new_servers" 



SUITE_AAAI21_COMPLETE = ['aaai21-logistics-driving-rs42-tc1', 'aaai21-logistics-driving-rs42-tc10', 'aaai21-logistics-driving-rs42-tc12', 'aaai21-logistics-driving-rs42-tc128', 'aaai21-logistics-driving-rs42-tc16', 'aaai21-logistics-driving-rs42-tc2', 'aaai21-logistics-driving-rs42-tc25', 'aaai21-logistics-driving-rs42-tc3', 'aaai21-logistics-driving-rs42-tc32', 'aaai21-logistics-driving-rs42-tc4', 'aaai21-logistics-driving-rs42-tc5', 'aaai21-logistics-driving-rs42-tc50', 'aaai21-logistics-driving-rs42-tc6', 'aaai21-logistics-driving-rs42-tc64', 'aaai21-logistics-driving-rs42-tc8', 'aaai21-logistics-driving-rs42-tcall', 'aaai21-logistics-rs42-tc1', 'aaai21-logistics-rs42-tc10', 'aaai21-logistics-rs42-tc12', 'aaai21-logistics-rs42-tc128', 'aaai21-logistics-rs42-tc16', 'aaai21-logistics-rs42-tc2', 'aaai21-logistics-rs42-tc25', 'aaai21-logistics-rs42-tc3', 'aaai21-logistics-rs42-tc32', 'aaai21-logistics-rs42-tc4', 'aaai21-logistics-rs42-tc5', 'aaai21-logistics-rs42-tc50', 'aaai21-logistics-rs42-tc6', 'aaai21-logistics-rs42-tc64', 'aaai21-logistics-rs42-tc8', 'aaai21-logistics-rs42-tcall', 'aaai21-nomystery-driving-rs42-tc1', 'aaai21-nomystery-driving-rs42-tc10', 'aaai21-nomystery-driving-rs42-tc12', 'aaai21-nomystery-driving-rs42-tc128', 'aaai21-nomystery-driving-rs42-tc16', 'aaai21-nomystery-driving-rs42-tc2', 'aaai21-nomystery-driving-rs42-tc25', 'aaai21-nomystery-driving-rs42-tc3', 'aaai21-nomystery-driving-rs42-tc32', 'aaai21-nomystery-driving-rs42-tc4', 'aaai21-nomystery-driving-rs42-tc5', 'aaai21-nomystery-driving-rs42-tc50', 'aaai21-nomystery-driving-rs42-tc6', 'aaai21-nomystery-driving-rs42-tc64', 'aaai21-nomystery-driving-rs42-tc8', 'aaai21-nomystery-driving-rs42-tcall', 'aaai21-nomystery-rs42-tc1', 'aaai21-nomystery-rs42-tc10', 'aaai21-nomystery-rs42-tc12', 'aaai21-nomystery-rs42-tc128', 'aaai21-nomystery-rs42-tc16', 'aaai21-nomystery-rs42-tc2', 'aaai21-nomystery-rs42-tc25', 'aaai21-nomystery-rs42-tc3', 'aaai21-nomystery-rs42-tc32', 'aaai21-nomystery-rs42-tc4', 'aaai21-nomystery-rs42-tc5', 'aaai21-nomystery-rs42-tc50', 'aaai21-nomystery-rs42-tc6', 'aaai21-nomystery-rs42-tc64', 'aaai21-nomystery-rs42-tc8', 'aaai21-nomystery-rs42-tcall', 'aaai21-pentesting-robustness-rs42-tc1', 'aaai21-pentesting-robustness-rs42-tc10', 'aaai21-pentesting-robustness-rs42-tc12', 'aaai21-pentesting-robustness-rs42-tc128', 'aaai21-pentesting-robustness-rs42-tc16', 'aaai21-pentesting-robustness-rs42-tc2', 'aaai21-pentesting-robustness-rs42-tc25', 'aaai21-pentesting-robustness-rs42-tc256', 'aaai21-pentesting-robustness-rs42-tc3', 'aaai21-pentesting-robustness-rs42-tc32', 'aaai21-pentesting-robustness-rs42-tc4', 'aaai21-pentesting-robustness-rs42-tc5', 'aaai21-pentesting-robustness-rs42-tc50', 'aaai21-pentesting-robustness-rs42-tc512', 'aaai21-pentesting-robustness-rs42-tc6', 'aaai21-pentesting-robustness-rs42-tc64', 'aaai21-pentesting-robustness-rs42-tc8', 'aaai21-pentesting-robustness-rs42-tcall', 'aaai21-rovers-driving-rs42-tc1', 'aaai21-rovers-driving-rs42-tc10', 'aaai21-rovers-driving-rs42-tc12', 'aaai21-rovers-driving-rs42-tc16', 'aaai21-rovers-driving-rs42-tc2', 'aaai21-rovers-driving-rs42-tc25', 'aaai21-rovers-driving-rs42-tc3', 'aaai21-rovers-driving-rs42-tc32', 'aaai21-rovers-driving-rs42-tc4', 'aaai21-rovers-driving-rs42-tc5', 'aaai21-rovers-driving-rs42-tc50', 'aaai21-rovers-driving-rs42-tc6', 'aaai21-rovers-driving-rs42-tc8', 'aaai21-rovers-driving-rs42-tcall', 'aaai21-rovers-rs42-tc1', 'aaai21-rovers-rs42-tc10', 'aaai21-rovers-rs42-tc12', 'aaai21-rovers-rs42-tc16', 'aaai21-rovers-rs42-tc2', 'aaai21-rovers-rs42-tc25', 'aaai21-rovers-rs42-tc3', 'aaai21-rovers-rs42-tc32', 'aaai21-rovers-rs42-tc4', 'aaai21-rovers-rs42-tc5', 'aaai21-rovers-rs42-tc50', 'aaai21-rovers-rs42-tc6', 'aaai21-rovers-rs42-tc8', 'aaai21-rovers-rs42-tcall', 'aaai21-tpp-driving-rs42-tc1', 'aaai21-tpp-driving-rs42-tc10', 'aaai21-tpp-driving-rs42-tc12', 'aaai21-tpp-driving-rs42-tc128', 'aaai21-tpp-driving-rs42-tc16', 'aaai21-tpp-driving-rs42-tc2', 'aaai21-tpp-driving-rs42-tc25', 'aaai21-tpp-driving-rs42-tc256', 'aaai21-tpp-driving-rs42-tc3', 'aaai21-tpp-driving-rs42-tc32', 'aaai21-tpp-driving-rs42-tc4', 'aaai21-tpp-driving-rs42-tc5', 'aaai21-tpp-driving-rs42-tc50', 'aaai21-tpp-driving-rs42-tc6', 'aaai21-tpp-driving-rs42-tc64', 'aaai21-tpp-driving-rs42-tc8', 'aaai21-tpp-driving-rs42-tcall', 'aaai21-tpp-rs42-tc1', 'aaai21-tpp-rs42-tc10', 'aaai21-tpp-rs42-tc12', 'aaai21-tpp-rs42-tc128', 'aaai21-tpp-rs42-tc16', 'aaai21-tpp-rs42-tc2', 'aaai21-tpp-rs42-tc25', 'aaai21-tpp-rs42-tc256', 'aaai21-tpp-rs42-tc3', 'aaai21-tpp-rs42-tc32', 'aaai21-tpp-rs42-tc4', 'aaai21-tpp-rs42-tc5', 'aaai21-tpp-rs42-tc50', 'aaai21-tpp-rs42-tc6', 'aaai21-tpp-rs42-tc64', 'aaai21-tpp-rs42-tc8', 'aaai21-tpp-rs42-tcall', 'aaai21-transport-driving-rs42-tc1', 'aaai21-transport-driving-rs42-tc10', 'aaai21-transport-driving-rs42-tc12', 'aaai21-transport-driving-rs42-tc128', 'aaai21-transport-driving-rs42-tc16', 'aaai21-transport-driving-rs42-tc2', 'aaai21-transport-driving-rs42-tc25', 'aaai21-transport-driving-rs42-tc256', 'aaai21-transport-driving-rs42-tc3', 'aaai21-transport-driving-rs42-tc32', 'aaai21-transport-driving-rs42-tc4', 'aaai21-transport-driving-rs42-tc5', 'aaai21-transport-driving-rs42-tc50', 'aaai21-transport-driving-rs42-tc6', 'aaai21-transport-driving-rs42-tc64', 'aaai21-transport-driving-rs42-tc8', 'aaai21-transport-driving-rs42-tcall', 'aaai21-transport-rs42-tc1', 'aaai21-transport-rs42-tc10', 'aaai21-transport-rs42-tc12', 'aaai21-transport-rs42-tc128', 'aaai21-transport-rs42-tc16', 'aaai21-transport-rs42-tc2', 'aaai21-transport-rs42-tc25', 'aaai21-transport-rs42-tc256', 'aaai21-transport-rs42-tc3', 'aaai21-transport-rs42-tc32', 'aaai21-transport-rs42-tc4', 'aaai21-transport-rs42-tc5', 'aaai21-transport-rs42-tc50', 'aaai21-transport-rs42-tc6', 'aaai21-transport-rs42-tc64', 'aaai21-transport-rs42-tc8', 'aaai21-transport-rs42-tcall', 'aaai21-visitall-driving-rs42-tc1', 'aaai21-visitall-driving-rs42-tc10', 'aaai21-visitall-driving-rs42-tc12', 'aaai21-visitall-driving-rs42-tc128', 'aaai21-visitall-driving-rs42-tc16', 'aaai21-visitall-driving-rs42-tc2', 'aaai21-visitall-driving-rs42-tc25', 'aaai21-visitall-driving-rs42-tc3', 'aaai21-visitall-driving-rs42-tc32', 'aaai21-visitall-driving-rs42-tc4', 'aaai21-visitall-driving-rs42-tc5', 'aaai21-visitall-driving-rs42-tc50', 'aaai21-visitall-driving-rs42-tc6', 'aaai21-visitall-driving-rs42-tc64', 'aaai21-visitall-driving-rs42-tc8', 'aaai21-visitall-driving-rs42-tcall', 'aaai21-visitall-rs42-tc1', 'aaai21-visitall-rs42-tc10', 'aaai21-visitall-rs42-tc12', 'aaai21-visitall-rs42-tc128', 'aaai21-visitall-rs42-tc16', 'aaai21-visitall-rs42-tc2', 'aaai21-visitall-rs42-tc25', 'aaai21-visitall-rs42-tc3', 'aaai21-visitall-rs42-tc32', 'aaai21-visitall-rs42-tc4', 'aaai21-visitall-rs42-tc5', 'aaai21-visitall-rs42-tc50', 'aaai21-visitall-rs42-tc6', 'aaai21-visitall-rs42-tc64', 'aaai21-visitall-rs42-tc8', 'aaai21-visitall-rs42-tcall' ]


SUITE_AAAI18_COMPLETE = ['aaai18-logistics98-robustness-driving-rs42-tc1', 'aaai18-logistics98-robustness-driving-rs42-tc10', 'aaai18-logistics98-robustness-driving-rs42-tc1024', 'aaai18-logistics98-robustness-driving-rs42-tc12', 'aaai18-logistics98-robustness-driving-rs42-tc128', 'aaai18-logistics98-robustness-driving-rs42-tc16', 'aaai18-logistics98-robustness-driving-rs42-tc2', 'aaai18-logistics98-robustness-driving-rs42-tc2048', 'aaai18-logistics98-robustness-driving-rs42-tc25', 'aaai18-logistics98-robustness-driving-rs42-tc256', 'aaai18-logistics98-robustness-driving-rs42-tc3', 'aaai18-logistics98-robustness-driving-rs42-tc32', 'aaai18-logistics98-robustness-driving-rs42-tc4', 'aaai18-logistics98-robustness-driving-rs42-tc4096', 'aaai18-logistics98-robustness-driving-rs42-tc5', 'aaai18-logistics98-robustness-driving-rs42-tc50', 'aaai18-logistics98-robustness-driving-rs42-tc512', 'aaai18-logistics98-robustness-driving-rs42-tc6', 'aaai18-logistics98-robustness-driving-rs42-tc64', 'aaai18-logistics98-robustness-driving-rs42-tc8', 'aaai18-logistics98-robustness-rs42-tc1', 'aaai18-logistics98-robustness-rs42-tc10', 'aaai18-logistics98-robustness-rs42-tc1024', 'aaai18-logistics98-robustness-rs42-tc12', 'aaai18-logistics98-robustness-rs42-tc128', 'aaai18-logistics98-robustness-rs42-tc16', 'aaai18-logistics98-robustness-rs42-tc2', 'aaai18-logistics98-robustness-rs42-tc2048', 'aaai18-logistics98-robustness-rs42-tc25', 'aaai18-logistics98-robustness-rs42-tc256', 'aaai18-logistics98-robustness-rs42-tc3', 'aaai18-logistics98-robustness-rs42-tc32', 'aaai18-logistics98-robustness-rs42-tc4', 'aaai18-logistics98-robustness-rs42-tc4096', 'aaai18-logistics98-robustness-rs42-tc5', 'aaai18-logistics98-robustness-rs42-tc50', 'aaai18-logistics98-robustness-rs42-tc512', 'aaai18-logistics98-robustness-rs42-tc6', 'aaai18-logistics98-robustness-rs42-tc64', 'aaai18-logistics98-robustness-rs42-tc8', 'aaai18-no-mystery-robustness-driving-fuel-rs42-tc1', 'aaai18-no-mystery-robustness-driving-fuel-rs42-tc10', 'aaai18-no-mystery-robustness-driving-fuel-rs42-tc12', 'aaai18-no-mystery-robustness-driving-fuel-rs42-tc16', 'aaai18-no-mystery-robustness-driving-fuel-rs42-tc2', 'aaai18-no-mystery-robustness-driving-fuel-rs42-tc25', 'aaai18-no-mystery-robustness-driving-fuel-rs42-tc3', 'aaai18-no-mystery-robustness-driving-fuel-rs42-tc32', 'aaai18-no-mystery-robustness-driving-fuel-rs42-tc4', 'aaai18-no-mystery-robustness-driving-fuel-rs42-tc5', 'aaai18-no-mystery-robustness-driving-fuel-rs42-tc6', 'aaai18-no-mystery-robustness-driving-fuel-rs42-tc8', 'aaai18-no-mystery-robustness-driving-rs42-tc1', 'aaai18-no-mystery-robustness-driving-rs42-tc10', 'aaai18-no-mystery-robustness-driving-rs42-tc12', 'aaai18-no-mystery-robustness-driving-rs42-tc16', 'aaai18-no-mystery-robustness-driving-rs42-tc2', 'aaai18-no-mystery-robustness-driving-rs42-tc25', 'aaai18-no-mystery-robustness-driving-rs42-tc3', 'aaai18-no-mystery-robustness-driving-rs42-tc32', 'aaai18-no-mystery-robustness-driving-rs42-tc4', 'aaai18-no-mystery-robustness-driving-rs42-tc5', 'aaai18-no-mystery-robustness-driving-rs42-tc6', 'aaai18-no-mystery-robustness-driving-rs42-tc8', 'aaai18-no-mystery-robustness-rs42-tc1', 'aaai18-no-mystery-robustness-rs42-tc10', 'aaai18-no-mystery-robustness-rs42-tc12', 'aaai18-no-mystery-robustness-rs42-tc16', 'aaai18-no-mystery-robustness-rs42-tc2', 'aaai18-no-mystery-robustness-rs42-tc25', 'aaai18-no-mystery-robustness-rs42-tc3', 'aaai18-no-mystery-robustness-rs42-tc32', 'aaai18-no-mystery-robustness-rs42-tc4', 'aaai18-no-mystery-robustness-rs42-tc5', 'aaai18-no-mystery-robustness-rs42-tc6', 'aaai18-no-mystery-robustness-rs42-tc8', 'aaai18-pentesting-robustness-rs42-tc1', 'aaai18-pentesting-robustness-rs42-tc10', 'aaai18-pentesting-robustness-rs42-tc12', 'aaai18-pentesting-robustness-rs42-tc128', 'aaai18-pentesting-robustness-rs42-tc16', 'aaai18-pentesting-robustness-rs42-tc2', 'aaai18-pentesting-robustness-rs42-tc25', 'aaai18-pentesting-robustness-rs42-tc256', 'aaai18-pentesting-robustness-rs42-tc3', 'aaai18-pentesting-robustness-rs42-tc32', 'aaai18-pentesting-robustness-rs42-tc4', 'aaai18-pentesting-robustness-rs42-tc5', 'aaai18-pentesting-robustness-rs42-tc50', 'aaai18-pentesting-robustness-rs42-tc512', 'aaai18-pentesting-robustness-rs42-tc6', 'aaai18-pentesting-robustness-rs42-tc64', 'aaai18-pentesting-robustness-rs42-tc8', 'aaai18-pipesworld-notankage-robustness-rs42-tc1', 'aaai18-pipesworld-notankage-robustness-rs42-tc10', 'aaai18-pipesworld-notankage-robustness-rs42-tc12', 'aaai18-pipesworld-notankage-robustness-rs42-tc2', 'aaai18-pipesworld-notankage-robustness-rs42-tc3', 'aaai18-pipesworld-notankage-robustness-rs42-tc4', 'aaai18-pipesworld-notankage-robustness-rs42-tc5', 'aaai18-pipesworld-notankage-robustness-rs42-tc6', 'aaai18-pipesworld-notankage-robustness-rs42-tc8', 'aaai18-rovers-robustness-driving-rs42-tc1', 'aaai18-rovers-robustness-driving-rs42-tc10', 'aaai18-rovers-robustness-driving-rs42-tc12', 'aaai18-rovers-robustness-driving-rs42-tc128', 'aaai18-rovers-robustness-driving-rs42-tc16', 'aaai18-rovers-robustness-driving-rs42-tc2', 'aaai18-rovers-robustness-driving-rs42-tc25', 'aaai18-rovers-robustness-driving-rs42-tc256', 'aaai18-rovers-robustness-driving-rs42-tc3', 'aaai18-rovers-robustness-driving-rs42-tc32', 'aaai18-rovers-robustness-driving-rs42-tc4', 'aaai18-rovers-robustness-driving-rs42-tc5', 'aaai18-rovers-robustness-driving-rs42-tc50', 'aaai18-rovers-robustness-driving-rs42-tc512', 'aaai18-rovers-robustness-driving-rs42-tc6', 'aaai18-rovers-robustness-driving-rs42-tc64', 'aaai18-rovers-robustness-driving-rs42-tc8', 'aaai18-rovers-robustness-rs42-tc1', 'aaai18-rovers-robustness-rs42-tc10', 'aaai18-rovers-robustness-rs42-tc12', 'aaai18-rovers-robustness-rs42-tc128', 'aaai18-rovers-robustness-rs42-tc16', 'aaai18-rovers-robustness-rs42-tc2', 'aaai18-rovers-robustness-rs42-tc25', 'aaai18-rovers-robustness-rs42-tc256', 'aaai18-rovers-robustness-rs42-tc3', 'aaai18-rovers-robustness-rs42-tc32', 'aaai18-rovers-robustness-rs42-tc4', 'aaai18-rovers-robustness-rs42-tc5', 'aaai18-rovers-robustness-rs42-tc50', 'aaai18-rovers-robustness-rs42-tc512', 'aaai18-rovers-robustness-rs42-tc6', 'aaai18-rovers-robustness-rs42-tc64', 'aaai18-rovers-robustness-rs42-tc8', 'aaai18-sokoban-opt11-strips-robustness-rs42-tc1', 'aaai18-sokoban-opt11-strips-robustness-rs42-tc10', 'aaai18-sokoban-opt11-strips-robustness-rs42-tc12', 'aaai18-sokoban-opt11-strips-robustness-rs42-tc128', 'aaai18-sokoban-opt11-strips-robustness-rs42-tc16', 'aaai18-sokoban-opt11-strips-robustness-rs42-tc2', 'aaai18-sokoban-opt11-strips-robustness-rs42-tc25', 'aaai18-sokoban-opt11-strips-robustness-rs42-tc256', 'aaai18-sokoban-opt11-strips-robustness-rs42-tc3', 'aaai18-sokoban-opt11-strips-robustness-rs42-tc32', 'aaai18-sokoban-opt11-strips-robustness-rs42-tc4', 'aaai18-sokoban-opt11-strips-robustness-rs42-tc5', 'aaai18-sokoban-opt11-strips-robustness-rs42-tc50', 'aaai18-sokoban-opt11-strips-robustness-rs42-tc6', 'aaai18-sokoban-opt11-strips-robustness-rs42-tc64', 'aaai18-sokoban-opt11-strips-robustness-rs42-tc8', 'aaai18-sokoban-opt11-strips-robustness-walls-rs42-tc1', 'aaai18-sokoban-opt11-strips-robustness-walls-rs42-tc10', 'aaai18-sokoban-opt11-strips-robustness-walls-rs42-tc12', 'aaai18-sokoban-opt11-strips-robustness-walls-rs42-tc128', 'aaai18-sokoban-opt11-strips-robustness-walls-rs42-tc16', 'aaai18-sokoban-opt11-strips-robustness-walls-rs42-tc2', 'aaai18-sokoban-opt11-strips-robustness-walls-rs42-tc25', 'aaai18-sokoban-opt11-strips-robustness-walls-rs42-tc256', 'aaai18-sokoban-opt11-strips-robustness-walls-rs42-tc3', 'aaai18-sokoban-opt11-strips-robustness-walls-rs42-tc32', 'aaai18-sokoban-opt11-strips-robustness-walls-rs42-tc4', 'aaai18-sokoban-opt11-strips-robustness-walls-rs42-tc5', 'aaai18-sokoban-opt11-strips-robustness-walls-rs42-tc50', 'aaai18-sokoban-opt11-strips-robustness-walls-rs42-tc6', 'aaai18-sokoban-opt11-strips-robustness-walls-rs42-tc64', 'aaai18-sokoban-opt11-strips-robustness-walls-rs42-tc8', 'aaai18-tpp-robustness-driving-rs42-tc1', 'aaai18-tpp-robustness-driving-rs42-tc10', 'aaai18-tpp-robustness-driving-rs42-tc12', 'aaai18-tpp-robustness-driving-rs42-tc16', 'aaai18-tpp-robustness-driving-rs42-tc2', 'aaai18-tpp-robustness-driving-rs42-tc3', 'aaai18-tpp-robustness-driving-rs42-tc4', 'aaai18-tpp-robustness-driving-rs42-tc5', 'aaai18-tpp-robustness-driving-rs42-tc6', 'aaai18-tpp-robustness-driving-rs42-tc8', 'aaai18-tpp-robustness-rs42-tc1', 'aaai18-tpp-robustness-rs42-tc10', 'aaai18-tpp-robustness-rs42-tc12', 'aaai18-tpp-robustness-rs42-tc16', 'aaai18-tpp-robustness-rs42-tc2', 'aaai18-tpp-robustness-rs42-tc3', 'aaai18-tpp-robustness-rs42-tc4', 'aaai18-tpp-robustness-rs42-tc5', 'aaai18-tpp-robustness-rs42-tc6', 'aaai18-tpp-robustness-rs42-tc8', 'aaai18-visitall-opt14-strips-robustness-driving-rs42-tc1', 'aaai18-visitall-opt14-strips-robustness-driving-rs42-tc10', 'aaai18-visitall-opt14-strips-robustness-driving-rs42-tc1024', 'aaai18-visitall-opt14-strips-robustness-driving-rs42-tc12', 'aaai18-visitall-opt14-strips-robustness-driving-rs42-tc128', 'aaai18-visitall-opt14-strips-robustness-driving-rs42-tc16', 'aaai18-visitall-opt14-strips-robustness-driving-rs42-tc2', 'aaai18-visitall-opt14-strips-robustness-driving-rs42-tc25', 'aaai18-visitall-opt14-strips-robustness-driving-rs42-tc256', 'aaai18-visitall-opt14-strips-robustness-driving-rs42-tc3', 'aaai18-visitall-opt14-strips-robustness-driving-rs42-tc32', 'aaai18-visitall-opt14-strips-robustness-driving-rs42-tc4', 'aaai18-visitall-opt14-strips-robustness-driving-rs42-tc5', 'aaai18-visitall-opt14-strips-robustness-driving-rs42-tc50', 'aaai18-visitall-opt14-strips-robustness-driving-rs42-tc512', 'aaai18-visitall-opt14-strips-robustness-driving-rs42-tc6', 'aaai18-visitall-opt14-strips-robustness-driving-rs42-tc64', 'aaai18-visitall-opt14-strips-robustness-driving-rs42-tc8', 'aaai18-visitall-opt14-strips-robustness-rs42-tc1', 'aaai18-visitall-opt14-strips-robustness-rs42-tc10', 'aaai18-visitall-opt14-strips-robustness-rs42-tc1024', 'aaai18-visitall-opt14-strips-robustness-rs42-tc12', 'aaai18-visitall-opt14-strips-robustness-rs42-tc128', 'aaai18-visitall-opt14-strips-robustness-rs42-tc16', 'aaai18-visitall-opt14-strips-robustness-rs42-tc2', 'aaai18-visitall-opt14-strips-robustness-rs42-tc25', 'aaai18-visitall-opt14-strips-robustness-rs42-tc256', 'aaai18-visitall-opt14-strips-robustness-rs42-tc3', 'aaai18-visitall-opt14-strips-robustness-rs42-tc32', 'aaai18-visitall-opt14-strips-robustness-rs42-tc4', 'aaai18-visitall-opt14-strips-robustness-rs42-tc5', 'aaai18-visitall-opt14-strips-robustness-rs42-tc50', 'aaai18-visitall-opt14-strips-robustness-rs42-tc512', 'aaai18-visitall-opt14-strips-robustness-rs42-tc6', 'aaai18-visitall-opt14-strips-robustness-rs42-tc64', 'aaai18-visitall-opt14-strips-robustness-rs42-tc8']


SUITE_AAAI21 = [dom for dom in SUITE_AAAI21_COMPLETE if dom.endswith('all') or int(dom.split('tc')[-1])% 2 == 0]
SUITE_AAAI18 = [dom for dom in SUITE_AAAI18_COMPLETE if dom.endswith('all') or int(dom.split('tc')[-1])% 2 == 0]

SUITES = {"aaai18ipc" : [dom for dom in SUITE_AAAI18 if "pentesting" not in dom],
          "aaai21ipc" : [dom for dom in SUITE_AAAI21 if "pentesting" not in dom],
          "aaai18pentesting" : [dom for dom in SUITE_AAAI18 if "pentesting" in dom],
          "aaai21pentesting" : [dom for dom in SUITE_AAAI21 if "pentesting" in dom]
          "aaai21ipcfixed" : [dom.replace("driving", "drivingfixed")  for dom in SUITE_AAAI21 if "rovers-driving" in dom or "logistics-driving" in dom ]
          
}


class Config:    
    def __init__(self, folder, nick, config, revision, machines, preprocess_revision=PREPROCESS_REVISION_DEFAULT, suite = "aaai21ipc"):
        self.folder = folder
        self.nick = nick
        self.config = config
        self.revision = revision
        self.machines = machines
        self.preprocess_revision = preprocess_revision
        if "aaai21" in suite: 
            self.benchmarks_dir = "robustness-aaai21"
        else:
            self.benchmarks_dir = "robustness-aaai18"
        self.SUITE = SUITES[suite]
                        

    def __repr__(self):
        return ", ".join(map(str, [self.folder, self.nick, self.config,  self.revision, self.machines]))


    def with_soft_goals(self):
        return Config (self.folder + "-soft", self.nick + "-soft", self.config, self.revision, self.machines, PREPROCESS_REVISION_SOFT)

    def with_suite(self, suite):
        return Config (self.folder + "-" + suite, self.nick + "-" + suite, self.config, self.revision, self.machines, PREPROCESS_REVISION_DEFAULT, suite)





config_list = defaultdict(list)
config_list["symbolic_leader"] = [
    Config('ss-sbd', 'ss-sbd', ["--search", "sym_stackelberg(optimal_engine=symbolic(plan_reuse_minimal_task_upper_bound=false, plan_reuse_upper_bound=false), upper_bound_pruning=false)"], REVISION, SERVERS),
    # Config('ss-sbd-up', 'ss-sbd-up', ["--search", "sym_stackelberg(optimal_engine=symbolic(plan_reuse_minimal_task_upper_bound=false, plan_reuse_upper_bound=false), upper_bound_pruning=true)"], REVISION, SERVERS), 
    Config('ss-sbd-ubreuse', 'ss-sbd-ubreuse', ["--search", "sym_stackelberg(optimal_engine=symbolic(plan_reuse_minimal_task_upper_bound=false, plan_reuse_upper_bound=true), upper_bound_pruning=false)"], REVISION, SERVERS), 
    # Config('ss-sbd-up-ubreuse-tlim', 'ss-sbd-up-ubreuse-tlim', ["--search", "sym_stackelberg(optimal_engine=symbolic(plan_reuse_minimal_task_upper_bound=true, plan_reuse_upper_bound=true, force_bw_search_minimum_task_seconds=30, time_limit_seconds_minimum_task=300), upper_bound_pruning=true)"], REVISION, SERVERS),
]

config_list["baseline"] = [
    Config('original-lmcut-pdbs', 'original-lmcut-pdbs', ["--heuristic", "h1=deadpdbs(max_time=120)", "--heuristic", "h2=lmcut", "--search", "fixsearch(search_engine=astar(max([h1,h2]), pruning=null), attack_heuristic=h2, initial_attack_budget=2147483647, initial_fix_budget=2147483647, attack_op_dom_pruning=false, sort_fix_ops=true, greedy=false, upper_bound_pruning=true)"], REVISION, SERVERS),
    Config('baseline-lmcut-pdbs', 'baseline-lmcut-pdbs', ["--heuristic", "h1=deadpdbs(max_time=120)", "--heuristic", "h2=lmcut", "--search", "stackelberg(search_engine=astar(max([h1,h2])), follower_heuristic=h2)"], REVISION, SERVERS),
    Config('baseline-lmcut', 'baseline-lmcut', ["--heuristic", "h2=lmcut", "--search", "stackelberg(search_engine=astar(h2), follower_heuristic=h2)"], REVISION, SERVERS),
    Config('baseline-sbd', 'baseline-sbd', ["--search", "stackelberg(search_engine=sbd())"], REVISION, SERVERS)
]

config_list["symbolic_leader_lmcut"] = [
    Config('ss-lmcut', 'ss-lmcut', ["--search", "sym_stackelberg(optimal_engine=explicit(search_engine=astar(lmcut()), is_optimal_solver=true, plan_reuse_upper_bound=false), upper_bound_pruning=false)"], REVISION, SERVERS),
    Config('ss-lmcut-pdbs', 'ss-lmcut-pdbs', ["--search", "sym_stackelberg(optimal_engine=explicit(search_engine=astar(max([deadpdbs(max_time=120),lmcut])), is_optimal_solver=true, plan_reuse_upper_bound=false), upper_bound_pruning=false)"], REVISION, SERVERS),
    Config('ss-lmcut-ubreuse', 'ss-lmcut-ubreuse', ["--search", "sym_stackelberg(optimal_engine=explicit(search_engine=astar(lmcut()), is_optimal_solver=true, plan_reuse_upper_bound=true), upper_bound_pruning=false)"], REVISION, SERVERS),
    Config('ss-lmcut-pdbs-ubreuse', 'ss-lmcut-pdbs-ubresuse', ["--search", "sym_stackelberg(optimal_engine=explicit(search_engine=astar(max([deadpdbs(max_time=120),lmcut])), is_optimal_solver=true, plan_reuse_upper_bound=true), upper_bound_pruning=false)"], REVISION, SERVERS),
    ]



for (cb_name, cb_engine) in [("cbff-1s","explicit(search_engine=eager_greedy(ff(), max_time=1, use_heuristics_for_bound_pruning=false), is_optimal_solver=false)"),
                             ("cbff-10s","explicit(search_engine=eager_greedy(ff(), max_time=10, use_heuristics_for_bound_pruning=false), is_optimal_solver=false)"),
                             ("cbffpr-10s","explicit(search_engine=eager_greedy(ff(), max_time=10, use_heuristics_for_bound_pruning=true), is_optimal_solver=false)"),]:

    config_list["symbolic_leader_cb"] += [Config('ss-sbd-{}'.format(cb_name), 'ss-sbd-{}'.format(cb_name), ["--search", "sym_stackelberg(optimal_engine=symbolic(plan_reuse_minimal_task_upper_bound=false, plan_reuse_upper_bound=false), cost_bounded_engine={}, upper_bound_pruning=false)".format(cb_engine)], REVISION, SERVERS),
                                          Config('ss-sbd-ubreuse-{}'.format(cb_name), 'ss-sbd-ubreuse-{}'.format(cb_name), ["--search", "sym_stackelberg(optimal_engine=symbolic(plan_reuse_minimal_task_upper_bound=false, plan_reuse_upper_bound=true), cost_bounded_engine={}, upper_bound_pruning=false)".format(cb_engine)], REVISION, SERVERS), 
    ]



CONFIGS = defaultdict(list)
for k in config_list:
    for config in config_list[k]:
        for suite in SUITES:
            print (k, suite)
            config_suite = config.with_suite(suite)
            CONFIGS[k].append(config_suite)
            if "aaai21" in suite: 
                CONFIGS[k].append(config_suite.with_soft_goals())
            

def get_configs(experiment):
    if experiment == "all":
        res = []
        for a in CONFIGS:
            for d in CONFIGS[a]:
                res.append(d)
        return res
    else:
        return CONFIGS[experiment]



for conf in CONFIGS:
    print conf
    for aux in CONFIGS[conf]:
        print aux
        print
        print
    
