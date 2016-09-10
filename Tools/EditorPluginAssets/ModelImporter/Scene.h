#pragma once

#include <Foundation/Containers/IdTable.h>
#include <Foundation/Types/UniquePtr.h>
#include <EditorPluginAssets/ModelImporter/Handles.h>

namespace ezModelImporter
{
  class HierarchyObject;
  class Mesh;
  class Node;
  struct Material;

  /// Data representation of an imported scene.
  ///
  /// A scene contains all data that has been imported from a single file.
  /// All data is as raw as possible, however we apply basic preprocessing during import already to fit into our data structure.
  /// \see ezModelImporter::Importer
  class Scene
  {
  public:
    Scene();
    ~Scene();

    // Data access.
  public:

    /// Returns a list of all root objects.
    ezArrayPtr<const HierarchyObject* const> GetRootObjects() const { return ezMakeArrayPtr(m_RootObjects); }
    ezArrayPtr<HierarchyObject*> GetRootObjects() { return ezMakeArrayPtr(m_RootObjects); }

    //const HierarchyObject* GetObject(ObjectReference handle) const;
    HierarchyObject* GetObject(ObjectHandle handle);
    const Material* GetMaterial(MaterialHandle handle) const;

    const ezIdTable<ObjectId, ezUniquePtr<Node>>& GetNodes() const           { return m_Nodes; }
    const ezIdTable<ObjectId, ezUniquePtr<Mesh>>& GetMeshes() const          { return m_Meshes; }
    const ezIdTable<MaterialId, ezUniquePtr<Material>>& GetMaterials() const { return m_Materials; }

    // Manipulation methods for importer implementations.
  public:

    ObjectHandle AddNode(ezUniquePtr<Node> node);
    ObjectHandle AddMesh(ezUniquePtr<Mesh> mesh);
    MaterialHandle AddMaterial(ezUniquePtr<Material> material);

    /// Adds all objects without a parent to the list of root objects.
    /// Called by Importer after ImporterImplementation is done.
    void RefreshRootList();

    // Postprocessing
  public:
    /// Merges all meshes into a single one.
    /// 
    /// Transformations from nodes will be applied. The resulting mesh will be stored in the list of root objects.
    /// \return
    ///   Pointer to the newly created meshnode.
    Mesh* MergeAllMeshes();

    /// Generates vertex normals for all meshes that do not have them yet.
    void GenerateVertexNormals();

    /// Generates tangents for all meshes that do not have tangents.
    /// Does not work if a mesh has no normals an UV coordinates.
    void GenerateVertexTangents();


  private:
    ezDynamicArray<HierarchyObject*> m_RootObjects;
    
    ezIdTable<ObjectId, ezUniquePtr<Node>> m_Nodes;
    ezIdTable<ObjectId, ezUniquePtr<Mesh>> m_Meshes;
    ezIdTable<MaterialId, ezUniquePtr<Material>> m_Materials;
  };
}