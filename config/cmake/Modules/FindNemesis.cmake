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
# 
#  
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


#
# Jali Nemesis Find Module
#
# Usage:
#    Control the search through Nemesis_DIR or setting environment variable
#    Nemesis_ROOT to the Nemesis installation prefix.
#
#    This module does not search default paths! 
#
#    Following variables are set:
#    Nemesis_FOUND            (BOOL)       Flag indicating if Nemesis was found
#    Nemesis_INCLUDE_DIR      (PATH)       Path to the Nemesis include file
#    Nemesis_INCLUDE_DIRS     (LIST)       List of all required include files
#    Nemesis_LIBRARY_DIR      (PATH)       Path to the Nemesis library
#    Nemesis_LIBRARY          (FILE)       Nemesis library
#    Nemesis_LIBRARIES        (LIST)       List of all required Nemesis libraries
#
#    Additional variables
#    Nemesis_VERSION          (STRING)     Nemesis Version string
#
# #############################################################################

# Standard CMake modules see CMAKE_ROOT/Modules
include(FindPackageHandleStandardArgs)

include(AddImportedLibrary)

# Jali CMake functions see <root>/tools/cmake for source
include(PrintVariable)
include(AddPackageDependency)

if ( Nemesis_LIBRARIES AND Nemesis_INCLUDE_DIRS )

    # Do nothing. Variables are set. No need to search again

else(Nemesis_LIBRARIES AND Nemesis_INCLUDE_DIRS)

    # Cache variables
    if(Nemesis_DIR)
        set(Nemesis_DIR "${Nemesis_DIR}" CACHE PATH "Path to search for Nemesis include and library files")
    endif()

    if(Nemesis_INCLUDE_DIR)
        set(Nemesis_INCLUDE_DIR "${Nemesis_INCLUDE_DIR}" CACHE PATH "Path to search for Nemesis include files")
    endif()

    if(Nemesis_LIBRARY_DIR)
        set(Nemesis_LIBRARY_DIR "${Nemesis_LIBRARY_DIR}" CACHE PATH "Path to search for Nemesis library files")
    endif()

    
    # Search for include files
    # Search order preference:
    #  (1) Nemesis_INCLUDE_DIR - check existence of path AND if the include files exist
    #  (2) Nemesis_DIR/<include>
    #  (3) Default CMake paths See cmake --html-help=out.html file for more information.
    #
    set(nemesis_inc_names "ne_nemesisI.h")
    if (Nemesis_INCLUDE_DIR)

        if (EXISTS "${Nemesis_INCLUDE_DIR}")

            find_path(nemesis_test_include_path
                      NAMES ${nemesis_inc_names}
                      HINTS ${Nemesis_INCLUDE_DIR}
                      NO_DEFAULT_PATH)
            if(NOT nemesis_test_include_path)
                message(SEND_ERROR "Can not locate ${nemesis_inc_names} in ${Nemesis_INCLUDE_DIR}")
            endif()
            set(Nemesis_INCLUDE_DIR "${nemesis_test_include_path}")

        else()
            message(SEND_ERROR "Nemesis_INCLUDE_DIR=${Nemesis_INCLUDE_DIR} does not exist")
            set(Nemesis_INCLUDE_DIR "Nemesis_INCLUDE_DIR-NOTFOUND")
        endif()

   else() 

        set(nemesis_inc_suffixes "include")
        if(Nemesis_DIR)

            if (EXISTS "${Nemesis_DIR}" )

                find_path(Nemesis_INCLUDE_DIR
                          NAMES ${nemesis_inc_names}
                          HINTS ${Nemesis_DIR}
                          PATH_SUFFIXES ${nemesis_inc_suffixes}
                          NO_DEFAULT_PATH)

            else()
                 message(SEND_ERROR "Nemesis_DIR=${Nemesis_DIR} does not exist")
                 set(Nemesis_INCLUDE_DIR "Nemesis_INCLUDE_DIR-NOTFOUND")
            endif()    


        else()

            find_path(Nemesis_INCLUDE_DIR
                      NAMES ${nemesis_inc_names}
                      PATH_SUFFIXES ${nemesis_inc_suffixes})

        endif()

    endif()

    if ( NOT Nemesis_INCLUDE_DIR )
        message(SEND_ERROR "Can not locate Nemesis include directory")
    endif()

    # Search for libraries 
    # Search order preference:
    #  (1) Nemesis_LIBRARY_DIR - check existence of path AND if the library file exists
    #  (2) Nemesis_DIR/<lib,Lib>
    #  (3) Default CMake paths See cmake --html-help=out.html file for more information.
    #
    set(nemesis_lib_names "nemesis")
    if (Nemesis_LIBRARY_DIR)

        if (EXISTS "${Nemesis_LIBRARY_DIR}")

            find_library(_Nemesis_LIBRARY
                         NAMES ${nemesis_lib_names}
                         HINTS ${Nemesis_LIBRARY_DIR}
                         NO_DEFAULT_PATH)

        else()
            message(SEND_ERROR "Nemesis_LIBRARY_DIR=${Nemesis_LIBRARY_DIR} does not exist")
            set(_Nemesis_LIBRARY "Nemesis_LIBRARY-NOTFOUND")
        endif()

    else() 

        list(APPEND nemesis_lib_suffixes "lib" "Lib")
        if(Nemesis_DIR)

            if (EXISTS "${Nemesis_DIR}" )

                find_library(_Nemesis_LIBRARY
                             NAMES ${nemesis_lib_names}
                             HINTS ${Nemesis_DIR}
                             PATH_SUFFIXES ${nemesis_lib_suffixes}
                             NO_DEFAULT_PATH)
                
            else()
                 message(SEND_ERROR "Nemesis_DIR=${Nemesis_DIR} does not exist")
                 set(Nemesis_LIBRARY "Nemesis_LIBRARY-NOTFOUND")
            endif()    


        else()

            find_library(_Nemesis_LIBRARY
                         NAMES ${nemesis_lib_names}
                         PATH_SUFFIXES ${nemesis_lib_suffixes})

        endif()

    endif()

    # Create the library target store the name in Nemesis_LIBRARY
    if ( _Nemesis_LIBRARY )
        set(Nemesis_LIBRARY nemesis)
        add_imported_library(${Nemesis_LIBRARY}
                     LOCATION ${_Nemesis_LIBRARY}
                     LINK_LANGUAGES "C")
    else()
        message(SEND_ERROR "Can not locate Nemesis library")
    endif()

    # Define prerequisite packages
    set(Nemesis_INCLUDE_DIRS ${Nemesis_INCLUDE_DIR})
    set(Nemesis_LIBRARIES    ${Nemesis_LIBRARY})

    # Search for ExodusII
    find_package(ExodusII QUIET REQUIRED)
    set_target_properties(${Nemesis_LIBRARY} PROPERTIES
                          IMPORTED_LINK_INTERFACE_LIBRARIES "${ExodusII_LIBRARY}")
    list(APPEND Nemesis_INCLUDE_DIRS ${ExodusII_INCLUDE_DIRS})

   
endif(Nemesis_LIBRARIES AND Nemesis_INCLUDE_DIRS )    

# Define the version
if ( Nemesis_INCLUDE_DIR )
    set(nemesis_h ${Nemesis_INCLUDE_DIR}/ne_nemesisI.h)
    file(STRINGS ${nemesis_h} nemesis_version_string REGEX "^\#define NEMESIS_API_VERSION[ \t]+")
    string(REGEX REPLACE ".+NEMESIS_API_VERSION[ \t]+([0-9]+\\.[0-9]+).*$" "\\1" nemesis_version "${nemesis_version_string}")

    #PRINT_VARIABLE(nemesis_version_string)
    #PRINT_VARIABLE(nemesis_version)

    set(Nemesis_VERSION "${nemesis_version}")

endif()    

# Send useful message if everything is found
find_package_handle_standard_args(Nemesis DEFAULT_MSG
                                           Nemesis_INCLUDE_DIR
                                           Nemesis_LIBRARIES)

# find_package)handle)standard_args should set Nemesis_FOUND but it does not!
if ( Nemesis_LIBRARIES AND Nemesis_INCLUDE_DIRS)
    set(Nemesis_FOUND TRUE)
else()
    set(Nemesis_FOUND FALSE)
endif()


mark_as_advanced(
  Nemesis_VERSION
  Nemesis_INCLUDE_DIR
  Nemesis_INCLUDE_DIRS
  Nemesis_LIBRARY
  Nemesis_LIBRARIES
  Nemesis_LIBRARY_DIR
)
