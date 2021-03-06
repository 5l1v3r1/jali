# Copyright (c) 2019, Triad National Security, LLC
# All rights reserved.

# Copyright 2019. Triad National Security, LLC. This software was
# produced under U.S. Government contract 89233218CNA000001 for Los
# Alamos National Laboratory (LANL), which is operated by Triad
# National Security, LLC for the U.S. Department of Energy. 
# All rights in the program are reserved by Triad National Security,
# LLC, and the U.S. Department of Energy/National Nuclear Security
# Administration. The Government is granted for itself and others acting
# on its behalf a nonexclusive, paid-up, irrevocable worldwide license
# in this material to reproduce, prepare derivative works, distribute
# copies to the public, perform publicly and display publicly, and to
# permit others to do so
 
# 
# This is open source software distributed under the 3-clause BSD license.
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
# 
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of Triad National Security, LLC, Los Alamos
#    National Laboratory, LANL, the U.S. Government, nor the names of its
#    contributors may be used to endorse or promote products derived from this
#    software without specific prior written permission.

 
# THIS SOFTWARE IS PROVIDED BY TRIAD NATIONAL SECURITY, LLC AND
# CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
# BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
# FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
# TRIAD NATIONAL SECURITY, LLC OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.



################################################################################

import os, sys
import types

from trilinos import ParameterList

################################################################################
class BoxInterface(ParameterList):

    def __init__(self,lo=[],hi=[],file=None):

        ParameterList.__init__(self,"box",file)

        if len(lo) != 0:
            self.add_parameter("lo",lo)

        if len(hi) != 0:
            self.add_parameter("hi",hi)


class RegionsInterface(ParameterList):

    def __init__(self,file=None):

        ParameterList.__init__(self,"Regions",file)

    def add_region(self,label=None,lo=[],hi=[]):

        if label == None:
            raise ValueError, 'Must define a label to add a region'

        if len(lo) == 0:
            raise ValueError, 'Must define a lower bound region'

        if len(hi) == 0:
            raise ValueError, 'Must define an upper bound region'

        new_region = self.add_sublist(label)
        box = BoxInterface(lo,hi)
        new_region.add_sublist(box)

        return new_region


    
################################################################################
def RegionInputList(file=None):

    return RegionsInterface(file)



################################################################################
if __name__ == '__main__':

    # Example based on the deep vadose example

    regions = RegionInputList()

    # Rwia region
    regions.add_region('Rwia region',[0.0,0.0,0.0],[103.2,0.0,6.0])

    # Rlm region
    regions.add_region('Rlm region',[0.0,0.0,6.0],[103.2,0.0,11.4])

    # CCug region
    regions.add_region('CCug region',[0.0,0.0,11.4],[103.2,0.0,18.0])

    # CCuz region
    regions.add_region('CCuz region',[0.0,0.0,18.0],[103.2,0.0,22.2])

    regions.dumpXML()


