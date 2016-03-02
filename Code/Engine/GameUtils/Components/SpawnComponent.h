#pragma once

#include <GameUtils/Basics.h>
#include <Core/World/World.h>
#include <Core/World/Component.h>
#include <Foundation/Time/Time.h>
#include <GameUtils/Prefabs/PrefabResource.h>

struct ezInputComponentMessage;

struct ezSpawnComponentFlags
{
  typedef ezUInt16 StorageType;

  enum Enum
  {
    None = 0,
    SpawnAtStart = EZ_BIT(0), ///< The component will schedule a spawn once at creation time
    SpawnContinuously = EZ_BIT(1), ///< Every time a scheduled spawn was done, a new one is scheduled
    AttachAsChild = EZ_BIT(2), ///< All objects spawned will be attached as children to this node
    SpawnInFlight = EZ_BIT(3), ///< [internal] A spawn trigger message has been posted.

    Default = None
  };

  struct Bits
  {
    StorageType SpawnAtStart : 1;
    StorageType SpawnContinuously : 1;
    StorageType AttachAsChild : 1;
  };
};

EZ_DECLARE_FLAGS_OPERATORS(ezSpawnComponentFlags);

typedef ezComponentManager<class ezSpawnComponent> ezSpawnComponentManager;

class EZ_GAMEUTILS_DLL ezSpawnComponent : public ezComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezSpawnComponent, ezComponent, ezSpawnComponentManager);

public:
  ezSpawnComponent();

  virtual void SerializeComponent(ezWorldWriter& stream) const override;
  virtual void DeserializeComponent(ezWorldReader& stream) override;

  /// \brief Spawns a new object, unless the minimum spawn delay has not been reached between calls to this function.
  ///
  /// Manual spawns and continuous (scheduled) spawns are independent from each other regarding minimum spawn delays.
  /// If this function is called in too short intervals, it is ignored and false is returned.
  /// Returns true, if an object was spawned.
  bool TriggerManualSpawn();

  /// \brief Unless a spawn is already scheduled, this will schedule one within the configured time frame.
  ///
  /// If continuous spawning is enabled, this will kick of the first spawn and then continue infinitely.
  /// To stop continuously spawning, remove the continuous spawn flag.
  void ScheduleSpawn();

  // ************************************* PROPERTIES ***********************************

  void SetPrefabFile(const char* szFile);
  const char* GetPrefabFile() const;

  bool GetSpawnAtStart() const { return m_SpawnFlags.IsAnySet(ezSpawnComponentFlags::SpawnAtStart); }
  void SetSpawnAtStart(bool b) { m_SpawnFlags.AddOrRemove(ezSpawnComponentFlags::SpawnAtStart, b); }

  bool GetSpawnContinuously() const { return m_SpawnFlags.IsAnySet(ezSpawnComponentFlags::SpawnContinuously); }
  void SetSpawnContinuously(bool b) { m_SpawnFlags.AddOrRemove(ezSpawnComponentFlags::SpawnContinuously, b); }

  void SetPrefab(const ezPrefabResourceHandle& hPrefab);
  EZ_FORCE_INLINE const ezPrefabResourceHandle& GetPrefab() const { return m_hPrefab; }

  /// The minimum delay between spawning objects. This is also enforced for manually spawning things.
  ezTime m_MinDelay;

  /// For scheduled spawns (continuous / at start) this is an additional random range on top of the minimum spawn delay.
  ezTime m_DelayRange;

private:

  void OnTriggered(ezComponentTriggerMessage& msg);

protected:
  ezBitflags<ezSpawnComponentFlags> m_SpawnFlags;

  // ************************************* FUNCTIONS *****************************

  void InputComponentMessageHandler(ezInputComponentMessage& msg);

  virtual Initialization Initialize() override;

  bool SpawnOnce();


private:
  ezTime m_LastManualSpawn;
  ezPrefabResourceHandle m_hPrefab;
};
