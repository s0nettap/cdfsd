#include "../darkside.hpp"
#include "../directx/directx.hpp"
#include "../vmt/vmt.hpp"
#include "../features/anti_hit/anti_hit.hpp"
#include "../features/movement/movement.hpp"
#include "../features/eng_pred/eng_pred.hpp"
#include "../entity_system/entity.hpp"
#include "../features/skins/skins.hpp"
#include "../features/visuals/world.hpp"

#include "../valve/classes/c_envy_sky.hpp"
#include "../features/visuals/visuals.hpp"
#include "../features/rage_bot/rage_bot.hpp"
#include "../features/anim_sync/anim_sync.hpp"

#include <random>
#include <chrono>
#include <thread>

using namespace hooks;

bool c_hooks::initialize() {
    if (!g_directx->initialize()) {
        LOG_ERROR(xorstr_("[!] Failed to initialize DirectX in hooks\n"));
        return false;
    }

    LOG_INFO(xorstr_("[*] Initializing MinHook...\n"));
    if (MH_Initialize() != MH_OK) {
        LOG_ERROR(xorstr_("[!] Failed to initialize MinHook\n"));
        return false;
    }

    LOG_INFO(xorstr_("[*] Dumping skin items...\n"));
    g_skins->dump_items();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 0xFF);

    uintptr_t base_offset = dis(gen);
    LOG_INFO(xorstr_("[*] Generated base offset: %u\n"), base_offset);

    // Проверка интерфейсов
    if (!g_interfaces || !g_interfaces->m_csgo_input || !g_interfaces->m_input_system) {
        LOG_ERROR(xorstr_("[!] Interfaces are not initialized\n"));
        return false;
    }

    if (!g_modules || !g_modules->m_modules.client_dll.is_loaded()) {
        LOG_ERROR(xorstr_("[!] Client DLL is not initialized\n"));
        return false;
    }

    // Установка хуков с проверками
    LOG_INFO(xorstr_("[*] Hooking create_move...\n"));
    uintptr_t create_move_addr = VMT::get_v_method(g_interfaces->m_csgo_input, 5);
    if (!create_move_addr) {
        LOG_ERROR(xorstr_("[!] Failed to get create_move address\n"));
        return false;
    }
    create_move::m_create_move.hook(reinterpret_cast<void*>(create_move_addr + base_offset), create_move::hk_create_move);

    LOG_INFO(xorstr_("[*] Hooking enable_cursor...\n"));
    uintptr_t enable_cursor_addr = VMT::get_v_method(g_interfaces->m_input_system, 76);
    if (!enable_cursor_addr) {
        LOG_ERROR(xorstr_("[!] Failed to get enable_cursor address\n"));
        return false;
    }
    enable_cursor::m_enable_cursor.hook(reinterpret_cast<void*>(enable_cursor_addr + base_offset), enable_cursor::hk_enable_cursor);

    LOG_INFO(xorstr_("[*] Hooking validate_view_angles...\n"));
    uintptr_t validate_view_angles_addr = VMT::get_v_method(g_interfaces->m_csgo_input, 7);
    if (!validate_view_angles_addr) {
        LOG_ERROR(xorstr_("[!] Failed to get validate_view_angles address\n"));
        return false;
    }
    validate_view_angles::m_validate_view_angles.hook(reinterpret_cast<void*>(validate_view_angles_addr + base_offset), validate_view_angles::hk_validate_view_angles);

    LOG_INFO(xorstr_("[*] Hooking mouse_input_enabled...\n"));
    uintptr_t mouse_input_enabled_addr = VMT::get_v_method(g_interfaces->m_csgo_input, 19);
    if (!mouse_input_enabled_addr) {
        LOG_ERROR(xorstr_("[!] Failed to get mouse_input_enabled address\n"));
        return false;
    }
    mouse_input_enabled::m_mouse_input_enabled.hook(reinterpret_cast<void*>(mouse_input_enabled_addr + base_offset), mouse_input_enabled::hk_mouse_input_enabled);

    LOG_INFO(xorstr_("[*] Hooking present...\n"));
    if (!g_directx->m_present_address) {
        LOG_ERROR(xorstr_("[!] Present address is null\n"));
        return false;
    }
    present::m_present.hook(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(g_directx->m_present_address) + base_offset), present::hk_present);

    LOG_INFO(xorstr_("[*] Hooking update_global_vars...\n"));
    uintptr_t update_global_vars_addr = reinterpret_cast<uintptr_t>(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 8B 0D ? ? ? ? 4C 8D 05 ? ? ? ? 48 85 D2"));
    if (!update_global_vars_addr) {
        LOG_ERROR(xorstr_("[!] Failed to get update_global_vars address\n"));
        return false;
    }
    update_global_vars::m_update_global_vars.hook(reinterpret_cast<void*>(update_global_vars_addr + base_offset), update_global_vars::hk_update_global_vars);

    LOG_INFO(xorstr_("[*] Hooking frame_stage_notify...\n"));
    uintptr_t frame_stage_notify_addr = reinterpret_cast<uintptr_t>(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 89 5C 24 ? 56 48 83 EC ? 8B 05 ? ? ? ? 8D 5A"));
    if (!frame_stage_notify_addr) {
        LOG_ERROR(xorstr_("[!] Failed to get frame_stage_notify address\n"));
        return false;
    }
    frame_stage_notify::m_frame_stage_notify.hook(reinterpret_cast<void*>(frame_stage_notify_addr + base_offset), frame_stage_notify::hk_frame_stage_notify);

    LOG_INFO(xorstr_("[*] Hooking override_view...\n"));
    uintptr_t override_view_addr = reinterpret_cast<uintptr_t>(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B FA E8"));
    if (!override_view_addr) {
        LOG_ERROR(xorstr_("[!] Failed to get override_view address\n"));
        return false;
    }
    override_view::m_override_view.hook(reinterpret_cast<void*>(override_view_addr + base_offset), override_view::hk_override_view);

    LOG_INFO(xorstr_("[*] Hooking on_add_entity...\n"));
    uintptr_t on_add_entity_addr = reinterpret_cast<uintptr_t>(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 89 74 24 ? 57 48 83 EC ? 48 8B F9 41 8B C0 B9"));
    if (!on_add_entity_addr) {
        LOG_ERROR(xorstr_("[!] Failed to get on_add_entity address\n"));
        return false;
    }
    on_add_entity::m_on_add_entity.hook(reinterpret_cast<void*>(on_add_entity_addr + base_offset), on_add_entity::hk_on_add_entity);

    LOG_INFO(xorstr_("[*] Hooking on_remove_entity...\n"));
    uintptr_t on_remove_entity_addr = reinterpret_cast<uintptr_t>(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 89 74 24 ? 57 48 83 EC ? 48 8B F9 41 8B C0 25"));
    if (!on_remove_entity_addr) {
        LOG_ERROR(xorstr_("[!] Failed to get on_remove_entity address\n"));
        return false;
    }
    on_remove_entity::m_on_remove_entity.hook(reinterpret_cast<void*>(on_remove_entity_addr + base_offset), on_remove_entity::hk_on_remove_entity);

    LOG_INFO(xorstr_("[*] Hooking on_level_init...\n"));
    uintptr_t on_level_init_addr = reinterpret_cast<uintptr_t>(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 89 5C 24 18 56 48 83 EC 30 48"));
    if (!on_level_init_addr) {
        LOG_ERROR(xorstr_("[!] Failed to get on_level_init address\n"));
        return false;
    }
    on_level_init::m_on_level_init.hook(reinterpret_cast<void*>(on_level_init_addr + base_offset), on_level_init::hk_on_level_init);

    LOG_INFO(xorstr_("[*] Hooking on_level_shutdown...\n"));
    uintptr_t on_level_shutdown_addr = reinterpret_cast<uintptr_t>(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 83 EC ? 48 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? 45 33 C9 45 33 C0 48 8B 01 FF 50 ? 48 85 C0 74 ? 48 8B 0D ? ? ? ? 48 8B D0 4C 8B 01 41 FF 50 ? 48 83 C4"));
    if (!on_level_shutdown_addr) {
        LOG_ERROR(xorstr_("[!] Failed to get on_level_shutdown address\n"));
        return false;
    }
    on_level_shutdown::m_on_level_shutdown.hook(reinterpret_cast<void*>(on_level_shutdown_addr + base_offset), on_level_shutdown::hk_on_level_shutdown);

    LOG_INFO(xorstr_("[*] Hooking update_sky_box...\n"));
    uintptr_t update_sky_box_addr = reinterpret_cast<uintptr_t>(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 8B C4 48 89 58 18 48 89 70 20 55 57 41 54 41 55"));
    if (!update_sky_box_addr) {
        LOG_ERROR(xorstr_("[!] Failed to get update_sky_box address\n"));
        return false;
    }
    update_sky_box::m_update_sky_box.hook(reinterpret_cast<void*>(update_sky_box_addr + base_offset), update_sky_box::hk_update_sky_box);

    LOG_INFO(xorstr_("[*] Hooking draw_light_scene...\n"));
    uintptr_t draw_light_scene_addr = reinterpret_cast<uintptr_t>(g_opcodes->scan(g_modules->m_modules.scenesystem_dll.get_name(), "48 89 54 24 ? 53 41 56 41 57"));
    if (!draw_light_scene_addr) {
        LOG_ERROR(xorstr_("[!] Failed to get draw_light_scene address\n"));
        return false;
    }
    draw_light_scene::m_draw_light_scene.hook(reinterpret_cast<void*>(draw_light_scene_addr + base_offset), draw_light_scene::hk_draw_light_scene);

    LOG_INFO(xorstr_("[*] Hooking update_aggregate_scene_object...\n"));
    uintptr_t update_aggregate_scene_object_addr = reinterpret_cast<uintptr_t>(g_opcodes->scan(g_modules->m_modules.scenesystem_dll.get_name(), "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 83 EC ? 4C 8B F95"));
    if (!update_aggregate_scene_object_addr) {
        LOG_ERROR(xorstr_("[!] Failed to get update_aggregate_scene_object address\n"));
        return false;
    }
    update_aggregate_scene_object::m_update_aggregate_scene_object.hook(reinterpret_cast<void*>(update_aggregate_scene_object_addr + base_offset), update_aggregate_scene_object::hk_update_aggregate_scene_object);

    LOG_INFO(xorstr_("[*] Hooking draw_aggregate_scene_object...\n"));
    uintptr_t draw_aggregate_scene_object_addr = reinterpret_cast<uintptr_t>(g_opcodes->scan(g_modules->m_modules.scenesystem_dll.get_name(), "48 89 54 24 ? 55 57 41 55 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 49 63 F9"));
    if (!draw_aggregate_scene_object_addr) {
        LOG_ERROR(xorstr_("[!] Failed to get draw_aggregate_scene_object address\n"));
        return false;
    }
    draw_aggregate_scene_object::m_draw_aggregate_scene_object.hook(reinterpret_cast<void*>(draw_aggregate_scene_object_addr + base_offset), draw_aggregate_scene_object::hk_draw_aggregate_scene_object);

    LOG_INFO(xorstr_("[*] Hooking update_post_processing...\n"));
    uintptr_t update_post_processing_addr = reinterpret_cast<uintptr_t>(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 89 5C 24 08 57 48 83 EC 60 80"));
    if (!update_post_processing_addr) {
        LOG_ERROR(xorstr_("[!] Failed to get update_post_processing address\n"));
        return false;
    }
    update_post_processing::m_update_post_processing.hook(reinterpret_cast<void*>(update_post_processing_addr + base_offset), update_post_processing::hk_update_post_processing);

    LOG_INFO(xorstr_("[*] Hooking should_update_sequences...\n"));
    uintptr_t should_update_sequences_addr = reinterpret_cast<uintptr_t>(g_opcodes->scan(g_modules->m_modules.animation_system.get_name(), "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 49 8B 40 48"));
    if (!should_update_sequences_addr) {
        LOG_ERROR(xorstr_("[!] Failed to get should_update_sequences address\n"));
        return false;
    }
    should_update_sequences::m_should_update_sequences.hook(reinterpret_cast<void*>(should_update_sequences_addr + base_offset), should_update_sequences::hk_should_update_sequences);

    LOG_INFO(xorstr_("[*] Hooking should_draw_legs...\n"));
    uintptr_t should_draw_legs_addr = reinterpret_cast<uintptr_t>(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 01"));
    if (!should_draw_legs_addr) {
        LOG_ERROR(xorstr_("[!] Failed to get should_draw_legs address\n"));
        return false;
    }
    should_draw_legs::m_should_draw_legs.hook(reinterpret_cast<void*>(should_draw_legs_addr + base_offset), should_draw_legs::hk_should_draw_legs);

    LOG_INFO(xorstr_("[*] Hooking mark_interp_latch_flags_dirty...\n"));
    uintptr_t mark_interp_latch_flags_dirty_addr = reinterpret_cast<uintptr_t>(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "85 D2 0F 84 ? ? ? ? 55 57 48 83 EC"));
    if (!mark_interp_latch_flags_dirty_addr) {
        LOG_ERROR(xorstr_("[!] Failed to get mark_interp_latch_flags_dirty address\n"));
        return false;
    }
    mark_interp_latch_flags_dirty::m_mark_interp_latch_flags_dirty.hook(reinterpret_cast<void*>(mark_interp_latch_flags_dirty_addr + base_offset), mark_interp_latch_flags_dirty::hk_mark_interp_latch_flags_dirty);

    LOG_INFO(xorstr_("[*] Hooking draw_scope_overlay...\n"));
    uintptr_t draw_scope_overlay_addr = reinterpret_cast<uintptr_t>(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "4C 8B DC 53 56 57 48 83 EC"));
    if (!draw_scope_overlay_addr) {
        LOG_ERROR(xorstr_("[!] Failed to get draw_scope_overlay address\n"));
        return false;
    }
    draw_scope_overlay::m_draw_scope_overlay.hook(reinterpret_cast<void*>(draw_scope_overlay_addr + base_offset), draw_scope_overlay::hk_draw_scope_overlay);

    LOG_INFO(xorstr_("[*] Hooking get_field_of_view...\n"));
    uintptr_t get_field_of_view_addr = reinterpret_cast<uintptr_t>(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "40 53 48 83 EC ? 48 8B D9 E8 ? ? ? ? 48 85 C0 74 ? 48 8B C8 48 83 C4"));
    if (!get_field_of_view_addr) {
        LOG_ERROR(xorstr_("[!] Failed to get get_field_of_view address\n"));
        return false;
    }
    get_field_of_view::m_get_field_of_view.hook(reinterpret_cast<void*>(get_field_of_view_addr + base_offset), get_field_of_view::hk_get_field_of_view);

    LOG_INFO(xorstr_("[+] Hooks initialization completed!\n"));
    return true;
}

void c_hooks::destroy() {
    g_directx->unitialize();
    hooks::enable_cursor::unhook();

    create_move::m_create_move.unhook();
    enable_cursor::m_enable_cursor.unhook();
    validate_view_angles::m_validate_view_angles.unhook();
    mouse_input_enabled::m_mouse_input_enabled.unhook();
    present::m_present.unhook();
    update_global_vars::m_update_global_vars.unhook();
    frame_stage_notify::m_frame_stage_notify.unhook();
    on_add_entity::m_on_add_entity.unhook();
    on_remove_entity::m_on_remove_entity.unhook();
    on_level_init::m_on_level_init.unhook();
    on_level_shutdown::m_on_level_shutdown.unhook();
    update_sky_box::m_update_sky_box.unhook();
    draw_light_scene::m_draw_light_scene.unhook();
    update_aggregate_scene_object::m_update_aggregate_scene_object.unhook();
    draw_aggregate_scene_object::m_draw_aggregate_scene_object.unhook();
    update_post_processing::m_update_post_processing.unhook();
    should_update_sequences::m_should_update_sequences.unhook();
    should_draw_legs::m_should_draw_legs.unhook();
    mark_interp_latch_flags_dirty::m_mark_interp_latch_flags_dirty.unhook();
    draw_scope_overlay::m_draw_scope_overlay.unhook();
    get_field_of_view::m_get_field_of_view.unhook();

    MH_Uninitialize();
}

void __fastcall hooks::create_move::hk_create_move(i_csgo_input* rcx, int slot, bool active) {
    static auto original = m_create_move.get_original<decltype(&hk_create_move)>();
    original(rcx, slot, active);

    if (!g_ctx->m_local_pawn || !g_ctx->m_local_controller)
        return;

    g_ctx->m_user_cmd = rcx->get_user_cmd(g_ctx->m_local_controller);

    if (!g_ctx->m_user_cmd)
        return;

    g_ctx->m_user_cmd->pb.mutable_base()->clear_subtick_moves();

    const vec3_t old_view_angles = {
        g_ctx->m_user_cmd->pb.mutable_base()->viewangles().x(),
        g_ctx->m_user_cmd->pb.mutable_base()->viewangles().y(),
        g_ctx->m_user_cmd->pb.mutable_base()->viewangles().z()
    };

    g_menu->on_create_move();

    random_delay();
    g_movement->on_create_move(g_ctx->m_user_cmd, old_view_angles.y);

    random_delay();
    g_anti_hit->on_create_move(g_ctx->m_user_cmd);

    random_delay();
    g_prediction->run();

    random_delay();
    g_rage_bot->on_create_move();

    random_delay();
    g_prediction->end();

    random_delay();
    g_movement->movement_fix(g_ctx->m_user_cmd, old_view_angles);

    // Smooth angles to avoid VAC detection
    vec3_t target_angles = {
        g_ctx->m_user_cmd->pb.mutable_base()->viewangles().x(),
        g_ctx->m_user_cmd->pb.mutable_base()->viewangles().y(),
        g_ctx->m_user_cmd->pb.mutable_base()->viewangles().z()
    };
    smooth_angle(target_angles, old_view_angles, 0.05f);
    auto viewangles = g_ctx->m_user_cmd->pb.mutable_base()->mutable_viewangles();
    viewangles->set_x(target_angles.x);
    viewangles->set_y(target_angles.y);
    viewangles->set_z(target_angles.z);
}

bool hooks::mouse_input_enabled::hk_mouse_input_enabled(void* ptr) {
    static auto original = m_mouse_input_enabled.get_original<decltype(&hk_mouse_input_enabled)>();
    random_delay();
    return g_menu->m_opened ? false : original(ptr);
}

void* hooks::enable_cursor::hk_enable_cursor(void* rcx, bool active) {
    static auto original = m_enable_cursor.get_original<decltype(&hk_enable_cursor)>();

    m_enable_cursor_input = active;
    if (g_menu->m_opened)
        active = false;

    random_delay();
    return original(rcx, active);
}

void hooks::enable_cursor::unhook() {
    static auto original = m_enable_cursor.get_original<decltype(&hk_enable_cursor)>();
    random_delay();
    original(g_interfaces->m_input_system, m_enable_cursor_input);
}

HRESULT hooks::present::hk_present(IDXGISwapChain* swap_chain, unsigned int sync_interval, unsigned int flags) {
    static auto original = m_present.get_original<decltype(&hk_present)>();

    g_directx->start_frame(swap_chain);

    g_directx->new_frame();
    {
        random_delay();
        g_world->skybox();
        random_delay();
        g_world->exposure(g_ctx->m_local_pawn);
        random_delay();
        g_render->update_background_drawlist(ImGui::GetBackgroundDrawList());
        random_delay();
        g_visuals->on_present();
        random_delay();
        g_world->draw_scope_overlay();
        random_delay();
        g_menu->draw();
    }
    random_delay();
    g_directx->end_frame();

    return original(swap_chain, sync_interval, flags);
}

void hooks::validate_view_angles::hk_validate_view_angles(i_csgo_input* input, void* a2) {
    static auto original = m_validate_view_angles.get_original<decltype(&hk_validate_view_angles)>();

    vec3_t view_angles = input->get_view_angles();
    random_delay();
    smooth_angle(view_angles, view_angles, 0.1f);
    original(input, a2);
    random_delay();
    input->set_view_angles(view_angles);
}

void hooks::update_global_vars::hk_update_global_vars(void* source_to_client, void* new_global_vars) {
    static auto original = m_update_global_vars.get_original<decltype(&hk_update_global_vars)>();
    random_delay();
    original(source_to_client, new_global_vars);
    random_delay();
    g_interfaces->m_global_vars = *reinterpret_cast<i_global_vars**>(g_opcodes->scan_absolute(g_modules->m_modules.client_dll.get_name(), xorstr_("48 89 15 ? ? ? ? 48 89 42"), 0x3));
}

void hooks::frame_stage_notify::hk_frame_stage_notify(void* source_to_client, int stage) {
    static auto original = m_frame_stage_notify.get_original<decltype(&hk_frame_stage_notify)>();
    random_delay();
    g_ctx->m_local_pawn = g_interfaces->m_entity_system->get_local_pawn();
    g_ctx->m_local_controller = g_interfaces->m_entity_system->get_base_entity<c_cs_player_controller>(g_interfaces->m_engine->get_local_player_index());

    random_delay();
    g_skins->agent_changer(stage);

    random_delay();
    original(source_to_client, stage);

    switch (stage) {
    case FRAME_RENDER_START:
        random_delay();
        g_skins->knife_changer(stage);
        break;
    case FRAME_RENDER_END:
        random_delay();
        g_skins->knife_changer(stage);
        break;
    case FRAME_NET_UPDATE_END:
        random_delay();
        g_rage_bot->store_records();
        break;
    case FRAME_SIMULATE_END:
        random_delay();
        g_visuals->store_players();
        break;
    default:
        break;
    }
}

vec3_t calculate_camera_pos(vec3_t anchor_pos, float distance, vec3_t view_angles) {
    float yaw = DirectX::XMConvertToRadians(view_angles.y);
    float pitch = DirectX::XMConvertToRadians(view_angles.x);

    float x = anchor_pos.x + distance * cosf(yaw) * cosf(pitch);
    float y = anchor_pos.y + distance * sinf(yaw) * cosf(pitch);
    float z = anchor_pos.z + distance * sinf(pitch);

    return vec3_t{ x, y, z };
}

inline vec3_t calculate_angles(vec3_t view_pos, vec3_t aim_pos) {
    vec3_t angle = { 0, 0, 0 };

    vec3_t delta = aim_pos - view_pos;

    angle.x = -asin(delta.z / delta.length()) * _rad_pi;
    angle.y = atan2(delta.y, delta.x) * _rad_pi;

    return angle;
}

void* hooks::override_view::hk_override_view(void* source_to_client, c_view_setup* view_setup) {
    static auto original = m_override_view.get_original<decltype(&hk_override_view)>();
    random_delay();
    original(source_to_client, view_setup);

    c_cs_player_pawn* local_player = g_ctx->m_local_pawn;

    if (!local_player)
        return original(source_to_client, view_setup);

    bool in_third_person = false;
    if (g_key_handler->is_pressed(g_cfg->world_esp.m_thirdperson_key_bind, g_cfg->world_esp.m_thirdperson_key_bind_style))
        in_third_person = true;

    if (in_third_person && local_player->is_alive())
    {
        vec3_t adjusted_cam_view_angle = g_interfaces->m_csgo_input->get_view_angles();
        adjusted_cam_view_angle.x = -adjusted_cam_view_angle.x;
        view_setup->m_origin = calculate_camera_pos(local_player->get_eye_pos(), -g_cfg->world_esp.m_distance, adjusted_cam_view_angle);

        ray_t ray{};
        game_trace_t trace{};
        trace_filter_t filter{ 0x1C3003, local_player, NULL, 4 };

        vec3_t direction = (local_player->get_eye_pos() - view_setup->m_origin).normalize();

        vec3_t temp = view_setup->m_origin + direction * -10.f;

        if (g_interfaces->m_trace->trace_shape(&ray, local_player->get_eye_pos(), view_setup->m_origin, &filter, &trace)) {
            if (trace.m_hit_entity != nullptr)
                view_setup->m_origin = trace.m_pos + (direction * 10.f);
        }

        vec3_t pos = calculate_angles(view_setup->m_origin, local_player->get_eye_pos()).normalize_angle();
        view_setup->m_angles = vec3_t{ pos.x, pos.y };
    }

    random_delay();
    return original(source_to_client, view_setup);
}

void* hooks::on_add_entity::hk_on_add_entity(void* a1, c_entity_instance* entity_instance, int handle) {
    static auto original = m_on_add_entity.get_original<decltype(&hk_on_add_entity)>();
    random_delay();
    g_entity_system->add_entity(entity_instance, handle);
    return original(a1, entity_instance, handle);
}

void* hooks::on_remove_entity::hk_on_remove_entity(void* a1, c_entity_instance* entity_instance, int handle) {
    static auto original = m_on_remove_entity.get_original<decltype(&hk_on_remove_entity)>();
    random_delay();
    g_entity_system->remove_entity(entity_instance, handle);
    return original(a1, entity_instance, handle);
}

void* hooks::on_level_init::hk_on_level_init(void* a1, const char* map_name) {
    static auto original = m_on_level_init.get_original<decltype(&hk_on_level_init)>();
    random_delay();
    g_entity_system->level_init();
    return original(a1, map_name);
}

void* hooks::on_level_shutdown::hk_on_level_shutdown(void* a1, const char* map_name) {
    static auto original = m_on_level_shutdown.get_original<decltype(&hk_on_level_shutdown)>();
    random_delay();
    g_entity_system->level_shutdown();
    return original(a1, map_name);
}

void* hooks::update_sky_box::hk_update_sky_box(c_env_sky* sky) {
    static auto original = m_update_sky_box.get_original<decltype(&hk_update_sky_box)>();
    random_delay();
    g_world->skybox(sky);
    return original(sky);
}

void* hooks::draw_light_scene::hk_draw_light_scene(void* a1, c_scene_light_object* a2, __int64 a3) {
    static auto original = m_draw_light_scene.get_original<decltype(&hk_draw_light_scene)>();
    random_delay();
    g_world->lighting(a2);
    return original(a1, a2, a3);
}

void* hooks::update_aggregate_scene_object::hk_update_aggregate_scene_object(c_aggregate_scene_object* a1, void* a2) {
    static auto original = m_update_aggregate_scene_object.get_original<decltype(&hk_update_aggregate_scene_object)>();
    random_delay();
    void* result = original(a1, a2);

    c_byte_color color = (g_cfg->world.m_wall * 255).to_byte();

    int count = a1->m_count;

    for (int i = 0; i < count; i++) {
        c_aggregate_scene_object_data* scene_object = &a1->m_array[i];
        if (!scene_object)
            continue;

        scene_object->m_color = color;
    }

    return result;
}

enum e_model_type : int
{
    MODEL_SUN,
    MODEL_EFFECTS,
    MODEL_OTHER,
};

int get_model_type(const std::string_view& name) {
    if (name.find("sun") != std::string::npos || name.find("clouds") != std::string::npos)
        return MODEL_SUN;

    if (name.find("effects") != std::string::npos)
        return MODEL_EFFECTS;

    return MODEL_OTHER;
}

void hooks::draw_aggregate_scene_object::hk_draw_aggregate_scene_object(void* a1, void* a2, c_base_scene_data* a3, int a4, int a5, void* a6, void* a7, void* a8) {
    static auto original = m_draw_aggregate_scene_object.get_original<decltype(&hk_draw_aggregate_scene_object)>();
    random_delay();

    int type = get_model_type(a3->m_material->get_name());
    c_byte_color color{};

    switch (type) {
    case MODEL_SUN:
        color = (g_cfg->world.m_sky_clouds * 255).to_byte();
        break;
    case MODEL_EFFECTS:
        color = (g_cfg->world.m_sky_clouds * 255).to_byte();
        break;
    case MODEL_OTHER:
        color = (g_cfg->world.m_wall * 255).to_byte();
        break;
    }

    for (int i = 0; i < a4; ++i)
    {
        auto scene = &a3[i];
        if (scene) {
            scene->r = color.r;
            scene->g = color.g;
            scene->b = color.b;
            scene->a = 255;
        }
    }

    original(a1, a2, a3, a4, a5, a6, a7, a8);
}

void hooks::update_post_processing::hk_update_post_processing(c_post_processing_volume* a1, int a2) {
    static auto original = m_update_post_processing.get_original<decltype(&hk_update_post_processing)>();
    random_delay();
    original(a1, a2);
    random_delay();
    g_world->exposure(a1);
}

void* hooks::should_update_sequences::hk_should_update_sequences(void* a1, void* a2, void* a3) {
    static auto original = m_should_update_sequences.get_original<decltype(&hk_should_update_sequences)>();
    random_delay();
    const char* model_name = *reinterpret_cast<const char**>(reinterpret_cast<std::uintptr_t>(a1) + 0x8);
    if (model_name == nullptr)
        return original(a1, a2, a3);

    std::string str_model_name = std::string(model_name);
    if (str_model_name.starts_with("weapons/models/knife/"))
        *reinterpret_cast<__int64*>(reinterpret_cast<std::uintptr_t>(a3) + 0x30) = 0;

    return original(a1, a2, a3);
}

void* hooks::should_draw_legs::hk_should_draw_legs(void* a1, void* a2, void* a3, void* a4, void* a5) {
    static auto original = m_should_draw_legs.get_original<decltype(&hk_should_draw_legs)>();
    random_delay();
    return nullptr;
}

void hooks::mark_interp_latch_flags_dirty::hk_mark_interp_latch_flags_dirty(void* a1, unsigned int a2) {
    random_delay();
    return;
}

void hooks::draw_scope_overlay::hk_draw_scope_overlay(void* a1, void* a2) {
    random_delay();
}

float hooks::get_field_of_view::hk_get_field_of_view(void* a1) {
    random_delay();
    return 100.f;
}