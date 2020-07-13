#!/usr/bin/env python2
# encoding: utf-8

from collections import defaultdict


PREPROCESS_REVISION_DEFAULT = "92845ada3fd61a733f99702ffcfff2b3b99260f0"

PREPROCESS_REVISION_SOFT = "6f1b5abf91f0b5a9f7dace3fea8ffbedcff3c7dc"


class Config:    
    def __init__(self, folder, nick, config, revision, machines, preprocess_revision=PREPROCESS_REVISION_DEFAULT):
        self.folder = folder
        self.nick = nick
        self.config = config
        self.revision = revision
        self.machines = machines
        self.preprocess_revision = preprocess_revision

    def __repr__(self):
        return ", ".join(map(str, [self.folder, self.nick, self.config,  self.revision, self.machines]))


    def with_soft_goals(self):
        return Config (self.folder + "-soft", self.nick + "-soft", self.config, self.revision, self.machines, PREPROCESS_REVISION_SOFT)


import baseline
import symbolic_leader
import symbolic_leader_lmcut


CONFIGS = defaultdict(list)
for config_list in [baseline.CONFIGS, symbolic_leader.CONFIGS, symbolic_leader_lmcut.CONFIGS]:
    for k in config_list:
        for config in config_list[k]:
            CONFIGS[k].append(config)
            CONFIGS[k].append(config.with_soft_goals())

            


def get_configs(experiment):
    if experiment == "all":
        res = []
        for a in CONFIGS:
            for d in CONFIGS[a]:
                res.append(d)
        return res
    else:
        return CONFIGS[experiment]



# for conf in CONFIGS:
#     print conf
#     print ""
