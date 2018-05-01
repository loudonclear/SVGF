#include "util/util.h"

#include "glm/glm.hpp"

#include <QFileInfo>
#include <QImage>

#include <algorithm>
#include <iostream>

std::unique_ptr<glm::vec3[]> RenderBuffers::recombined() const {
  auto out = std::make_unique<glm::vec3[]>(this->size());
  for (std::size_t i = 0; i < this->size(); ++i) {
    out[i] = (l(DIRECT)[i] + l(INDIRECT)[i]) * l(ALBEDO)[i];
  }
  return out;
}

void RenderBuffers::save(const std::string &directory,
                   const std::string &name) const {
  std::string layer_names[NUM_LAYERS]  = {"albedo",      "direct",  "indirect",
                                         "position_id", "normals", "full"};

  QImage image(m_width, m_height, QImage::Format_RGB32);
  QRgb *data = reinterpret_cast<QRgb *>(image.bits());

  std::string prefix = directory + "/" + name + "_";
  QString filename;

  for (std::size_t i = 0; i < RenderBuffers::NUM_LAYERS; ++i) {
    filename = QString::fromStdString(prefix + layer_names[i] + ".png");
    toneMap(data, l(ALBEDO).get(), this->size());
    save_image(image, filename);
  }

  auto combined = this->recombined();
  filename = QString::fromStdString(prefix + "combined" + ".png");
  toneMap(data, combined.get(), this->size());
  save_image(image, filename);
}

std::unique_ptr<glm::vec4[]> RenderBuffers::interleave_pos_id() const {
  auto pos_id = std::make_unique<glm::vec4[]>(this->size());
  for(std::size_t i=0; i<this->size(); ++i){
    pos_id[i] = glm::vec4(l(POSITION)[i], l(OBJ_ID)[i][0]);
  }
  return pos_id;
}

RenderBuffers::Element& RenderBuffers::Element::operator=(const RenderBuffers::Slice& sl){
for(std::size_t i = 0; i < NUM_LAYERS; ++i){
    l(i) = sl[i];
  }
  return *this;
}

RenderBuffers::Element &RenderBuffers::Element::operator+=(const RenderBuffers::Element &e) {
for(std::size_t i = 0; i < NUM_LAYERS; ++i){
    l(i) += e[i];
  }
  return *this;
}

RenderBuffers::Element &RenderBuffers::Element::operator/=(float f) {
for(std::size_t i = 0; i < NUM_LAYERS; ++i){
    l(i) /= f;
  }
  return *this;
}

RenderBuffers::Element &RenderBuffers::Element::operator*=(float f) {
  for(std::size_t i = 0; i < NUM_LAYERS; ++i){
    l(i) *= f;
  }
  return *this;
}

bool RenderBuffers::Element::operator==(const RenderBuffers::Element &e) const {
  return m_layers == e.m_layers;
}

bool RenderBuffers::Element::operator!=(const RenderBuffers::Element& e) const {
  return !(*this == e);
}

// returns true if ANY elements are NaN
bool RenderBuffers::Element::isnan() const {
  bool out = false;
  for(const auto & l : m_layers){
    out |= glm::any(glm::isnan(l));
  }
  return out;
}

RenderBuffers::Element RenderBuffers::Element::zero() {
  return {{glm::vec3(0.0f)}};
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
