#include "mesh.h"


#include <iostream>
#include "pathtracer/pathtracer.h"

using namespace std;


void Mesh::init(int id, const std::vector<glm::vec3> &vertices,
           const std::vector<glm::vec3> &normals,
           const std::vector<glm::vec2> &uvs,
           const std::vector<glm::vec3> &colors,
           const std::vector<glm::ivec3> &faces,
           const std::vector<int> &materialIds,
           const std::vector<tinyobj::material_t> &materials,
           const CS123SceneMaterial &wholeObjectMaterial)
{
    m_id = id;
    _vertices = vertices;
    _normals = normals;
    _colors = colors;
    _uvs = uvs;
    _faces = faces;
    _materialIds = materialIds;
    _materials = materials;
    _wholeObjectMaterial = wholeObjectMaterial;
    calculateMeshStats();
    createMeshBVH();

    std::vector<std::vector<glm::ivec3>> subObjects;
    subObjects.resize(materials.size());
    m_VAOs.clear();
    m_VAOs.resize(materials.size());
    m_numVertices.resize(materials.size());

    for (unsigned int i = 0; i < faces.size(); i++) {
        const int matID = materialIds.at(i);
        std::vector<glm::ivec3> &subObject = subObjects.at(matID);
        subObject.push_back(faces.at(i));
    }

    for (unsigned int i = 0; i < subObjects.size(); i++) {
        std::vector<glm::ivec3> &subObject = subObjects.at(i);
        GLuint &Vao = m_VAOs.at(i);
        m_numVertices.at(i) = subObject.size() * 3;

        GLuint Vbo;
        GLuint Ibo;
        glGenBuffers(1, &Vbo);
        glGenBuffers(1, &Ibo);
        glGenVertexArrays(1, &Vao);

        glBindBuffer(GL_ARRAY_BUFFER, Vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size() * 3 * 2, nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * vertices.size() * 3, static_cast<const void *>(vertices.data()));
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size() * 3, sizeof(float) * vertices.size() * 3, static_cast<const void *>(normals.data()));
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 3 * subObject.size(), static_cast<const void *>(subObject.data()), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glBindVertexArray(Vao);
        glBindBuffer(GL_ARRAY_BUFFER, Vbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, static_cast<GLvoid *>(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid *>(sizeof(float) * vertices.size() * 3));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Ibo);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

Mesh::~Mesh()
{
    delete _meshBvh;
    delete _objects;
    delete[] _triangles;
}

void Mesh::render(std::shared_ptr<CS123::GL::Shader> &shader, const bool pipeline) const {

    if (pipeline) {
        shader->setUniform("meshID", static_cast<float>(m_id));
    }

    for (int i = 0; i < m_VAOs.size(); i++) {
        const tinyobj::material_t &mat = _materials.at(i);

        if (!pipeline) {
            shader->setUniform("ambient_color", glm::vec3(mat.ambient[0], mat.ambient[1], mat.ambient[2]));
            shader->setUniform("diffuse_color", glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]));
            shader->setUniform("specular_color", glm::vec3(mat.specular[0], mat.specular[1], mat.specular[2]));
            shader->setUniform("shininess", mat.shininess);
        } else {
            shader->setUniform("matID", static_cast<float>(m_id));
        }

        glBindVertexArray(m_VAOs.at(i));
        glDrawElements(GL_TRIANGLES, m_numVertices.at(i), GL_UNSIGNED_INT, reinterpret_cast<GLvoid *>(0));
        glBindVertexArray(0);
    }
}

bool Mesh::getIntersection(const Ray &ray, IntersectionInfo *intersection) const
{
    Ray r(ray.transform(inverseTransform));
    IntersectionInfo i;
    bool col = _meshBvh->getIntersection(r, &i, false);
    if(col) {
        intersection->t = i.t;
        intersection->object = this;
        intersection->data = i.object;

        return true;
    }
    return false;
}

glm::vec3 Mesh::sample() const {
    const float r1 = PathTracer::random();
    float indsa = r1 * m_sa;
    unsigned int ind;

    for (ind = 0; ind < _faces.size() - 1; ++ind) {
        indsa -= _triangles[ind].getSurfaceArea();
        if (indsa < 0) break;
    }

    return _triangles[ind].sample();
}

float Mesh::getSurfaceArea() const {
    return m_sa;
}

glm::vec3 Mesh::getNormal(const IntersectionInfo &I) const
{
    return static_cast<const Object *>(I.data)->getNormal(I);
}

BBox Mesh::getBBox() const
{
    return transformed_bbox;
}

glm::vec3 Mesh::getCentroid() const
{
    return (transform * glm::vec4(_centroid, 1)).xyz();
}

const glm::ivec3 Mesh::getTriangleIndices(int faceIndex) const
{
    return _faces[faceIndex];
}

const tinyobj::material_t &Mesh::getMaterial(int faceIndex) const
{
    return _materials[_materialIds[faceIndex]];
}

const glm::vec3 Mesh::getVertex(int vertexIndex) const
{
    return _vertices[vertexIndex];
}

const glm::vec3 Mesh::getNormal(int vertexIndex) const
{
    return _normals[vertexIndex];
}

const glm::vec3 Mesh::getColor(int vertexIndex) const
{
    return _colors[vertexIndex];
}

const glm::vec2 Mesh::getUV(int vertexIndex) const
{
    return _uvs[vertexIndex];
}

void Mesh::setTransform(glm::mat4x4 transform)
{
    Object::setTransform(transform);
    transformed_bbox = BBox();
    glm::vec3 min = _bbox.min;
    transformed_bbox.setP((transform * glm::vec4(min, 1)).xyz());
    transformed_bbox.expandToInclude((transform * glm::vec4(_bbox.max, 1)).xyz());
}

const CS123SceneMaterial Mesh::getMaterialForWholeObject() const
{
    return _wholeObjectMaterial;
}

void Mesh::calculateMeshStats()
{
    _bbox.setP(_vertices[0]);
    for(auto v : _vertices) {
        _centroid += v;
        _bbox.expandToInclude(v);
    }
    transformed_bbox = _bbox;
    _centroid /= _vertices.size();
}

void Mesh::createMeshBVH()
{
    m_sa = 0;
    _triangles = new Triangle[_faces.size()];
    _objects = new std::vector<Object *>;
    _objects->resize(_faces.size());
    for(unsigned int i = 0; i < _faces.size(); ++i) {
        glm::ivec3 face = _faces[i];
        glm::vec3 v1 = _vertices[face.x];
        glm::vec3 v2 = _vertices[face.y];
        glm::vec3 v3 = _vertices[face.z];
        glm::vec3 n1 = _normals[face.x];
        glm::vec3 n2 = _normals[face.y];
        glm::vec3 n3 = _normals[face.z];
        _triangles[i] = Triangle(v1, v2, v3, n1, n2, n3, i);
        (*_objects)[i] = &_triangles[i];

        m_sa += _triangles[i].getSurfaceArea();
    }

    _meshBvh = new BVH(_objects);
}
