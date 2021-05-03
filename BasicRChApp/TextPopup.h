#pragma once
#include <array>
#include <string>

#include "GuiWidgetBase.h"

namespace rch {

class TextPopup : public GuiPopup<void> {
 public:
  TextPopup(const char* winLabel, const char* txt)
      : GuiPopup(winLabel), m_text(txt) {
    m_frameDims = ImVec2(600.0f, 0);
  }

  virtual void update() override {
    if (m_isOpen) {
      ImGui::OpenPopup(m_winLabel.c_str());
    }
    if (ImGui::BeginPopupModal(m_winLabel.c_str())) {
      ImGui::Text(m_text.c_str());

      if (ImGui::Button("OK")) {
        ImGui::CloseCurrentPopup();
      }

      ImGui::EndPopup();
      m_isOpen = false;
    }
  };
  // virtual bool getChosen(void* result) override { return false; }

  void setText(const std::string& txt) { m_text = txt; }
  void setText(std::string&& txt) { m_text = std::move(txt); }

 private:
  std::string m_text;
};

// void TextPopup::update() {
//  if (m_isOpen) {
//    ImGui::OpenPopup(m_winLabel.c_str());
//  }
//  if (ImGui::BeginPopupModal(m_winLabel.c_str())) {
//    ImGui::Text(m_text.c_str());
//
//    if (ImGui::Button("OK")) {
//      ImGui::CloseCurrentPopup();
//    }
//
//    ImGui::EndPopup();
//    m_isOpen = false;
//  }
//}

}  // namespace rch