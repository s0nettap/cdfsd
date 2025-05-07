#pragma once
#include "../../../sdk/typedefs/vec_t.hpp"
#include "../../../sdk/vfunc/vfunc.hpp"
#include "../../../utils/utils.hpp"

#include "usercmd.pb.hpp"
#include "cs_usercmd.pb.hpp"
#include "network_connection.pb.hpp"
#include "networkbasetypes.pb.hpp"

#define	FL_ONGROUND				(1 << 0)
#define FL_DUCKING				(1 << 1)
#define	FL_WATERJUMP			(1 << 3)
#define FL_ONTRAIN				(1 << 4)
#define FL_INRAIN				(1 << 5)
#define FL_FROZEN				(1 << 6)
#define FL_ATCONTROLS			(1 << 7)
#define	FL_CLIENT				(1 << 8)
#define FL_FAKECLIENT			(1 << 9)
#define	FL_INWATER				(1 << 10)
#define FL_HIDEHUD_SCOPE		(1 << 11)

class c_cs_player_controller;

enum e_button : std::uint32_t
{
    IN_ATTACK = (1 << 0),
    IN_JUMP = (1 << 1),
    IN_DUCK = (1 << 2),
    IN_FORWARD = (1 << 3),
    IN_BACK = (1 << 4),
    IN_USE = (1 << 5),
    IN_CANCEL = (1 << 6),
    IN_LEFT = (1 << 7),
    IN_RIGHT = (1 << 8),
    IN_MOVELEFT = (1 << 9),
    IN_MOVERIGHT = (1 << 10),
    IN_ATTACK2 = (1 << 11),
    IN_RUN = (1 << 12),
    IN_RELOAD = (1 << 13),
    IN_LEFT_ALT = (1 << 14),
    IN_RIGHT_ALT = (1 << 15),
    IN_SCORE = (1 << 16),
    IN_SPEED = (1 << 17),
    IN_WALK = (1 << 18),
    IN_ZOOM = (1 << 19),
    IN_FIRST_WEAPON = (1 << 20),
    IN_SECOND_WEAPON = (1 << 21),
    IN_BULLRUSH = (1 << 22),
    IN_FIRST_GRENADE = (1 << 23),
    IN_SECOND_GRENADE = (1 << 24),
    IN_MIDDLE_ATTACK = (1 << 25),
    IN_USE_OR_RELOAD = (1 << 26)
};

class c_in_button_state
{
public:
    std::byte pad_0000[8]; //0x0000
    uint64_t m_button_state; //0x0008
    uint64_t m_button_state2; //0x0010
    uint64_t m_button_state3; //0x0018
}; //Size: 0x0020
static_assert(sizeof(c_in_button_state) == 0x20);

class c_user_cmd
{
public:
    void* __vfptr; //0x00
    unsigned int m_command_number; //0x004
    c_csgo_user_cmd_pb pb;
    c_in_button_state m_button_state; //0x0040
private:
    std::byte pad_0068[0x8]; //0x0068
public:
    uint32_t random_seed; //0x0070
    float current_time;//0x0074
private:
    std::byte pad_78[16];//0x0078
public:
};

class c_subitck_moves
{
public:
    float m_when; //0x0000
    float m_delta; //0x0004
    uint64_t m_button; //0x0008
    bool m_pressed; //0x0010
    std::byte pad_0011[7]; //0x0011
}; //Size: 0x0018

class c_cs_input_moves
{
public:
    uint64_t m_current_bits; //0x0000
    std::byte pad_0008[24]; //0x0008
    float m_forward_move; //0x0020
    float m_side_move; //0x0024
    std::byte pad_0028[16]; //0x0028
    float m_fraction; //0x0038
    std::byte pad_003C[4]; //0x003C
    int32_t m_last_buttons; //0x0040
    std::byte pad_0044[4]; //0x0044
    bool m_in_move; //0x0048
    std::byte pad_0074[255]; //0x0074
    vec3_t m_view_angles; //0x0158
    std::byte pad_0164[48]; //0x0164
}; //Size: 0x0194

class i_csgo_input
{
public:
    char pad_0000[592]; //0x0000
    bool N0000013B; //0x0250
    bool m_in_third_person; //0x0251
    char pad_0252[6]; //0x0252
    vec3_t m_third_person_angles;

    vec3_t get_view_angles( ) {
        using get_view_angles_t = void* ( __fastcall* )( i_csgo_input*, int );
        static get_view_angles_t get_view_angles = reinterpret_cast<get_view_angles_t>( g_opcodes->scan( g_modules->m_modules.client_dll.get_name( ), "4C 8B C1 85 D2 74 08 48 8D 05 ? ? ? ? C3" ) ); 

        return *reinterpret_cast<vec3_t*>( get_view_angles( this, 0 ) );
    }

    void set_view_angles( vec3_t& view_angles ) {
        using set_view_angles_t = void ( __fastcall* )( i_csgo_input*, int, vec3_t& );
        static set_view_angles_t set_view_angles = reinterpret_cast<set_view_angles_t>( g_opcodes->scan( g_modules->m_modules.client_dll.get_name( ), "85 D2 75 3F 48" ) );

        set_view_angles( this, 0, view_angles );
    }

    c_user_cmd* get_user_cmd(c_cs_player_controller* local_controller)
    {
        static auto get_command_index = reinterpret_cast<void* (__fastcall*)(void*, int*)>(g_opcodes->scan_absolute(g_modules->m_modules.client_dll.get_name(), "E8 ? ? ? ? 8B 8D ? ? ? ? 8D 51", 0x1));
        if (!get_command_index)
            return nullptr;

        int index{};
        get_command_index(local_controller, &index);
        int command_index = index - 1;

        if (command_index == -1)
            command_index = 0xFFFFFFFFLL;

        static auto get_user_cmd_base = reinterpret_cast<void* (__fastcall*)(void*, int)>(g_opcodes->scan_absolute(g_modules->m_modules.client_dll.get_name(), "E8 ? ? ? ? 48 8B CF 4C 8B E8 44 8B B8", 0x1));
        if (!get_user_cmd_base)
            return nullptr;

        static void* cmd_base_address = *reinterpret_cast<void**>(g_opcodes->scan_absolute(g_modules->m_modules.client_dll.get_name(), "48 8B 0D ? ? ? ? E8 ? ? ? ? 48 8B CF 4C 8B E8", 0x3));
        if (!cmd_base_address)
            return nullptr;

        auto user_cmd_base = get_user_cmd_base(cmd_base_address, command_index);
        if (!user_cmd_base)
            return nullptr;

        DWORD sequence_number = *reinterpret_cast<DWORD*>((uintptr_t)user_cmd_base + 0x5C00);

        static auto get_user_cmd = reinterpret_cast<c_user_cmd * (__fastcall*)(void*, DWORD)>(g_opcodes->scan_absolute(g_modules->m_modules.client_dll.get_name(), "E8 ? ? ? ? 48 8B 0D ? ? ? ? 45 33 E4 48 89 44 24", 0x1));
        if (!get_user_cmd)
            return nullptr;

        auto user_cmd = get_user_cmd(local_controller, sequence_number);
        return user_cmd;
    }
};