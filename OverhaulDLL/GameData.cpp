/*
	DARK SOULS OVERHAUL
	
	Contributors to this file:
		Ashley						-	Reverse engineering, Low FPS Disconnect patch
		jellybaby34					-	Game version number patch
		Metal-Crow					-	Reverse engineering, Phantom Limit patch
		Youri "NullBy7e" de Mooij	-	Original Bonfire Input Fix
		Sean Pesce					-	C++
	
*/

#include "DllMain.h"



/*
	Initialize static variables:
*/

// Base address of Dark Souls game process
void *GameData::ds1_base = NULL;

// Base address for player character data
void *GameData::player_char_base = NULL;

// Player character status (loading, human, co-op, invader, hollow)
SpPointer GameData::player_char_status;

// Game saving on/off
SpPointer GameData::saves_enabled;

// Multiplayer node count
int GameData::node_count = -1;





// Initializes pointers that depend on the game's base address
void GameData::init_pointers()
{
	print_console(std::string("[Overhaul Mod] Initializing pointers...").c_str());

	// Obtain base address for player character data
	GameData::player_char_base = (void*)((unsigned int)GameData::ds1_base + 0xF7E204);

	// Player character status (loading, human, co-op, invader, hollow)
	GameData::player_char_status = SpPointer(GameData::player_char_base, { 0xA28 });

	// Game saving on/off
	GameData::saves_enabled = SpPointer((void*)0x13784A0, { 0xB40 });
}


// Changes the game version number to avoid compatibility issues with different game builds
void GameData::set_game_version(uint8_t version_number)
{
	std::stringstream hex_stream;
	hex_stream << std::hex << version_number;
	print_console(std::string("[Overhaul Mod] Setting game version number = 0x").append(hex_stream.str()).append("...").c_str());

	uint8_t patch1[5] = { 0xC6, 0x44, 0x24, 0x1C, version_number }; // mov byte ptr [esp+0x1C],0x3C
	uint8_t patch2[3] = { 0x80, 0x38, version_number }; // cmp byte ptr [eax],0x3C

	void *write_address = (uint8_t*)GameData::ds1_base + 0x7E73FA;
	apply_byte_patch(write_address, patch1, 5);

	write_address = (uint8_t*)GameData::ds1_base + 0x7E719D;
	apply_byte_patch(write_address, patch2, 3);

	write_address = (uint8_t*)GameData::ds1_base + 0x7E7229;
	apply_byte_patch(write_address, patch2, 3);
}


// Checks if the player is stuck at the bonfire, and if so, automatically applies the bonfire input fix
void GameData::check_bonfire_input_bug()
{
	// Check that the mod is fully loaded
	if (!ModData::initialized)
		return;

	/*
		If glitch conditions do not exist, this variable is set to 0.
		When bonfire input glitch is detected and first_detected is 0, the time is logged in first_detected.

		If the glitch is detected, and the time in first_detected is more than 1 second ago, the bonfire input fix is applied.
	*/
	static DWORD first_detected; // (Static variable persists between function calls)

	// Get current player status
	int status = DS1_PLAYER_STATUS_LOADING;
	if (player_char_status.read(&status) != ERROR_SUCCESS)
	{
		// Error reading memory location
		first_detected = 0;
		return;
	}


	// Check for bonfire input glitch conditions
	if (status == DS1_PLAYER_STATUS_HOLLOW || status == DS1_PLAYER_STATUS_HUMAN) // Check if player is hollow/human
	{
		// Nested if-statements are necessary to avoid crashes related to resolving multi-level pointers

		// Get pointer to current character animation
		SpPointer bonfire_anim_fix = SpPointer((void*)0x12E29E8, { 0x0, 0xFC });

		// Read current character animation
		uint32_t current_anim;
		if (bonfire_anim_fix.read(&current_anim) != ERROR_SUCCESS)
		{
			// Error reading memory location
			first_detected = 0;
			return;
		}


		// Read bonfire menu flag
		SpPointer bonfire_menu_flag = SpPointer((void*)((uint32_t)GetModuleHandle(NULL) + 0xF786D0), { 0x40 });
		uint8_t bonfire_menu_is_open;
		if (bonfire_menu_flag.read(&bonfire_menu_is_open) != ERROR_SUCCESS)
		{
			first_detected = 0;
			return;
		}

		// Read repair menu flag
		SpPointer repair_menu_flag = SpPointer((void*)((uint32_t)GetModuleHandle(NULL) + 0xF786D0), { 0x4C });
		uint8_t repair_menu_is_open;
		if (repair_menu_flag.read(&repair_menu_is_open) != ERROR_SUCCESS)
		{
			first_detected = 0;
			return;
		}

		// Read level up menu flag
		SpPointer level_menu_flag = SpPointer((void*)((uint32_t)GetModuleHandle(NULL) + 0xF786D0), { 0x78 });
		uint8_t level_menu_is_open;
		if (level_menu_flag.read(&level_menu_is_open) != ERROR_SUCCESS)
		{
			first_detected = 0;
			return;
		}

		// Read bottomless box menu flag
		SpPointer bottomless_menu_flag = SpPointer((void*)((uint32_t)GetModuleHandle(NULL) + 0xF786D0), { 0x84 });
		uint8_t bottomless_menu_is_open;
		if (bottomless_menu_flag.read(&bottomless_menu_is_open) != ERROR_SUCCESS)
		{
			first_detected = 0;
			return;
		}

		// Read attune magic menu flag
		SpPointer attune_menu_flag = SpPointer((void*)((uint32_t)GetModuleHandle(NULL) + 0xF786D0), { 0x80 });
		uint8_t attune_menu_is_open;
		if (attune_menu_flag.read(&attune_menu_is_open) != ERROR_SUCCESS)
		{
			first_detected = 0;
			return;
		}

		// Read reinforce weapon/armor menu flag
		SpPointer reinforce_menu_flag = SpPointer((void*)((uint32_t)GetModuleHandle(NULL) + 0xF786D0), { 0x50 });
		uint8_t reinforce_menu_is_open;
		if (reinforce_menu_flag.read(&reinforce_menu_is_open) != ERROR_SUCCESS)
		{
			first_detected = 0;
			return;
		}

		// Read warp menu flag
		SpPointer warp_menu_flag = SpPointer((void*)((uint32_t)GetModuleHandle(NULL) + 0xF786D0), { 0xAC });
		uint8_t warp_menu_is_open;
		if (warp_menu_flag.read(&warp_menu_is_open) != ERROR_SUCCESS)
		{
			first_detected = 0;
			return;
		}

		// Read "reverse hollowing?"/"can't kindle"/"can't reverse hollowing"/"insufficient attunement slots" dialog flag
		SpPointer dialog_menu_flag = SpPointer((void*)((uint32_t)GetModuleHandle(NULL) + 0xF786D0), { 0x60 });
		uint8_t dialog_menu_is_open;
		if (dialog_menu_flag.read(&dialog_menu_is_open) != ERROR_SUCCESS)
		{
			first_detected = 0;
			return;
		}


		// Check bonfire input fix conditions
		if ( (current_anim == 7701 || current_anim == 7711 || current_anim == 7721) // 3 different bonfire resting animation IDs
			&& !bonfire_menu_is_open && !repair_menu_is_open && !bottomless_menu_is_open && !reinforce_menu_is_open
			&& !level_menu_is_open && !attune_menu_is_open && !dialog_menu_is_open && !warp_menu_is_open) // Make sure no bonfire menus are open
		{
			// Bonfire input glitch conditions currently exist

			if (first_detected == 0)
			{
				// First detection; log time
				first_detected = GetTickCount();
				return;
			}
			else if ((GetTickCount() - first_detected) >= 1000)
			{
				// Too much time has elapsed; bonfire input glitch is definitely present
				fix_bonfire_input();
				print_console(_DS1_MOD_MSG_BONFIRE_INPUT_FIX_AUTOMATIC_);
			}
			else
			{
				// Not enough time has elapsed; return and check again later
				return;
			}
		}
	}

	first_detected = 0;
}


// Fixes input bug that causes players to be stuck at a bonfire (usually after turning human with framerate unlocked)
int GameData::fix_bonfire_input(bool print_to_text_feed, bool print_to_console)
{
	// Get current player status
	int status = DS1_PLAYER_STATUS_LOADING;
	if (player_char_status.read(&status) != ERROR_SUCCESS)
	{
		// Error reading memory location
		if (print_to_text_feed)
			print(_DS1_MOD_MSG_BONFIRE_INPUT_FIX_NO_INIT_FAIL_);

		if (print_to_console)
			print_console(_DS1_MOD_MSG_BONFIRE_INPUT_FIX_NO_INIT_FAIL_);
		return ERROR_INVALID_ADDRESS;
	}


	if (status == DS1_PLAYER_STATUS_HOLLOW || status == DS1_PLAYER_STATUS_HUMAN) // Check if player is hollow/human
	{
		// Get pointer to current character animation
		SpPointer bonfire_anim_fix = SpPointer((void*)0x12E29E8, { 0x0, 0xFC });

		// Read current character animation
		uint32_t current_anim;
		bonfire_anim_fix.read(&current_anim);

		// Check that player is currently sitting at a bonfire
		if (current_anim == 7701 || current_anim == 7711 || current_anim == 7721) // 3 different bonfire resting animation IDs
		{
			// Write zero to bonfire animation status address
			bonfire_anim_fix.write((uint32_t)0);

			if (print_to_text_feed)
				print(_DS1_MOD_MSG_BONFIRE_INPUT_FIX_SUCCESS_);

			if (print_to_console)
				print_console(_DS1_MOD_MSG_BONFIRE_INPUT_FIX_SUCCESS_);

			return ERROR_SUCCESS;
		}
		else
		{
			// Player is not at a bonfire
			if (print_to_text_feed)
				print(_DS1_MOD_MSG_BONFIRE_INPUT_FIX_NOT_AT_BONFIRE_FAIL_);

			if (print_to_console)
				print_console(_DS1_MOD_MSG_BONFIRE_INPUT_FIX_NOT_AT_BONFIRE_FAIL_);
		}
	}
	else
	{
		// Player is not hollow/human, so can't be at a bonfire
		if (print_to_text_feed)
			print(_DS1_MOD_MSG_BONFIRE_INPUT_FIX_NO_CHAR_FAIL_);

		if (print_to_console)
			print_console(_DS1_MOD_MSG_BONFIRE_INPUT_FIX_NO_CHAR_FAIL_);
	}

	return ERROR_BAD_ENVIRONMENT;
}


// Returns multiplayer node count as an int (or -1 if player is not online)
int GameData::get_node_count()
{
	SpPointer node_count_ptr = SpPointer((uint8_t*)ds1_base + 0xF7F77C, { 0x2C, 0x778, 0x80 });

	// Get node count
	int nodes = -1;
	node_count_ptr.read(&nodes);
	return nodes;
}




/////////////////////////////////////////
//////////////// PATCHES ////////////////
/////////////////////////////////////////


// Disables automatic game disconnection when low framerate is detected
void GameData::low_fps_disconnect_enabled(bool enable)
{
	print_console("[Overhaul Mod] Disabling low FPS disconnect...");

	uint8_t *fps_warn = NULL;
	fps_warn = (uint8_t*)aob_scan("74 17 B9 ?? ?? ?? ?? E8 ?? ?? ?? ?? 8B"); // Careful, this pattern returns 2 results

	if (fps_warn)
	{
		// AoB Scan was successful
		set_mem_protection(fps_warn, 1, MEM_PROTECT_RWX);
		fps_warn -= 0x1E;
		
		if (enable)
			*fps_warn = 0x51; // Enable low FPS disconnect
		else
			*fps_warn = 0xC3; // Disable low FPS disconnect
	}
}
