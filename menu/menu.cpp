#include "menu.hpp"
#include "../darkside.hpp"
#include "../features/skins/skins.hpp"
#include "../features/movement/movement.hpp"
#include "../features/anti_hit/anti_hit.hpp"
#include "../features/rage_bot/rage_bot.hpp"
#include "../features/visuals/visuals.hpp"
#include "C:\\Users\\עמע סאלûי\\Downloads\\darkside_sdk-fix-master\\darkside_sdk-fix-master\\darkside_sdk\\sdk\\includes\\imgui\\imgui.h"
#include "C:\\Users\\עמע סאלûי\\Downloads\\darkside_sdk-fix-master\\darkside_sdk-fix-master\\darkside_sdk\\sdk\\config_system\\config_system.hpp"
#include <algorithm>
#include <string>

void c_menu::initialize() {
    auto& style = ImGui::GetStyle();

    style.WindowPadding = ImVec2(6.0f, 6.0f);
    style.FramePadding = ImVec2(3.0f, 1.0f);
    style.ItemSpacing = ImVec2(4.0f, 2.0f);
    style.ItemInnerSpacing = ImVec2(2.0f, 2.0f);
    style.IndentSpacing = 12.0f;
    style.ScrollbarSize = 4.0f;

    style.WindowRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.TabRounding = 0.0f;
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 0.0f;

    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.01f, 0.01f, 0.03f, 1.0f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.03f, 0.03f, 0.05f, 1.0f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.98f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.55f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.10f, 0.20f, 0.60f, 0.50f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.25f, 0.70f, 0.70f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.08f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.08f, 0.12f, 0.20f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.10f, 0.15f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.15f, 0.25f, 0.70f, 1.0f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.20f, 0.30f, 0.80f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.15f, 0.25f, 0.70f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.08f, 1.0f);

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\verdana.ttf", 12.0f);
}

void c_menu::draw() {
    if (!m_opened) {
        m_alpha = 0.0f;
        m_content_slide = 0.0f;
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    m_alpha = (std::min)(m_alpha + io.DeltaTime * 3.0f, 1.0f);

    ImGui::SetNextWindowSize(ImVec2(650.0f, 400.0f), ImGuiCond_Once);
    ImGui::SetNextWindowBgAlpha(m_alpha);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Darkside CS2 Cheat", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

    ImGui::BeginChild("sidebar", ImVec2(100.0f, 0.0f), false);
    {
        static constexpr const char* tabs[] = { "Ragebot", "Anti-Aim", "World ESP", "Player ESP", "Movement", "Misc", "Skins", "Settings" };
        for (size_t i = 0; i < IM_ARRAYSIZE(tabs); ++i) {
            bool is_hovered = ImGui::IsItemHovered();
            m_tab_hovers[i] = (std::max)(0.0f, (std::min)(m_tab_hovers[i] + io.DeltaTime * (is_hovered ? 5.0f : -5.0f), 1.0f));
            if (m_selected_tab == i) {
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.15f, 0.25f, 0.70f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            }
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.20f, 0.60f, 0.50f * m_tab_hovers[i]));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.25f, 0.70f, 0.70f));
            if (ImGui::Button(tabs[i], ImVec2(-1.0f, 25.0f))) {
                m_selected_tab = i;
                m_content_slide = 0.0f;
            }
            ImGui::PopStyleColor(3);
            if (m_selected_tab == i) {
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
            }
        }
    }
    ImGui::EndChild();
    ImGui::SameLine();

    ImGui::BeginChild("content", ImVec2(0, 0), false);
    {
        m_content_slide = (std::min)(m_content_slide + io.DeltaTime * 4.0f, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_content_slide);
        ImGui::BeginChild("content_inner", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (1.0f - m_content_slide) * -50.0f);
            if (m_selected_tab == 0) {
                ImGui::Text("Enabled"); ImGui::SameLine(100); ImGui::Checkbox("##Enabled", &g_cfg->rage_bot.m_enabled);
                ImGui::Text("Silent Aim"); ImGui::SameLine(100); ImGui::Checkbox("##SilentAim", &g_cfg->rage_bot.m_silent);
                const char* hitboxes[] = { "Head", "Chest", "Stomach", "Arms", "Legs", "Feet" };
                ImGui::Text("Hitboxes"); ImGui::SameLine(100); ImGui::MultiCombo("##Hitboxes", g_cfg->rage_bot.m_hitboxes, hitboxes, IM_ARRAYSIZE(hitboxes));
                ImGui::Text("Multi-Points"); ImGui::SameLine(100); ImGui::MultiCombo("##MultiPoints", g_cfg->rage_bot.m_multi_points, hitboxes, IM_ARRAYSIZE(hitboxes));
                ImGui::Text("Min Damage"); ImGui::SameLine(100); ImGui::SliderInt("##MinDamage", &g_cfg->rage_bot.m_minimum_damage, 1, 100, "%d");
                ImGui::Text("Dmg Override"); ImGui::SameLine(100); ImGui::SliderInt("##DamageOverride", &g_cfg->rage_bot.m_minimum_damage_override, 1, 100, "%d");
                ImGui::Text("Override Key"); ImGui::SameLine(100); ImGui::Keybind("##OverrideKey", &g_cfg->rage_bot.m_override_damage_key_bind, &g_cfg->rage_bot.m_override_damage_key_bind_style);
                ImGui::Text("Hit Chance"); ImGui::SameLine(100); ImGui::SliderInt("##HitChance", &g_cfg->rage_bot.m_hit_chance, 1, 100, "%d%%");
                ImGui::Text("Auto Fire"); ImGui::SameLine(100); ImGui::Checkbox("##AutoFire", &g_cfg->rage_bot.m_auto_fire);
                ImGui::Text("Auto Stop"); ImGui::SameLine(100); ImGui::Checkbox("##AutoStop", &g_cfg->rage_bot.m_auto_stop);
            }
            if (m_selected_tab == 1) {
                ImGui::Text("Enabled"); ImGui::SameLine(100); ImGui::Checkbox("##AntiAimEnabled", &g_cfg->anti_hit.m_enabled);
                ImGui::Text("Override Pitch"); ImGui::SameLine(100); ImGui::Checkbox("##OverridePitch", &g_cfg->anti_hit.m_override_pitch);
                if (g_cfg->anti_hit.m_override_pitch) {
                    ImGui::Text("Pitch Angle"); ImGui::SameLine(100); ImGui::SliderInt("##PitchAngle", &g_cfg->anti_hit.m_pitch_amount, -89, 89, "%d°");
                }
                ImGui::Text("Override Yaw"); ImGui::SameLine(100); ImGui::Checkbox("##OverrideYaw", &g_cfg->anti_hit.m_override_yaw);
                if (g_cfg->anti_hit.m_override_yaw) {
                    ImGui::Text("At Target"); ImGui::SameLine(100); ImGui::Checkbox("##AtTarget", &g_cfg->anti_hit.m_at_target);
                    ImGui::Text("Yaw Angle"); ImGui::SameLine(100); ImGui::SliderInt("##YawAngle", &g_cfg->anti_hit.m_yaw_amount, 0, 180, "%d°");
                    ImGui::Text("Jitter Range"); ImGui::SameLine(100); ImGui::SliderInt("##JitterRange", &g_cfg->anti_hit.m_jitter_amount, 0, 180, "%d°");
                }
            }
            if (m_selected_tab == 2) {
                ImGui::Text("Third Person"); ImGui::SameLine(100); ImGui::Checkbox("##ThirdPerson", &g_cfg->world_esp.m_enable_thirdperson);
                ImGui::Text("Distance"); ImGui::SameLine(100); ImGui::SliderInt("##Distance", &g_cfg->world_esp.m_distance, 0, 180, "%d");
                ImGui::Text("Third Person Key"); ImGui::SameLine(100); ImGui::Keybind("##ThirdPersonKey", &g_cfg->world_esp.m_thirdperson_key_bind, &g_cfg->world_esp.m_thirdperson_key_bind_style);
                ImGui::Text("Render Fog"); ImGui::SameLine(100); ImGui::Checkbox("##RenderFog", &g_cfg->world.m_render_fog);
                ImGui::Text("Sky Color"); ImGui::SameLine(100); ImGui::ColorEdit4("##SkyColor", reinterpret_cast<float*>(&g_cfg->world.m_sky), ImGuiColorEditFlags_NoInputs);
                ImGui::Text("Sky Clouds"); ImGui::SameLine(100); ImGui::ColorEdit4("##SkyClouds", reinterpret_cast<float*>(&g_cfg->world.m_sky_clouds), ImGuiColorEditFlags_NoInputs);
                ImGui::Text("Wall Color"); ImGui::SameLine(100); ImGui::ColorEdit4("##WallColor", reinterpret_cast<float*>(&g_cfg->world.m_wall), ImGuiColorEditFlags_NoInputs);
                ImGui::Text("Lighting Color"); ImGui::SameLine(100); ImGui::ColorEdit4("##LightingColor", reinterpret_cast<float*>(&g_cfg->world.m_lighting), ImGuiColorEditFlags_NoInputs);
                ImGui::Text("Exposure"); ImGui::SameLine(100); ImGui::SliderInt("##Exposure", reinterpret_cast<int*>(&g_cfg->world.m_exposure), 1, 100, "%d%%");
            }
            if (m_selected_tab == 3) {
                ImGui::Text("Teammates"); ImGui::SameLine(100); ImGui::Checkbox("##Teammates", &g_cfg->visuals.m_player_esp.m_teammates);
                ImGui::Text("Bounding Box"); ImGui::SameLine(100); ImGui::Checkbox("##BoundingBox", &g_cfg->visuals.m_player_esp.m_bounding_box);
                ImGui::Text("Health Bar"); ImGui::SameLine(100); ImGui::Checkbox("##HealthBar", &g_cfg->visuals.m_player_esp.m_health_bar);
                ImGui::Text("Name"); ImGui::SameLine(100); ImGui::Checkbox("##Name", &g_cfg->visuals.m_player_esp.m_name);
                ImGui::Text("Weapon"); ImGui::SameLine(100); ImGui::Checkbox("##Weapon", &g_cfg->visuals.m_player_esp.m_weapon);
                ImGui::Text("Name Color"); ImGui::SameLine(100); ImGui::ColorEdit4("##NameColor", reinterpret_cast<float*>(&g_cfg->visuals.m_player_esp.m_name_color), ImGuiColorEditFlags_NoInputs);
            }
            if (m_selected_tab == 4) {
                ImGui::Text("Bunny Hop"); ImGui::SameLine(100); ImGui::Checkbox("##BunnyHop", &g_cfg->misc.m_bunny_hop);
                ImGui::Text("Auto Strafe"); ImGui::SameLine(100); ImGui::Checkbox("##AutoStrafe", &g_cfg->misc.m_auto_strafe);
                if (g_cfg->misc.m_auto_strafe) {
                    ImGui::Text("Strafe Smoothness"); ImGui::SameLine(100); ImGui::SliderInt("##StrafeSmoothness", &g_cfg->misc.m_strafe_smooth, 1, 99, "%d%%");
                }
            }
            if (m_selected_tab == 5) {
                ImGui::Text("Under Development...");
            }
            if (m_selected_tab == 6) {
                static constexpr const char* type[] = { "Weapons", "Knives", "Gloves", "Agents" };
                ImGui::Text("Type"); ImGui::SameLine(100); ImGui::Combo("##Type", &g_cfg->skins.m_selected_type, type, IM_ARRAYSIZE(type));
                if (g_cfg->skins.m_selected_type == 0) {
                    static std::vector<const char*> weapon_names;
                    if (weapon_names.size() < g_skins->m_dumped_items.size()) {
                        weapon_names.clear();
                        for (auto& item : g_skins->m_dumped_items) {
                            weapon_names.emplace_back(item.m_name);
                        }
                    }
                    ImGui::Text("Weapon"); ImGui::SameLine(100); ImGui::Combo("##Weapon", &g_cfg->skins.m_selected_weapon, weapon_names.data(), weapon_names.size());
                    auto& selected_entry = g_cfg->skins.m_skin_settings[g_cfg->skins.m_selected_weapon];
                    ImGui::Text("Skins"); ImGui::SameLine(100);
                    if (ImGui::BeginListBox("##Skins", ImVec2(0, 100))) {
                        auto& selected_weapon_entry = g_skins->m_dumped_items[g_cfg->skins.m_selected_weapon];
                        for (auto& skin : selected_weapon_entry.m_dumped_skins) {
                            ImGui::PushID(&skin);
                            if (ImGui::Selectable(skin.m_name, selected_entry.m_paint_kit == skin.m_id)) {
                                if (selected_weapon_entry.m_selected_skin == &skin) {
                                    selected_weapon_entry.m_selected_skin = nullptr;
                                }
                                else {
                                    selected_weapon_entry.m_selected_skin = &skin;
                                    selected_entry.m_paint_kit = skin.m_id;
                                }
                            }
                            ImGui::PopID();
                        }
                        ImGui::EndListBox();
                    }
                }
                if (g_cfg->skins.m_selected_type == 1) {
                    ImGui::Text("Knives"); ImGui::SameLine(100); ImGui::Combo("##Knives", &g_cfg->skins.m_knives.m_selected, g_skins->m_knives.m_dumped_knife_name.data(), g_skins->m_knives.m_dumped_knife_name.size());
                }
                if (g_cfg->skins.m_selected_type == 2) {
                    ImGui::Text("Gloves"); ImGui::SameLine(100); ImGui::Combo("##Gloves", &g_cfg->skins.m_gloves.m_selected, g_skins->m_gloves.m_dumped_glove_name.data(), g_skins->m_gloves.m_dumped_glove_name.size());
                }
                if (g_cfg->skins.m_selected_type == 3) {
                    ImGui::Text("CT Agents"); ImGui::SameLine(100); ImGui::Combo("##CTAgents", &g_cfg->skins.m_agents.m_selected_ct, g_skins->m_agents.m_dumped_agent_name.data(), g_skins->m_agents.m_dumped_agent_name.size());
                    ImGui::Text("T Agents"); ImGui::SameLine(100); ImGui::Combo("##TAgents", &g_cfg->skins.m_agents.m_selected, g_skins->m_agents.m_dumped_agent_name.data(), g_skins->m_agents.m_dumped_agent_name.size());
                }
            }
            if (m_selected_tab == 7) {
                ImGui::Text("Settings"); ImGui::SameLine(100); g_config_system->menu();
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::End();
}

void c_menu::on_create_move() {
    if (!m_opened) return;

    if (g_ctx && g_ctx->m_user_cmd) {
        g_ctx->m_user_cmd->m_button_state.m_button_state &= ~(IN_ATTACK | IN_ATTACK2);
        g_ctx->m_user_cmd->m_button_state.m_button_state2 &= ~(IN_ATTACK | IN_ATTACK2);
        g_ctx->m_user_cmd->m_button_state.m_button_state3 &= ~(IN_ATTACK | IN_ATTACK2);
    }
}