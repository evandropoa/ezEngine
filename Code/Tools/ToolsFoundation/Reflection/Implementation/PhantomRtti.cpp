#include <PCH.h>
#include <ToolsFoundation/Reflection/PhantomRtti.h>
#include <ToolsFoundation/Reflection/PhantomProperty.h>
#include <Foundation/Reflection/ReflectionUtils.h>

ezPhantomRTTI::ezPhantomRTTI(const char* szName, const ezRTTI* pParentType, ezUInt32 uiTypeSize, ezUInt32 uiTypeVersion, ezUInt32 uiVariantType, ezBitflags<ezTypeFlags> flags, const char* szPluginName)
  : ezRTTI(nullptr, pParentType, uiTypeSize, uiTypeVersion, uiVariantType, flags | ezTypeFlags::Phantom, nullptr, ezArrayPtr<ezAbstractProperty*>(), ezArrayPtr<ezPropertyAttribute*>(), ezArrayPtr<ezAbstractMessageHandler*>(), nullptr )
{
  m_sTypeNameStorage = szName;
  m_sPluginNameStorage = szPluginName;

  m_szTypeName = m_sTypeNameStorage.GetData();
  m_szPluginName = m_sPluginNameStorage.GetData();

  RegisterType(this);
}

ezPhantomRTTI::~ezPhantomRTTI()
{
  UnregisterType(this);
  m_szTypeName = nullptr;

  for (auto pProp : m_PropertiesStorage)
  {
    EZ_DEFAULT_DELETE(pProp);
  }
  for (auto pAttrib : m_AttributesStorage)
  {
    EZ_DEFAULT_DELETE(pAttrib);
  }
}

void ezPhantomRTTI::SetProperties(const ezDynamicArray<ezReflectedPropertyDescriptor>& properties)
{
  for (auto pProp : m_PropertiesStorage)
  {
    EZ_DEFAULT_DELETE(pProp);
  }
  m_PropertiesStorage.Clear();

  const ezUInt32 iCount = properties.GetCount();
  m_PropertiesStorage.Reserve(iCount);

  for (ezUInt32 i = 0; i < iCount; i++)
  {
    switch (properties[i].m_Category)
    {
    case ezPropertyCategory::Constant:
      {
        m_PropertiesStorage.PushBack(EZ_DEFAULT_NEW(ezPhantomConstantProperty, &properties[i]));
      }
      break;
    case ezPropertyCategory::Member:
      {
        m_PropertiesStorage.PushBack(EZ_DEFAULT_NEW(ezPhantomMemberProperty, &properties[i]));
      }
      break;
    case ezPropertyCategory::Function:
      {
        m_PropertiesStorage.PushBack(EZ_DEFAULT_NEW(ezPhantomFunctionProperty, &properties[i]));
      }
      break;
    case ezPropertyCategory::Array:
      {
        m_PropertiesStorage.PushBack(EZ_DEFAULT_NEW(ezPhantomArrayProperty, &properties[i]));
      }
      break;
    case ezPropertyCategory::Set:
      {
        m_PropertiesStorage.PushBack(EZ_DEFAULT_NEW(ezPhantomSetProperty, &properties[i]));
      }
      break;
    case ezPropertyCategory::Map:
      {
        EZ_ASSERT_NOT_IMPLEMENTED;
        //m_PropertiesStorage.PushBack(EZ_DEFAULT_NEW(ezPhantomMapProperty, &properties[i]));
      }
      break;
    }

  }

  m_Properties = m_PropertiesStorage;
}

void ezPhantomRTTI::SetAttributes(ezHybridArray<ezPropertyAttribute*, 2>& attributes)
{
  for (auto pAttrib : m_AttributesStorage)
  {
    EZ_DEFAULT_DELETE(pAttrib);
  }
  m_AttributesStorage.Clear();
  m_AttributesStorage = attributes;
  m_Attributes = m_AttributesStorage;
  attributes.Clear();
}

void ezPhantomRTTI::UpdateType(const ezReflectedTypeDescriptor& desc)
{
  ezRTTI::UpdateType(ezRTTI::FindTypeByName(desc.m_sParentTypeName), desc.m_uiTypeSize, desc.m_uiTypeVersion, ezVariantType::Invalid, desc.m_Flags);

  m_sPluginNameStorage = desc.m_sPluginName;
  m_szPluginName = m_sPluginNameStorage.GetData();

  SetProperties(desc.m_Properties);
  SetAttributes(desc.m_Attributes);
}

bool ezPhantomRTTI::IsEqualToDescriptor(const ezReflectedTypeDescriptor& desc)
{
  if ((desc.m_Flags.GetValue() & ~ezTypeFlags::Phantom) != (GetTypeFlags().GetValue() & ~ezTypeFlags::Phantom))
    return false;

  if (desc.m_sParentTypeName.IsEmpty() && GetParentType() != nullptr)
    return false;

  if (GetParentType() != nullptr && desc.m_sParentTypeName != GetParentType()->GetTypeName())
    return false;

  if (desc.m_sPluginName != GetPluginName())
    return false;

  if (desc.m_sTypeName != GetTypeName())
    return false;

  if (desc.m_Properties.GetCount() != GetProperties().GetCount())
    return false;

  for (ezUInt32 i = 0; i < GetProperties().GetCount(); i++)
  {
    if (desc.m_Properties[i].m_Category != GetProperties()[i]->GetCategory())
      return false;

    if (desc.m_Properties[i].m_sName != GetProperties()[i]->GetPropertyName())
      return false;

    if ((desc.m_Properties[i].m_Flags.GetValue() & ~ezPropertyFlags::Phantom) != (GetProperties()[i]->GetFlags().GetValue() & ~ezPropertyFlags::Phantom))
      return false;

    switch (desc.m_Properties[i].m_Category)
    {
    case ezPropertyCategory::Constant:
      {
        auto pProp = (ezPhantomConstantProperty*)GetProperties()[i];

        if (pProp->GetSpecificType() != ezRTTI::FindTypeByName(desc.m_Properties[i].m_sType))
          return false;

        if (pProp->GetConstant() != desc.m_Properties[i].m_ConstantValue)
          return false;
      }
      break;
    case ezPropertyCategory::Member:
      {
        if (GetProperties()[i]->GetSpecificType() != ezRTTI::FindTypeByName(desc.m_Properties[i].m_sType))
          return false;
      }
      break;
    case ezPropertyCategory::Function:
      {
        EZ_ASSERT_NOT_IMPLEMENTED;
      }
      break;
    case ezPropertyCategory::Array:
      {
        if (GetProperties()[i]->GetSpecificType() != ezRTTI::FindTypeByName(desc.m_Properties[i].m_sType))
          return false;
      }
      break;
    case ezPropertyCategory::Set:
      {
        if (GetProperties()[i]->GetSpecificType() != ezRTTI::FindTypeByName(desc.m_Properties[i].m_sType))
          return false;
      }
      break;
    case ezPropertyCategory::Map:
      {
        EZ_ASSERT_NOT_IMPLEMENTED;
      }
      break;

    }

    if (desc.m_Properties[i].m_Attributes.GetCount() != GetProperties()[i]->GetAttributes().GetCount())
      return false;

    for (ezUInt32 i2 = 0; i2 < desc.m_Properties[i].m_Attributes.GetCount(); i2++)
    {
      if (!ezReflectionUtils::IsEqual(desc.m_Properties[i].m_Attributes[i2], GetProperties()[i]->GetAttributes()[i2]))
        return false;
    }
  }

  if (desc.m_Attributes.GetCount() != GetAttributes().GetCount())
    return false;

  // TODO: compare attribute values?
  for (ezUInt32 i = 0; i < GetAttributes().GetCount(); i++)
  {
    if (desc.m_Attributes[i]->GetDynamicRTTI() != GetAttributes()[i]->GetDynamicRTTI())
      return false;
  }
  return true;
}
