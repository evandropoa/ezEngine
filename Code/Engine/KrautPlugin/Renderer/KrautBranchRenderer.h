#pragma once

#include <KrautPlugin/Renderer/KrautRenderData.h>
#include <RendererCore/Pipeline/Declarations.h>

struct ezPerInstanceData;

/// \brief Implements rendering of static meshes
class EZ_KRAUTPLUGIN_DLL ezKrautBranchRenderer : public ezRenderer
{
  EZ_ADD_DYNAMIC_REFLECTION(ezKrautBranchRenderer, ezRenderer);
  EZ_DISALLOW_COPY_AND_ASSIGN(ezKrautBranchRenderer);

public:
  ezKrautBranchRenderer();
  ~ezKrautBranchRenderer();

  // ezRenderer implementation
  virtual void GetSupportedRenderDataTypes(ezHybridArray<const ezRTTI*, 8>& types) override;
  virtual void RenderBatch(const ezRenderViewContext& renderContext, ezRenderPipelinePass* pPass, const ezRenderDataBatch& batch) override;

protected:
  virtual void FillPerInstanceData(const ezVec3& vLodCamPos, ezArrayPtr<ezPerInstanceData> instanceData,
                                   const ezRenderDataBatch& batch, ezUInt32 uiStartIndex, ezUInt32& out_uiFilteredCount);
};
