#include <PCH.h>
#include <Core/World/World.h>

ezComponentManagerBase::ezComponentManagerBase(ezWorld* pWorld)
  : ezWorldModule(pWorld)
  , m_Components(pWorld->GetAllocator())
{
}

ezComponentManagerBase::~ezComponentManagerBase()
{
}


ezComponentHandle ezComponentManagerBase::CreateComponent()
{
  ezComponent* pDummy;
  return CreateComponent(pDummy);
}

void ezComponentManagerBase::DeleteComponent(const ezComponentHandle& component)
{
  ezComponent* pComponent = nullptr;
  if (!m_Components.TryGetValue(component, pComponent))
    return;

  DeinitializeComponent(pComponent);

  m_Components.Remove(pComponent->m_InternalId);

  pComponent->m_InternalId.Invalidate();
  pComponent->m_ComponentFlags.Remove(ezObjectFlags::Active);

  GetWorld()->m_Data.m_DeadComponents.PushBack(pComponent);
}

void ezComponentManagerBase::InitializeComponent(const ezComponentHandle& hComponent)
{
  GetWorld()->AddComponentToInitialize(hComponent);
}

void ezComponentManagerBase::DeinitializeComponent(ezComponent* pComponent)
{
  if (ezGameObject* pOwner = pComponent->GetOwner())
  {
    pOwner->DetachComponent(pComponent);
  }

  if (pComponent->IsInitialized())
  {
    pComponent->Deinitialize();
    pComponent->m_ComponentFlags.Remove(ezObjectFlags::Initialized);
  }
}

void ezComponentManagerBase::PatchIdTable(ezComponent* pComponent)
{
  ezGenericComponentId id = pComponent->m_InternalId;
  if (id.m_InstanceIndex != ezGenericComponentId::INVALID_INSTANCE_INDEX)
    m_Components[id] = pComponent;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

ezComponentManagerFactory::ezComponentManagerFactory()
{
}

ezComponentManagerFactory* ezComponentManagerFactory::GetInstance()
{
  static ezComponentManagerFactory* pInstance = new ezComponentManagerFactory();
  return pInstance;
}

ezUInt16 ezComponentManagerFactory::GetTypeId(const ezRTTI* pRtti)
{
  ezUInt16 uiTypeId = 0xFFFF;
  m_TypeToId.TryGetValue(pRtti, uiTypeId);
  return uiTypeId;
}

ezComponentManagerBase* ezComponentManagerFactory::CreateComponentManager(ezUInt16 typeId, ezWorld* pWorld)
{
  if (typeId < m_CreatorFuncs.GetCount())
  {
    CreatorFunc func = m_CreatorFuncs[typeId];
    return (*func)(pWorld->GetAllocator(), pWorld);
  }

  return nullptr;
}

EZ_STATICLINK_FILE(Core, Core_World_Implementation_ComponentManager);

