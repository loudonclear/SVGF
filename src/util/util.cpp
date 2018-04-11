#include "util/util.h"

#include "glm/glm.hpp"

#include <QFileInfo>
#include <QImage>

#include <algorithm>
#include <iostream>

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
