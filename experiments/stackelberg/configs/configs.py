#!/usr/bin/env python
# encoding: utf-8

from collections import defaultdict


PREPROCESS_REVISION_DEFAULT = "f503350c800543392b4054ca91b074252776dd34"

PREPROCESS_REVISION_SOFT = "0ab0299b306776810922f24fe1c65df7bf82d3f8"


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


CONFIGS = defaultdict(list)
for config_list in [baseline.CONFIGS, symbolic_leader.CONFIGS]:
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
