#ifndef UTIL_H
#define UTIL_H

#include "glm/glm.hpp"
#include "util/CS123Common.h"

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

  std::size_t size() const { return m_width * m_height; }

  // creates a buffer for albedo * (direct + indirect)
  // Ideally, this should be the same as full()
  std::unique_ptr<glm::vec3[]> recombined() const;

  // A single element w/ information on all the channels
  class Slice;
  class Element {
  public:
    glm::vec3 m_albedo, m_direct, m_indirect, m_full;

    Element& operator=(const Slice& sl);
    Element &operator+=(const Element &e);
    Element &operator/=(float f);
    Element &operator*=(float f);
    bool operator==(const Element &e) const;
    bool operator!=(const Element& e) const;
    // returns true if ANY elements are NaN
    bool isnan() const;

    static Element zero();
  };

  // A reference to a specific index in the buffers, over all channels.
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

    // access specific channels
    glm::vec3 &albedo() { return m_buffs.m_albedo[m_offset]; }
    const glm::vec3 &albedo() const { return m_buffs.m_albedo[m_offset]; }
    glm::vec3 &direct() { return m_buffs.m_direct[m_offset]; }
    const glm::vec3 &direct() const { return m_buffs.m_direct[m_offset]; }
    glm::vec3 &indirect() { return m_buffs.m_indirect[m_offset]; }
    const glm::vec3 &indirect() const { return m_buffs.m_indirect[m_offset]; }
    glm::vec3 &full() { return m_buffs.m_full[m_offset]; }
    const glm::vec3 &full() const { return m_buffs.m_full[m_offset]; }

    // For creating const slices
    static const Slice make_const(std::size_t offset,
                                  const RenderBuffers &buffs) {
      return Slice(offset, const_cast<RenderBuffers &>(buffs));
    }

  private:
    std::size_t m_offset;
    RenderBuffers &m_buffs;
  };

  // Return a slice, like a "reference" that can be assigned to.
  Slice operator[](std::size_t i) { return Slice(i, *this); }
  const Slice operator[](std::size_t i) const {
    return Slice::make_const(i, *this);
  }
};

// Convert a floating point value in [0,1] to an integer [0,255] and clamp it.
inline unsigned char REAL2byte(REAL f) {
  int i = static_cast<int>((f * 255.0 + 0.5));
  return (i < 0) ? 0 : (i > 255) ? 255 : i;
}

// Converts a vector of 3 float values to a clamped QRgb value
inline QRgb vec3ToQRgb(const glm::vec3 &v) {
  return qRgb(REAL2byte(v.x), REAL2byte(v.y), REAL2byte(v.z));
}

void toneMap(QRgb *imageData, glm::vec3 *intensityValues, std::size_t size);

// simple tone map that just clamps to [0,1[(]or [0,255]
void toneMapSimple(QRgb *imageData, glm::vec3 *intensityValues,
                   std::size_t size);

// Saves all buffers, and a combined version, in the given location
void save_render_buffers(const RenderBuffers &buffs,
                         std::string directory = "../res/results/",
                         std::string name = "output");

// Save a single image
bool save_image(const QImage &img, const QString &filename);

#endif // UTIL_H
