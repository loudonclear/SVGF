#ifndef MESH_H
#define MESH_H

#include <BVH/Object.h>
#include <BVH/BVH.h>

#include "triangle.h"

#include <util/tiny_obj_loader.h>

#include <vector>

#include <Eigen/StdVector>

#include "util/CS123SceneData.h"

EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Matrix2f)
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Matrix3f)
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Eigen::Matrix3i)

class Mesh : public Object
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    virtual ~Mesh();
    void init(const std::vector<Eigen::Vector3f> &vertices,
         const std::vector<Eigen::Vector3f> &normals,
         const std::vector<Eigen::Vector2f> &uvs,
         const std::vector<Eigen::Vector3f> &colors,
         const std::vector<Eigen::Vector3i> &faces,
         const std::vector<int> &materialIds,
         const std::vector<tinyobj::material_t> &materials,
         const CS123SceneMaterial &wholeObjectMaterial);

    virtual bool getIntersection(const Ray &ray, IntersectionInfo *intersection) const;

    virtual Eigen::Vector3f sample() const;

    virtual float getSurfaceArea() const;

    virtual Eigen::Vector3f getNormal(const IntersectionInfo &I) const;

    virtual BBox getBBox() const;

    virtual Eigen::Vector3f getCentroid() const;

    const Eigen::Vector3i getTriangleIndices(int faceIndex) const;
    const tinyobj::material_t& getMaterial(int faceIndex) const;

    const Eigen::Vector3f getVertex(int vertexIndex) const;
    const Eigen::Vector3f getNormal(int vertexIndex) const;
    const Eigen::Vector3f getColor(int vertexIndex) const;
    const Eigen::Vector2f getUV(int vertexIndex) const;

    virtual void setTransform(Eigen::Affine3f transform) override;
    const CS123SceneMaterial getMaterialForWholeObject() const;

private:
    // Properties fromt the scene file
    CS123SceneMaterial _wholeObjectMaterial;

    // Properties from the .obj file
    std::vector<Eigen::Vector3f> _vertices;
    std::vector<Eigen::Vector3f> _normals;
    std::vector<Eigen::Vector3f> _colors;
    std::vector<Eigen::Vector2f> _uvs;
    std::vector<Eigen::Vector3i> _faces;
    std::vector<int> _materialIds;
    std::vector<tinyobj::material_t> _materials;

    BVH *_meshBvh;
    float m_sa;

    Eigen::Vector3f _centroid;

    BBox _bbox;
    BBox transformed_bbox;

    std::vector<Object *> *_objects;
    Triangle *_triangles;

    void calculateMeshStats();
    void createMeshBVH();
};

#endif // MESH_H
