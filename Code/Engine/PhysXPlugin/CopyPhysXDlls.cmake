
# Function parameter output for debugging:
#message(STATUS "${MSVC_BUILD_CONFIG_DIR}")
#message(STATUS "${BUILDSYSTEM_PLATFORM_64BIT}")
#message(STATUS "${PHYSX_SOURCE}")
#message(STATUS "${PHYSX_TARGET}")

if (BUILDSYSTEM_PLATFORM_64BIT STREQUAL "ON")
  SET (NVTOOLS_EXT "64_1.dll")
else ()
  SET (NVTOOLS_EXT "32_1.dll")
endif ()

file(COPY "${PHYSX_SOURCE}/nvToolsExt${NVTOOLS_EXT}"
        DESTINATION "${PHYSX_TARGET}/")

SET (PHYSX_EXT "")
if (MSVC_BUILD_CONFIG_DIR STREQUAL "Debug")
  SET (PHYSX_EXT "${PHYSX_EXT}DEBUG")
endif ()

if (BUILDSYSTEM_PLATFORM_64BIT STREQUAL "ON")
  SET (PHYSX_EXT "${PHYSX_EXT}_x64.dll")
else ()
  SET (PHYSX_EXT "${PHYSX_EXT}_x86.dll")
endif ()

file(COPY "${PXSHARED_SOURCE}/PxFoundation${PHYSX_EXT}"
        DESTINATION "${PHYSX_TARGET}/")
file(COPY "${PXSHARED_SOURCE}/PxPvdSDK${PHYSX_EXT}"
        DESTINATION "${PHYSX_TARGET}/")

file(COPY "${PHYSX_SOURCE}/PhysX3CharacterKinematic${PHYSX_EXT}"
        DESTINATION "${PHYSX_TARGET}/")
file(COPY "${PHYSX_SOURCE}/PhysX3Common${PHYSX_EXT}"
        DESTINATION "${PHYSX_TARGET}/")
file(COPY "${PHYSX_SOURCE}/PhysX3Cooking${PHYSX_EXT}"
        DESTINATION "${PHYSX_TARGET}/")
file(COPY "${PHYSX_SOURCE}/PhysX3${PHYSX_EXT}"
        DESTINATION "${PHYSX_TARGET}/")