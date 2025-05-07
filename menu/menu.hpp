#pragma once

#include <memory>

class c_menu {
    int m_selected_tab{ };
    float m_alpha{ 0.0f }; // Для fade-in анимации
    float m_tab_hovers[8]{}; // Для анимации наведения на вкладки
    float m_content_slide{ 0.0f }; // Для анимации "въезжания" контента

public:
    bool m_opened;

    void initialize(); // Инициализация стилей и шрифтов
    void draw();
    void on_create_move();
};

inline const auto g_menu = std::make_unique<c_menu>();