#include <PCH.h>
#include <ParticlePlugin/Type/Fragment/FragmentRenderer.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Context/Context.h>
#include <RendererCore/Pipeline/RenderDataBatch.h>
#include <RendererCore/Shader/ShaderResource.h>
#include <Foundation/Types/ScopeExit.h>

#include <RendererCore/../../../Data/Base/Shaders/Particles/ParticleSystemConstants.h>

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleFragmentRenderData, 1, ezRTTINoAllocator)
EZ_END_DYNAMIC_REFLECTED_TYPE

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleFragmentRenderer, 1, ezRTTIDefaultAllocator<ezParticleFragmentRenderer>);
EZ_END_DYNAMIC_REFLECTED_TYPE

ezParticleFragmentRenderer::~ezParticleFragmentRenderer()
{
  if (!m_hDataBuffer.IsInvalidated())
  {
    ezGALDevice::GetDefaultDevice()->DestroyBuffer(m_hDataBuffer);
    m_hDataBuffer.Invalidate();
  }
}

void ezParticleFragmentRenderer::GetSupportedRenderDataTypes(ezHybridArray<const ezRTTI*, 8>& types)
{
  types.PushBack(ezGetStaticRTTI<ezParticleFragmentRenderData>());
}

void ezParticleFragmentRenderer::CreateDataBuffer()
{
  if (m_hDataBuffer.IsInvalidated())
  {
    ezGALBufferCreationDescription desc;
    desc.m_uiStructSize = sizeof(ezFragmentParticleData);
    desc.m_uiTotalSize = s_uiParticlesPerBatch * desc.m_uiStructSize;
    desc.m_BufferType = ezGALBufferType::Generic;
    desc.m_bUseAsStructuredBuffer = true;
    desc.m_bAllowShaderResourceView = true;
    desc.m_ResourceAccess.m_bImmutable = false;

    m_hDataBuffer = ezGALDevice::GetDefaultDevice()->CreateBuffer(desc);
  }
}

void ezParticleFragmentRenderer::RenderBatch(const ezRenderViewContext& renderViewContext, ezRenderPipelinePass* pPass, const ezRenderDataBatch& batch)
{
  ezGALDevice* pDevice = ezGALDevice::GetDefaultDevice();
  ezGALContext* pGALContext = renderViewContext.m_pRenderContext->GetGALContext();

  /// \todo This pattern looks like it is inefficient. Should it use the GPU pool instead somehow?
  // prepare the constant buffer
  ezConstantBufferStorage<ezParticleSystemConstants>* pConstantBuffer;
  ezConstantBufferStorageHandle hConstantBuffer = ezRenderContext::CreateConstantBufferStorage(pConstantBuffer);
  EZ_SCOPE_EXIT(ezRenderContext::DeleteConstantBufferStorage(hConstantBuffer));
  renderViewContext.m_pRenderContext->BindConstantBuffer("ezParticleSystemConstants", hConstantBuffer);

  // Bind the Fragment particle shader
  {
    if (!m_hShader.IsValid())
    {
      m_hShader = ezResourceManager::LoadResource<ezShaderResource>("Shaders/Particles/Fragment.ezShader");
    }

    renderViewContext.m_pRenderContext->BindShader(m_hShader);
  }

  // make sure our structured buffer is allocated and bound
  {
    CreateDataBuffer();
    renderViewContext.m_pRenderContext->BindMeshBuffer(ezGALBufferHandle(), ezGALBufferHandle(), nullptr, ezGALPrimitiveTopology::Triangles, s_uiParticlesPerBatch * 2);
    renderViewContext.m_pRenderContext->BindBuffer(ezGALShaderStage::VertexShader, "particleData", pDevice->GetDefaultResourceView(m_hDataBuffer));
  }

  // now render all particle effects of type Fragment
  for (auto it = batch.GetIterator<ezParticleFragmentRenderData>(0, batch.GetCount()); it.IsValid(); ++it)
  {
    const ezParticleFragmentRenderData* pRenderData = it;
    ezUInt32 uiNumParticles = pRenderData->m_ParticleData.GetCount();

    const ezFragmentParticleData* pParticleData = pRenderData->m_ParticleData.GetPtr();

    renderViewContext.m_pRenderContext->BindTexture2D(ezGALShaderStage::PixelShader, "ParticleTexture", pRenderData->m_hTexture);

    // fill the constant buffer
    {
      ezParticleSystemConstants& cb = pConstantBuffer->GetDataForWriting();
      cb.ObjectToWorldMatrix = pRenderData->m_GlobalTransform.GetAsMat4();
    }

    while (uiNumParticles > 0)
    {
      // upload this batch of particle data
      const ezUInt32 uiNumParticlesInBatch = ezMath::Min<ezUInt32>(uiNumParticles, s_uiParticlesPerBatch);
      pGALContext->UpdateBuffer(m_hDataBuffer, 0, ezMakeArrayPtr(pParticleData, uiNumParticlesInBatch).ToByteArray());

      // do one drawcall
      renderViewContext.m_pRenderContext->DrawMeshBuffer(uiNumParticlesInBatch * 2);

      uiNumParticles -= uiNumParticlesInBatch;
      pParticleData += uiNumParticlesInBatch;
    }
  }
}



EZ_STATICLINK_FILE(ParticlePlugin, ParticlePlugin_Type_Fragment_FragmentRenderer);

