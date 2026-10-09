# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
import os
import sys
from typing import Dict

import Muscat.Helpers.ParserHelper as PH

def PluginLoader(name:str,ops:Dict):
    import importlib

    if name != "Plugin":
        print("Are you sure this is a plugin : " + str(name) + " , " + str(ops) )

    load = PH.ReadBool(ops.get("load",True))
    if load :
        if "name" in ops:
            name = PH.ReadString(ops["name"])
            plugin = importlib.import_module(name)

        if "path" in ops:
            path = os.path.abspath(PH.ReadString(ops["path"]))
            print(path)
            spec = importlib.util.spec_from_file_location("module.name", path)
            plugin = importlib.util.module_from_spec(spec)
            sys.modules["module.name"] = plugin
            spec.loader.exec_module(plugin)

        assert plugin is not None, "Plugin '" + name +"' not Loaded correctly"
        print("Plugin '" + name +"' Loaded correctly")

def CheckIntegrity():
    return "OK"

if __name__ == '__main__':
        print(CheckIntegrity())
