# Role-level namespaces for the consolidated engine.
#
# Target names do NOT change in TB.1: EterBase, EterLib, … stay, and the
# existing Foo::Foo aliases keep working. These INTERFACE targets are the names
# consumers should use from now on (TB.3 shim) and the only names the script
# bridges and the game layer may link against.

function(_neoeter_role _name)
    if(TARGET Neoeter::${_name})
        return()
    endif()
    add_library(Neoeter_${_name} INTERFACE)
    add_library(Neoeter::${_name} ALIAS Neoeter_${_name})
    foreach(_dep IN LISTS ARGN)
        if(TARGET ${_dep})
            target_link_libraries(Neoeter_${_name} INTERFACE ${_dep})
        endif()
    endforeach()
endfunction()

_neoeter_role(Platform EterBase EterLocale SphereLib)
_neoeter_role(IO       EterPack)
_neoeter_role(Image    EterImageLib)
_neoeter_role(Render   EterLib)
_neoeter_role(Audio    MilesLib)
_neoeter_role(Model    EterGrnLib SpeedTreeLib)
_neoeter_role(Terrain  PRTerrainLib)
_neoeter_role(Effect   EffectLib)
