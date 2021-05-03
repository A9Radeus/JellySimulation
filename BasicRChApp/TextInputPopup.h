#pragma once
#include <array>
#include <string>

#include "GuiWidgetBase.h"

namespace rch {

template <size_t BufSize = 256>
class TextInputPopup : public GuiPopup<std::string> {
 public:
  TextInputPopup(const char* winLabel, const char* inputLabel)
      : GuiPopup(winLabel), m_inputFieldLbl(inputLabel) {
    m_frameDims = ImVec2(600.0f, 0);
  }

  virtual void update();

 private:
  std::array<char, BufSize> m_textBuf{
      "saved_scenes/"};  // todo move to the c-tor
  const std::string m_inputFieldLbl;
};

template <size_t BufSize>
inline void TextInputPopup<BufSize>::update() {
  if (m_isOpen) {
    ImGui::OpenPopup(m_winLabel.c_str());
  }
  if (ImGui::BeginPopupModal(m_winLabel.c_str())) {
    ImGui::InputText(m_inputFieldLbl.c_str(), m_textBuf.data(),
                     m_textBuf.size());

    if (ImGui::Button("OK")) {
      m_choiceReady = true;
      m_chosen = std::string(m_textBuf.data());
      ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
    m_isOpen = false;
  }
}

}  // namespace rch