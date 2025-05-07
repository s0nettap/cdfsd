#pragma once

#include "../../darkside.hpp"

class c_eng_pred {
	struct c_local_data {
		float m_absolute_frame_time{}, m_absolute_frame_start_time_std_dev{}, m_spread{}, m_inaccuracy{}, m_player_tick_fraction{}, m_render_tick_fraction{};
		int m_current_time{}, m_current_time2{}, m_tick_count{}, m_tick_base{}, m_player_tick{}, m_render_tick{}, m_shoot_tick{};
		vec3_t m_velocity{}, m_eye_pos{};
	} m_pred_data{};

	bool m_initialized = false;
public:
	void run( );
	void predict( );
	void end( );

	c_local_data* get_local_data( ) {
		return &m_pred_data;
	}
};

inline const auto g_prediction = std::make_unique<c_eng_pred>( );