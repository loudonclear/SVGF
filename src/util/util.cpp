#include "util/util.h"

#include "glm/glm.hpp"

#include <QFileInfo>
#include <QImage>

#include <algorithm>
#include <iostream>

std::unique_ptr<glm::vec3[]> RenderBuffers::recombined() const {
  auto out = std::make_unique<glm::vec3[]>(this->size());
  for (std::size_t i = 0; i < this->size(); ++i) {
    out[i] = (m_direct[i] + m_indirect[i]) * m_albedo[i];
  }
  return out;
}


RenderBuffers::Element& RenderBuffers::Element::operator=(const RenderBuffers::Slice& sl){
  m_albedo = sl.albedo();
  m_direct = sl.direct();
  m_indirect = sl.indirect();
  m_full = sl.full();
  return *this;
}

RenderBuffers::Element &RenderBuffers::Element::operator+=(const RenderBuffers::Element &e) {
  m_albedo += e.m_albedo;
  m_direct += e.m_direct;
  m_indirect += e.m_indirect;
  m_full += e.m_full;
  return *this;
}

RenderBuffers::Element &RenderBuffers::Element::operator/=(float f) {
  m_albedo /= f;
  m_direct /= f;
  m_indirect /= f;
  m_full /= f;
  return *this;
}

RenderBuffers::Element &RenderBuffers::Element::operator*=(float f) {
  m_albedo *= f;
  m_direct *= f;
  m_indirect *= f;
  m_full *= f;
  return *this;
}

bool RenderBuffers::Element::operator==(const RenderBuffers::Element &e) const {
  bool b = true;
  b &= (m_albedo == e.m_albedo);
  b &= (m_direct == e.m_direct);
  b &= (m_indirect == e.m_indirect);
  b &= (m_full == e.m_full);
  return b;
}

bool RenderBuffers::Element::operator!=(const RenderBuffers::Element& e) const {
  return !(*this == e);
}

// returns true if ANY elements are NaN
bool RenderBuffers::Element::isnan() const {
  return glm::any(glm::isnan(m_albedo + m_direct + m_indirect + m_full));
}

RenderBuffers::Element RenderBuffers::Element::zero() {
  return {glm::vec3(0.0), glm::vec3(0.0), glm::vec3(0.0), glm::vec3(0.0)};
}


float luma(const glm::vec3& c){
  return glm::dot(c, glm::vec3(0.2126, 0.7152, 0.0722));
}

void toneMap(QRgb *imageData, glm::vec3 *intensityValues, std::size_t size) {
  auto tonemap_lambda = [](const glm::vec3 &v) {
    return qRgb(REAL2byte(v.x / (v.x + 1.f)), REAL2byte(v.y / (v.y + 1.f)),
                REAL2byte(v.z / (v.z + 1.f)));
  };
  std::transform(intensityValues, std::next(intensityValues, size), imageData,
                 tonemap_lambda);
}

// A simple tone map that just clamps intensity values and converts to int's
void toneMapSimple(QRgb *imageData, glm::vec3 *intensityValues,
                   std::size_t size) {
  // o n e l i n e r
  std::transform(intensityValues, std::next(intensityValues, size), imageData,
                 vec3ToQRgb);
}

void save_render_buffers(const RenderBuffers &buffs, std::string directory,
                         std::string name) {
  QImage image(buffs.m_width, buffs.m_height, QImage::Format_RGB32);
  QRgb *data = reinterpret_cast<QRgb *>(image.bits());

  std::string prefix = directory + "/" + name + "_";
  QString filename;

  filename = QString::fromStdString(prefix + "albedo" + ".png");
  toneMap(data, buffs.m_albedo.get(), buffs.size());
  save_image(image, filename);

  filename = QString::fromStdString(prefix + "direct" + ".png");
  toneMap(data, buffs.m_direct.get(), buffs.size());
  save_image(image, filename);

  filename = QString::fromStdString(prefix +  "indirect" + ".png");
  toneMap(data, buffs.m_indirect.get(), buffs.size());
  save_image(image, filename);

  filename = QString::fromStdString(prefix + "full" + ".png");
  toneMap(data, buffs.m_full.get(), buffs.size());
  save_image(image, filename);

  auto combined = buffs.recombined();
  filename = QString::fromStdString(prefix + "combined" + ".png");
  toneMap(data, combined.get(), buffs.size());
  save_image(image, filename);
}

bool save_image(const QImage &img, const QString &filename) {
  bool success = img.save(QFileInfo(filename).absoluteFilePath());
  if (!success) {
    success = img.save(filename, "PNG");
  }
  if (success) {
    std::cout << "Wrote rendered image " << std::endl;
  } else {
    std::cerr << "Error: failed to write image " << std::endl;
  }
  return success;
}
