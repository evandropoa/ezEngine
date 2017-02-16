#include <PCH.h>
#include <ParticlePlugin/Type/Billboard/ParticleTypeBillboard.h>
#include <RendererCore/Shader/ShaderResource.h>
#include <RendererFoundation/Descriptors/Descriptors.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererCore/Pipeline/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererCore/Meshes/MeshBufferResource.h>
#include <RendererCore/Textures/Texture2DResource.h>
#include <Core/World/GameObject.h>
#include <RendererCore/Pipeline/ExtractedRenderData.h>
#include <Core/World/World.h>

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleTypeBillboardFactory, 1, ezRTTIDefaultAllocator<ezParticleTypeBillboardFactory>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Texture", m_sTexture)->AddAttributes(new ezAssetBrowserAttribute("Texture 2D")),
  }
  EZ_END_PROPERTIES
}
EZ_END_DYNAMIC_REFLECTED_TYPE

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleTypeBillboard, 1, ezRTTIDefaultAllocator<ezParticleTypeBillboard>)
EZ_END_DYNAMIC_REFLECTED_TYPE

const ezRTTI* ezParticleTypeBillboardFactory::GetTypeType() const
{
  return ezGetStaticRTTI<ezParticleTypeBillboard>();
}


void ezParticleTypeBillboardFactory::CopyTypeProperties(ezParticleType* pObject) const
{
  ezParticleTypeBillboard* pType = static_cast<ezParticleTypeBillboard*>(pObject);

  pType->m_hTexture.Invalidate();

  if (!m_sTexture.IsEmpty())
    pType->m_hTexture = ezResourceManager::LoadResource<ezTexture2DResource>(m_sTexture);
}

enum class TypeBillboardVersion
{
  Version_0 = 0,
  Version_1,
  Version_2, // added texture

  // insert new version numbers above
  Version_Count,
  Version_Current = Version_Count - 1
};

void ezParticleTypeBillboardFactory::Save(ezStreamWriter& stream) const
{
  const ezUInt8 uiVersion = (int)TypeBillboardVersion::Version_Current;
  stream << uiVersion;

  stream << m_sTexture;
}

void ezParticleTypeBillboardFactory::Load(ezStreamReader& stream)
{
  ezUInt8 uiVersion = 0;
  stream >> uiVersion;

  EZ_ASSERT_DEV(uiVersion <= (int)TypeBillboardVersion::Version_Current, "Invalid version {0}", uiVersion);

  if (uiVersion >= 2)
  {
    stream >> m_sTexture;
  }
}


ezParticleTypeBillboard::ezParticleTypeBillboard()
{
}

void ezParticleTypeBillboard::CreateRequiredStreams()
{
  CreateStream("Position", ezProcessingStream::DataType::Float3, &m_pStreamPosition, false);
  CreateStream("Size", ezProcessingStream::DataType::Float, &m_pStreamSize, false);
  CreateStream("Color", ezProcessingStream::DataType::Float4, &m_pStreamColor, false);
  CreateStream("RotationSpeed", ezProcessingStream::DataType::Float, &m_pStreamRotationSpeed, false);
}

void ezParticleTypeBillboard::ExtractTypeRenderData(const ezView& view, ezExtractedRenderData* pExtractedRenderData, const ezTransform& instanceTransform, ezUInt64 uiExtractedFrame) const
{
  if (!m_hTexture.IsValid())
    return;

  const ezUInt32 numParticles = (ezUInt32)GetOwnerSystem()->GetNumActiveParticles();

  if (numParticles == 0)
    return;

  const ezTime tCur = GetOwnerSystem()->GetWorld()->GetClock().GetAccumulatedTime();

  // don't copy the data multiple times in the same frame, if the effect is instanced
  if (m_uiLastExtractedFrame != uiExtractedFrame)
  {
    m_uiLastExtractedFrame = uiExtractedFrame;

    const ezVec3* pPosition = m_pStreamPosition->GetData<ezVec3>();
    const float* pSize = m_pStreamSize->GetData<float>();
    const ezColor* pColor = m_pStreamColor->GetData<ezColor>();
    const float* pRotationSpeed = m_pStreamRotationSpeed->GetData<float>();

    // this will automatically be deallocated at the end of the frame
    m_ParticleData = EZ_NEW_ARRAY(ezFrameAllocator::GetCurrentAllocator(), ezBillboardParticleData, numParticles);

    ezTransform t;

    for (ezUInt32 p = 0; p < numParticles; ++p)
    {
      t.m_Rotation.SetRotationMatrixY(ezAngle::Radian((float)(tCur.GetSeconds() * pRotationSpeed[p])));
      t.m_vPosition = pPosition[p];
      m_ParticleData[p].Transform = t;
      m_ParticleData[p].Size = pSize[p];
      m_ParticleData[p].Color = pColor[p];
    }
  }

  /// \todo Is this batch ID correct?
  const ezUInt32 uiBatchId = m_hTexture.GetResourceIDHash();
  auto pRenderData = ezCreateRenderDataForThisFrame<ezParticleBillboardRenderData>(nullptr, uiBatchId);

  pRenderData->m_GlobalTransform = instanceTransform;
  pRenderData->m_hTexture = m_hTexture;
  pRenderData->m_ParticleData = m_ParticleData;

  /// \todo Generate a proper sorting key?
  const ezUInt32 uiSortingKey = 0;
  pExtractedRenderData->AddRenderData(pRenderData, ezDefaultRenderDataCategories::SimpleTransparent, uiSortingKey);
}




EZ_STATICLINK_FILE(ParticlePlugin, ParticlePlugin_Type_Billboard_ParticleTypeBillboard);

