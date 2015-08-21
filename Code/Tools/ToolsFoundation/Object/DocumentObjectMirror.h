#pragma once

#include <ToolsFoundation/Basics.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>
#include <Foundation/Serialization/RttiConverter.h>
#include <Foundation/Containers/HybridArray.h>


class ezDocumentObjectMirror
{
public:
  ezDocumentObjectMirror();
  ~ezDocumentObjectMirror();

  void Init(const ezDocumentObjectManager* pManager);
  void DeInit();

  void TreeStructureEventHandler(const ezDocumentObjectStructureEvent& e);
  void TreePropertyEventHandler(const ezDocumentObjectPropertyEvent& e);

private:
  bool IsRootObject(const ezDocumentObjectBase* pParent);
  bool IsHeapAllocated(const ezDocumentObjectBase* pParent, const char* szParentProperty);
  ezUuid FindRootOpObject(const ezDocumentObjectBase* pObject, ezHybridArray<const ezDocumentObjectBase*, 8>& path);

private:
  ezRttiConverterContext m_Context;
  const ezDocumentObjectManager* m_pManager;
};