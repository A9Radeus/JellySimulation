#pragma once
#include <limits>
//#include <cstddef>
#include "imgui.h"
#include "implot.h"

// utility structure for realtime plots
class PltCircularBuffer {
public:
  // capacity = 4096 was too much for ImPlot (for 4 buffers at once*)
  PltCircularBuffer(float stepVal = 0.1f, size_t capacity = 2048,
                 size_t offset = 0)
      : m_step(stepVal), m_capacity(capacity), m_offset(offset), m_stepAcc(0) {
    m_data.reserve(m_capacity);
  }
  
  // Only Y coordinate supports min/max
  void addVal(float x, float y) {
    updateMinMax(y);
    // Fill up the buffer
    if (m_data.size() < m_capacity) {
      m_data.push_back(ImVec2(x, y));
    }
    // Data[tail + 1 % tail]
    else {
      m_data[m_offset] = ImVec2(x, y);
      m_offset = (m_offset + 1) % m_capacity;
    }
  }
  
  void addVal(float val) {
    updateMinMax(val);
    m_stepAcc += m_step;
    // Fill up the buffer
    if (m_data.size() < m_capacity) {
      m_data.push_back(ImVec2(m_stepAcc, val));
    }
    // Data[tail + 1 % tail]
    else {
      m_data[m_offset] = ImVec2(m_stepAcc, val);
      m_offset = (m_offset + 1) % m_capacity;
    }
  }

  void setStep(float step) { m_step = step; }

  const size_t& offset() const { return m_offset; }
  const ImVector<ImVec2>& data() const { return m_data; }
  float step() const { return m_step; }
  float minVal() const { return m_minVal; }
  float maxVal() const { return m_maxVal; }

  // PTODO
  void erase() {
    if (m_data.size() > 0) {
      m_data.shrink(0);
      m_offset = 0;
    }
  }

private:
  void updateMinMax(float val) { 
    if (val < m_minVal) {
      m_minVal = val;
    } else if (val > m_maxVal) {
      m_maxVal = val;
    }
  }

  ImVector<ImVec2> m_data;
  size_t m_capacity;
  size_t m_offset;

  float m_step;
  float m_stepAcc; // accumulator

  // Additional information for plotting
  //float m_maxVal = std::numeric_limits<float>::max();
  float m_maxVal = FLT_MIN;
  float m_minVal = FLT_MAX;
};

static void plotLineCb(const char* lbl, PltCircularBuffer& buff) {
  if (buff.data().size() <= 0) {
    return;
  }
  ImPlot::PlotLine(lbl, &buff.data()[0].x, &buff.data()[0].y,
                   buff.data().size(), buff.offset(), 2 * sizeof(float));
};

static float min3(float a, float b, float c) {
  return min(a, min(b, c));
};

static float min4(float a, float b, float c, float d) {
  return min(a, min(b, min(c, d)));
};

static float max3(float a, float b, float c) {
  return max(a, max(b, c));
};

static float max4(float a, float b, float c, float d) {
  return max(a, max(b, max(c, d)));
};