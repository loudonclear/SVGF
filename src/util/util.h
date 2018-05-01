#ifndef UTIL_H
#define UTIL_H

#include "glm/glm.hpp"
#include "util/CS123Common.h"

#include <array>
#include <memory>
#include <QImage>

// Class for representing a set of render buffers together, and accessing them.
class RenderBuffers {
public:
  enum layer_idx : std::size_t {
    ALBEDO, DIRECT, INDIRECT, POSITION, OBJ_ID, NORMALS, FULL, NUM_LAYERS
 };

  RenderBuffers(std::size_t width, std::size_t height)
      : m_width(width), m_height(height) {
    for(auto& l : m_layers){
      l = std::make_unique<glm::vec3[]>(m_width * m_height);
    }
  }

  std::size_t m_width, m_height;
  std::array<std::unique_ptr<glm::vec3[]>, NUM_LAYERS> m_layers;

  std::size_t size() const { return m_width * m_height; }
  std::unique_ptr<glm::vec3[]>& l(std::size_t i) { return m_layers[i];}
  const std::unique_ptr<glm::vec3[]>& l(std::size_t i) const { return m_layers[i];}

  // creates a buffer for albedo * (direct + indirect)
  // Ideally, this should be the same as full()
  std::unique_ptr<glm::vec3[]> recombined() const;

  // Saves all buffers, and a combined version, in the given location
  void save(const std::string& directory = "./res/results", const std::string& name = "output") const;

  // since in current setup each layer has to be vec3, here we interlave position and ID into a vec4
  std::unique_ptr<glm::vec4[]> interleave_pos_id() const;

  // A single element w/ information on all the channels
  class Slice;
  class Element {
  public:
    std::array<glm::vec3, NUM_LAYERS> m_layers;

    glm::vec3& l(std::size_t i) {return m_layers[i];}
    const glm::vec3& l(std::size_t i) const {return m_layers[i];}
    glm::vec3& operator[](std::size_t i) {return m_layers[i];}
    const glm::vec3& operator[](std::size_t i) const {return m_layers[i];}

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

    glm::vec3& l(std::size_t i) {return m_buffs.l(i)[m_offset];}
    const glm::vec3& l(std::size_t i) const {return m_buffs.l(i)[m_offset];}
    glm::vec3& operator[](std::size_t i) {return m_buffs.l(i)[m_offset];}
    const glm::vec3& operator[](std::size_t i) const {return m_buffs.l(i)[m_offset];}


    // Assign values into a buffer (does not change what the slice points to, slices are like a reference)
    Slice &operator=(const Element &elem) {
      for(std::size_t i = 0; i <NUM_LAYERS; ++i){
        this->l(i) = elem[i];
      }
      return *this;
    }
    Slice &operator=(const Slice &sl) {
      for(std::size_t i = 0; i <NUM_LAYERS; ++i){
        this->l(i) = sl[i];
      }
      return *this;
    }

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

float luma(const glm::vec3& c);

void toneMap(QRgb *imageData, glm::vec3 *intensityValues, std::size_t size);

// simple tone map that just clamps to [0,1[(]or [0,255]
void toneMapSimple(QRgb *imageData, glm::vec3 *intensityValues,
                   std::size_t size);

// Save a single image
bool save_image(const QImage &img, const QString &filename);

#endif // UTIL_H
