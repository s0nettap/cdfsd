#pragma once

#include <memory>

class c_menu {
    int m_selected_tab{ };
    float m_alpha{ 0.0f }; // ��� fade-in ��������
    float m_tab_hovers[8]{}; // ��� �������� ��������� �� �������
    float m_content_slide{ 0.0f }; // ��� �������� "���������" ��������

public:
    bool m_opened;

    void initialize(); // ������������� ������ � �������
    void draw();
    void on_create_move();
};

inline const auto g_menu = std::make_unique<c_menu>();