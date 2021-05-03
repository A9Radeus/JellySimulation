#pragma once
#include <functional>
#include <cmath>

#include "GuiWidgetBase.h"
#include "Simulation.h"


namespace rch {

class SimulationPopup : public GuiPopup<JellySimParams> {
 public:
  using Params = JellySimParams;

  SimulationPopup(const char* label) : GuiPopup(label){};
  virtual ~SimulationPopup() = default;

  virtual void update() {
    if (m_isOpen) {
      ImGui::OpenPopup(m_winLabel.c_str());
    }
    if (ImGui::BeginPopupModal(m_winLabel.c_str())) {
      //ImGui::Separator();
      ImGui::Text("Simulation: ");
      inputFloatClamped("dt", &(m_chosen.dt), s_eps, 10000, 0.001f);
      inputFloatClamped("rand vel limit", &(m_chosen.maxRandVel), 0.f, 10.f, 0.001f);

      ImGui::Separator();
      ImGui::Text("Cube: ");
      inputFloatClamped("starting side length", &(m_chosen.cubeSide), 0.1f, 5.f, 0.01f);
      if (m_lockAlignment) {
        m_chosen.l0 = m_chosen.cubeSide / static_cast<float>(JellySimParams::partiPerSide - 1);
        inputFloat_Disabled("rest length", m_chosen.l0, 0.01f, 100, 0.01f);
      } else {
        inputFloatClamped("rest length", &(m_chosen.l0), 0.01f, 100, 0.01f);
      }
      ImGui::Checkbox("Inscribe", &m_lockAlignment);


      ImGui::Separator();
      ImGui::Text("Attributes: ");
      inputFloatClamped("mass (1 pt)", &(m_chosen.mass), 0.01f, 100, 0.01f);
      inputFloatClamped("k (damping)", &(m_chosen.k), 0.01f, 100, 0.01f);
      inputFloatClamped("c1 (jelly)", &(m_chosen.c1), 0.01f, 100, 0.01f);
      inputFloatClamped("c2 (frame)", &(m_chosen.c2), 0.01f,  100, 0.01f);

      if (ImGui::Button("Create")) {
        m_choiceReady = true;
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

 private:
  bool m_lockAlignment = true;
  static constexpr float s_eps = 0.0000001f;
};

}  // namespace rch