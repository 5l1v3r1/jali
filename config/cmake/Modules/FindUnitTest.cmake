# Copyright (c) 2017, Los Alamos National Security, LLC
# All rights reserved.

# Copyright 2017. Los Alamos National Security, LLC. This software was
# produced under U.S. Government contract DE-AC52-06NA25396 for Los
# Alamos National Laboratory (LANL), which is operated by Los Alamos
# National Security, LLC for the U.S. Department of Energy. The
# U.S. Government has rights to use, reproduce, and distribute this
# software.  NEITHER THE GOVERNMENT NOR LOS ALAMOS NATIONAL SECURITY,
# LLC MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LIABILITY
# FOR THE USE OF THIS SOFTWARE.  If software is modified to produce
# derivative works, such modified software should be clearly marked, so
# as not to confuse it with the version available from LANL.
 
# Additionally, redistribution and use in source and binary forms, with
# or without modification, are permitted provided that the following
# conditions are met:

# 1.  Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
# 3.  Neither the name of Los Alamos National Security, LLC, Los Alamos
# National Laboratory, LANL, the U.S. Government, nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
 
# THIS SOFTWARE IS PROVIDED BY LOS ALAMOS NATIONAL SECURITY, LLC AND
# CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
# BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
# FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LOS
# ALAMOS NATIONAL SECURITY, LLC OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#
# Jali UnitTest Find Module
#
# Usage:
#    Control the search through UnitTest_DIR or setting environment variable
#    UnitTest_ROOT to the UnitTest installation prefix.
#
#    This module does not search default paths! 
#
#    Following variables are set:
#    UnitTest_FOUND            (BOOL)       Flag indicating if UnitTest was found
#    UnitTest_INCLUDE_DIR      (PATH)       Path to the UnitTest include file
#    UnitTest_INCLUDE_DIRS     (LIST)       List of all required include files
#    UnitTest_LIBRARY_DIR      (PATH)       Path to the UnitTest library
#    UnitTest_LIBRARY          (FILE)       UnitTest library
#    UnitTest_LIBRARIES        (LIST)       List of all required UnitTest libraries
#
# #############################################################################

# Standard CMake modules see CMAKE_ROOT/Modules
include(FindPackageHandleStandardArgs)

# Jali CMake functions see <root>/tools/cmake for source
include(PrintVariable)

if ( UnitTest_LIBRARIES AND UnitTest_INCLUDE_DIRS )

    # Do nothing. Variables are set. No need to search again

else(UnitTest_LIBRARIES AND UnitTest_INCLUDE_DIRS)

    # Cache variables
    if(UnitTest_DIR)
        set(UnitTest_DIR "${UnitTest_DIR}" CACHE PATH "Path to search for UnitTest include and library files")
    endif()

    if(UnitTest_INCLUDE_DIR)
        set(UnitTest_INCLUDE_DIR "${UnitTest_INCLUDE_DIR}" CACHE PATH "Path to search for UnitTest include files")
    endif()

    if(UnitTest_LIBRARY_DIR)
        set(UnitTest_LIBRARY_DIR "${UnitTest_LIBRARY_DIR}" CACHE PATH "Path to search for UnitTest library files")
    endif()

    
    # Search for include files
    # Search order preference:
    #  (1) UnitTest_INCLUDE_DIR - check existence of path AND if the include files exist
    #  (2) UnitTest_DIR/<include,include/UnitTest++,include/unittest++>
    #  (3) Default CMake paths See cmake --html-help=out.html file for more information.
    #
    set(unittest_inc_names "UnitTest++.h" "unittest++.h")
    set(unittest_inc_suffixes "UnitTest++" "unittest++")
    if (UnitTest_INCLUDE_DIR)

        if (EXISTS "${UnitTest_INCLUDE_DIR}")

            find_path(UnitTest_test_include_path
                      NAMES ${unittest_inc_names}
                      HINTS ${UnitTest_INCLUDE_DIR}
                      PATH_SUFFIXES ${unittest_inc_suffixes}
                      NO_DEFAULT_PATH)
            if(NOT UnitTest_test_include_path)
                message(SEND_ERROR "Can not locate ${unittest_inc_names} in ${UnitTest_INCLUDE_DIR} "
                                   "with suffixes ${unittest_inc_suffixes}")
            endif()
             set(UnitTest_INCLUDE_DIR "${UnitTest_test_include_path}")

        else()
            message(SEND_ERROR "UnitTest_INCLUDE_DIR=${UnitTest_INCLUDE_DIR} does not exist")
            set(UnitTest_INCLUDE_DIR "UnitTest_INCLUDE_DIR-NOTFOUND")
        endif()

    else() 

        list(APPEND unittest_inc_suffixes "include" "include/UnitTest++" "include/unittest++")
        if(UnitTest_DIR)

            if (EXISTS "${UnitTest_DIR}" )

                find_path(UnitTest_INCLUDE_DIR
                          NAMES ${unittest_inc_names}
                          HINTS ${UnitTest_DIR}
                          PATH_SUFFIXES ${unittest_inc_suffixes}
                          NO_DEFAULT_PATH)

            else()
                 message(SEND_ERROR "UnitTest_DIR=${UnitTest_DIR} does not exist")
                 set(UnitTest_INCLUDE_DIR "UnitTest_INCLUDE_DIR-NOTFOUND")
            endif()    


        else()

            find_path(UnitTest_INCLUDE_DIR
                      NAMES ${unittest_inc_names}
                      PATH_SUFFIXES ${unittest_inc_suffixes})

        endif()

    endif()

    if ( NOT UnitTest_INCLUDE_DIR )
        message(SEND_ERROR "Can not locate UnitTest include directory")
    endif()

    # Search for libraries 
    # Search order preference:
    #  (1) UnitTest_LIBRARY_DIR - check existence of path AND if the include files exist
    #  (2) UnitTest_DIR/<lib,lib/UnitTest++,lib/unittest++>
    #  (3) Default CMake paths See cmake --html-help=out.html file for more information.
    #
    set(unittest_lib_names "UnitTest++" "unittest++")
    set(unittest_lib_suffixes "UnitTest++" "unittest++")
    if (UnitTest_LIBRARY_DIR)

        if (EXISTS "${UnitTest_LIBRARY_DIR}")

            find_library(UnitTest_LIBRARY
                         NAMES ${unittest_lib_names}
                         HINTS ${UnitTest_LIBRARY_DIR}
                         PATH_SUFFIXES ${unittest_lib_suffixes}
                         NO_DEFAULT_PATH)
        else()
            message(SEND_ERROR "UnitTest_LIBRARY_DIR=${UnitTest_LIBRARY_DIR} does not exist")
            set(UnitTest_LIBRARY "UnitTest_LIBRARY-NOTFOUND")
        endif()

    else() 

        list(APPEND unittest_lib_suffixes "lib" "lib/UnitTest++" "lib/unittest++")
        list(APPEND unittest_Lib_suffixes "Lib" "Lib/UnitTest++" "Lib/unittest++")
        if(UnitTest_DIR)

            if (EXISTS "${UnitTest_DIR}" )

                find_library(UnitTest_LIBRARY
                             NAMES ${unittest_lib_names}
                             HINTS ${UnitTest_DIR}
                             PATH_SUFFIXES ${unittest_lib_suffixes}
                             NO_DEFAULT_PATH)

            else()
                 message(SEND_ERROR "UnitTest_DIR=${UnitTest_DIR} does not exist")
                 set(UnitTest_LIBRARY "UnitTest_LIBRARY-NOTFOUND")
            endif()    


        else()

            find_library(UnitTest_LIBRARY
                         NAMES ${unittest_lib_names}
                         PATH_SUFFIXES ${unittest_lib_suffixes})

        endif()

    endif()

    if ( NOT UnitTest_LIBRARY )
        message(SEND_ERROR "Can not locate UnitTest library")
    endif()    

   
    # UnitTest does not have any prerequisite libraries
    set(UnitTest_INCLUDE_DIRS ${UnitTest_INCLUDE_DIR})
    set(UnitTest_LIBRARIES    ${UnitTest_LIBRARY})

   
endif(UnitTest_LIBRARIES AND UnitTest_INCLUDE_DIRS )    

# Send useful message if everything is found
find_package_handle_standard_args(UnitTest DEFAULT_MSG
                                           UnitTest_LIBRARIES
                                           UnitTest_INCLUDE_DIRS)

# find_package)handle)standard_args should set UnitTest_FOUND but it does not!
if ( UnitTest_LIBRARIES AND UnitTest_INCLUDE_DIRS)
    set(UnitTest_FOUND TRUE)
else()
    set(UnitTest_FOUND FALSE)
endif()

mark_as_advanced(
  UnitTest_INCLUDE_DIR
  UnitTest_INCLUDE_DIRS
  UnitTest_LIBRARY
  UnitTest_LIBRARIES
  UnitTest_LIBRARY_DIR
)
