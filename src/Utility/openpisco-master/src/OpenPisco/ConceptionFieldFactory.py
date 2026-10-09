# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

from Muscat.Helpers.Factory import Factory

def RegisterClass(name, classtype, constructor=None, withError = True):
    return ConceptionFieldFactory.RegisterClass(name,classtype, constructor=constructor, withError = withError )

def Create(name,ops=None):
   dims=  ops.get('dimensionality',3)
   name += str(dims)+'D'
   if 'dimensionality' in ops:
       del ops['dimensionality']
   if ops['support'].props.get("IsConstantRectilinear",False):
       name += "S"
   else:
       name += "U"
   return ConceptionFieldFactory.Create(name,ops)


class ConceptionFieldFactory(Factory):
    _Catalog = {}
    _SetCatalog = set()

    def __init__(self):
        super(ConceptionFieldFactory,self).__init__()

def InitAllConceptionField():
      import OpenPisco.Structured.Levelset2D
      import OpenPisco.Structured.Levelset3D
      import OpenPisco.Unstructured.Levelset

def CheckIntegrity():
    return "OK"

if __name__ == '__main__':
        print(CheckIntegrity())
