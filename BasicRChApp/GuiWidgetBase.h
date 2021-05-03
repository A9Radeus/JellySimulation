#pragma once
#include <string>

#include "imgui.h"
//#include "imgui_impl_dx11.h"
//#include "imgui_impl_win32.h"

namespace rch {

class GuiWidget {
 public:
  GuiWidget() = default;
  virtual ~GuiWidget() = default;

  GuiWidget(const GuiWidget&) = delete;
  GuiWidget& operator=(const GuiWidget&) = delete;

  GuiWidget(GuiWidget&&) = delete;
  GuiWidget& operator=(GuiWidget&&) = delete;

  virtual void update() = 0;
  /* ImGui handles rendering on its own */
};

// PTODO pointer to DataType instead?
template <typename DataType>
class GuiPopup {
 public:
  GuiPopup(const char* label) : m_winLabel(label), m_chosen(){};
  virtual ~GuiPopup() = default;

  GuiPopup(const GuiPopup&) = delete;
  GuiPopup& operator=(const GuiPopup&) = delete;

  GuiPopup(GuiPopup&&) = delete;
  GuiPopup& operator=(GuiPopup&&) = delete;

  virtual void update() = 0;
  // rendering's done by the ImGui

  void open() { m_isOpen = true; }
  void close() { m_isOpen = false; }

  bool chosenReady() const { return m_choiceReady; }

  virtual bool getChosen(DataType* result) {
    if (m_choiceReady == false) {
      return false;
    }
    *result = m_chosen;
    m_choiceReady = false;
    return true;
  }

 protected:
  std::string m_winLabel;
  DataType m_chosen;
  ImVec2 m_frameDims;

  bool m_choiceReady = false;
  bool m_isOpen = false;
};

template <>
class GuiPopup<void> {
 public:
  GuiPopup(const char* label) : m_winLabel(label){};
  virtual ~GuiPopup() = default;

  GuiPopup(const GuiPopup&) = delete;
  GuiPopup& operator=(const GuiPopup&) = delete;

  GuiPopup(GuiPopup&&) = delete;
  GuiPopup& operator=(GuiPopup&&) = delete;

  virtual void update() = 0;
  // rendering's done by the ImGui

  void open() { m_isOpen = true; }
  void close() { m_isOpen = false; }

 protected:
  std::string m_winLabel;
  // DataType m_chosen;
  ImVec2 m_frameDims;

  bool m_choiceReady = false;
  bool m_isOpen = false;
};

static bool inputFloatClamped(const char* lbl, float* var, float min, float max,
                              float defaultVal = 1.0f, float step = 0.1f) {
  if (ImGui::InputFloat(lbl, var, step, step * 2, "%f")) {
    if ((*var < min) || (*var > max)) {
      *var = defaultVal;
    }
    return true;
  }
  return false;
}

static void inputFloat_Disabled(const char* lbl, const float var, float min,
                                float max, float step = 0.1f) {
  float buff = var;
  ImGui::InputFloat(lbl, &buff, step, step * 2, "%f");
}

static void inputIntClamped(const char* lbl, int* var, int min, int max,
                            int defaultVal = 1.0f, int step = 1) {
  if (ImGui::InputInt(lbl, var, step, step * 2)) {
    if ((*var < min) || (*var > max)) {
      *var = defaultVal;
    }
  }
}

class ComboBoxWidget {
 public:
  using SizeType = std::vector<std::string>::size_type;

  ComboBoxWidget(std::vector<std::string>&& items, const char* label,
                 SizeType selected = 0)
      : m_items(items), m_label(label), m_sel(selected){};
  ComboBoxWidget(const std::vector<std::string>& items, const char* label,
                 SizeType selected = 0)
      : m_items(items), m_label(label), m_sel(selected){};

  void update() {
    const auto& currSelName = m_items[m_sel];
    if (ImGui::BeginCombo(m_label, currSelName.c_str())) {
      auto idx = 0;
      for (const auto& item : m_items) {
        if (ImGui::Selectable(item.c_str(), m_sel == idx)) {
          m_sel = idx;
        }
        idx += 1;
      }
      ImGui::EndCombo();
    }
  }

  SizeType getSelectedIdx() const { return m_sel; }
  std::string getSelected() const { return m_items[m_sel]; }
  const std::string& getSelectedRef() const { return m_items[m_sel]; }

 private:
  std::vector<std::string> m_items;
  const char* m_label;
  // initially selected item (before user chooses another)
  SizeType m_sel;
};

}  // namespace rch
