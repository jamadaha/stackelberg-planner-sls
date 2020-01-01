#!/usr/bin/env python
# encoding: utf-8

class Config:    
    def __init__(self, folder, nick, config, revision, machines):
        self.folder = folder
        self.nick = nick
        self.config = config
        self.revision = revision
        self.machines = machines

    def __repr__(self):
        return ", ".join(map(str, [self.folder, self.nick, self.config,  self.revision, self.machines]))



import baseline




CONFIGS = {}
for config_list in [baseline.CONFIGS]:
    for k in config_list:
        CONFIGS[k] = config_list[k]


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
