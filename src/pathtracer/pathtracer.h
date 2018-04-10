#ifndef PATHTRACER_H
#define PATHTRACER_H

#include "scene/scene.h"

#include <QImage>

#include <memory>

// Class for representing a set of render buffers together, and accessing them.
class RenderBuffers {
public:
  RenderBuffers(std::size_t width, std::size_t height)
      : m_width(width), m_height(height),
        m_albedo(new glm::vec3[m_width * m_height]),
        m_direct(new glm::vec3[m_width * m_height]),
        m_indirect(new glm::vec3[m_width * m_height]),
        m_full(new glm::vec3[m_width * m_height]) {}

  std::size_t m_width, m_height;
  std::unique_ptr<glm::vec3[]> m_albedo;
  std::unique_ptr<glm::vec3[]> m_direct;
  std::unique_ptr<glm::vec3[]> m_indirect;
  std::unique_ptr<glm::vec3[]> m_full;

  // A single element w/ information on all the channels
  class Element {
  public:
    glm::vec3 m_albedo, m_direct, m_indirect, m_full;

    Element &operator+=(const Element &e) {
      m_albedo += e.m_albedo;
      m_direct += e.m_direct;
      m_indirect += e.m_indirect;
      m_full += e.m_full;
      return *this;
    }
    Element &operator/=(float f) {
      m_albedo /= f;
      m_direct /= f;
      m_indirect /= f;
      m_full /= f;
      return *this;
    }
  };

  class Slice {
  public:
    Slice(std::size_t offset, RenderBuffers &buffs)
        : m_offset(offset), m_buffs(buffs) {}
    Slice &operator=(const Element &elem) {
      albedo() = elem.m_albedo;
      direct() = elem.m_direct;
      indirect() = elem.m_indirect;
      full() = elem.m_full;
      return *this;
    }
    Slice &operator=(const Slice &sl) {
      albedo() = sl.albedo();
      direct() = sl.direct();
      indirect() = sl.indirect();
      full() = sl.full();
      return *this;
    }

    glm::vec3 &albedo() { return m_buffs.m_albedo[m_offset]; }
    const glm::vec3 &albedo() const { return m_buffs.m_albedo[m_offset]; }
    glm::vec3 &direct() { return m_buffs.m_direct[m_offset]; }
    const glm::vec3 &direct() const { return m_buffs.m_direct[m_offset]; }
    glm::vec3 &indirect() { return m_buffs.m_indirect[m_offset]; }
    const glm::vec3 &indirect() const { return m_buffs.m_indirect[m_offset]; }
    glm::vec3 &full() { return m_buffs.m_full[m_offset]; }
    const glm::vec3 &full() const { return m_buffs.m_full[m_offset]; }

    // For creating const slices
    static const Slice make_const(std::size_t offset, const RenderBuffers& buffs) {
      return Slice(offset, const_cast<RenderBuffers&>(buffs));
    }

  private:
    std::size_t m_offset;
    RenderBuffers &m_buffs;
  };

  Slice operator[](std::size_t i) { return Slice(i, *this); }
  const Slice operator[](std::size_t i) const { return Slice::make_const(i, *this); }
};

class PathTracer
{
public:
    PathTracer(int width, int height, int numSamples);

    void traceScene(QRgb *imageData, const Scene &scene);
    void render(const Scene& scene, glm::vec3 *intensityValues, glm::mat4x4 &invViewMat, int x0, int y0, int x1, int y1);
    static float random();

private:
    int m_width, m_height, m_numSamples;

    void toneMap(QRgb *imageData, glm::vec3 *intensityValues);

    glm::vec3 tracePixel(int x, int y, const Scene &scene, const glm::mat4x4 &invViewMatrix);
    glm::vec3 traceRay(const Ray& r, const Scene &scene, int depth);
};

#endif // PATHTRACER_H
