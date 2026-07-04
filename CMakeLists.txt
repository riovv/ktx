cmake_minimum_required(VERSION 3.10.0)

# Set project name and languge.
project(qwprogs C)

include(CheckCCompilerFlag)

set(CMAKE_COLOR_DIAGNOSTICS ON)

# CMake option for bot support, we check it below and add C preprocessor directive if required.
option(BOT_SUPPORT "Build with bot support" ON)
# Option for FTE extensions. We also have some basic support for FTE which does not controlled by this setting.
# This is en example and extensions unused for now, so its turned off by default. Also turning it ON makes binary incompatibe with MVDSV.
option(FTESV "Build with FTE extensions support" OFF)

# Set where sources located.
set(DIR_SRC "src")

# Sources for native library.
set(SRC_NATIVE
	"${DIR_SRC}/g_syscalls.c"
	"${DIR_SRC}/native_lib.c"
	)
# Common sources.
set(SRC_COMMON
	"${DIR_SRC}/g_main.c"
	"${DIR_SRC}/admin.c"
	"${DIR_SRC}/arena.c"
	"${DIR_SRC}/bot_aim.c"
	"${DIR_SRC}/bot_blocked.c"
	"${DIR_SRC}/bot_botenemy.c"
	"${DIR_SRC}/bot_botgoals.c"
	"${DIR_SRC}/bot_bothazd.c"
	"${DIR_SRC}/bot_bothelp.c"
	"${DIR_SRC}/bot_botimp.c"
	"${DIR_SRC}/bot_botjump.c"
	"${DIR_SRC}/bot_botpath.c"
	"${DIR_SRC}/bot_botphys.c"
	"${DIR_SRC}/bot_botstat.c"
	"${DIR_SRC}/bot_botthink.c"
	"${DIR_SRC}/bot_botwater.c"
	"${DIR_SRC}/bot_botweap.c"
	"${DIR_SRC}/bot_client.c"
	"${DIR_SRC}/bot_commands.c"
	"${DIR_SRC}/bot_items.c"
	"${DIR_SRC}/bot_load.c"
	"${DIR_SRC}/bot_loadmap.c"
	"${DIR_SRC}/bot_match.c"
	"${DIR_SRC}/bot_movement.c"
	"${DIR_SRC}/bot_routing.c"
	"${DIR_SRC}/bot_world.c"
	"${DIR_SRC}/buttons.c"
	"${DIR_SRC}/captain.c"
	"${DIR_SRC}/coach.c"
	"${DIR_SRC}/clan_arena.c"
	"${DIR_SRC}/client.c"
	"${DIR_SRC}/combat.c"
	"${DIR_SRC}/commands.c"
	"${DIR_SRC}/ctf.c"
	"${DIR_SRC}/doors.c"
	"${DIR_SRC}/fb_globals.c"
	"${DIR_SRC}/files.c"
	"${DIR_SRC}/func_bob.c"
	"${DIR_SRC}/func_laser.c"
	"${DIR_SRC}/g_cmd.c"
	"${DIR_SRC}/globals.c"
	"${DIR_SRC}/g_mem.c"
	"${DIR_SRC}/grapple.c"
	"${DIR_SRC}/g_spawn.c"
	"${DIR_SRC}/g_userinfo.c"
	"${DIR_SRC}/g_utils.c"
	"${DIR_SRC}/g_syscalls_extra.c"
	"${DIR_SRC}/hiprot.c"
	"${DIR_SRC}/hoonymode.c"
	"${DIR_SRC}/items.c"
	"${DIR_SRC}/logs.c"
	"${DIR_SRC}/maps.c"
	"${DIR_SRC}/maps_map_amphi2.c"
	"${DIR_SRC}/maps_map_dm3.c"
	"${DIR_SRC}/maps_map_dm4.c"
	"${DIR_SRC}/maps_map_dm6.c"
	"${DIR_SRC}/maps_map_povdmm4.c"
	"${DIR_SRC}/marker_load.c"
	"${DIR_SRC}/marker_util.c"
	"${DIR_SRC}/match.c"
	"${DIR_SRC}/mathlib.c"
	"${DIR_SRC}/misc.c"
	"${DIR_SRC}/motd.c"
	"${DIR_SRC}/plats.c"
	"${DIR_SRC}/player.c"
	"${DIR_SRC}/q_shared.c"
	"${DIR_SRC}/race.c"
	"${DIR_SRC}/rng.c"
	"${DIR_SRC}/rng_gen_impl.c"
	"${DIR_SRC}/rng_seed_impl.c"
	"${DIR_SRC}/route_calc.c"
	"${DIR_SRC}/route_fields.c"
	"${DIR_SRC}/route_lookup.c"
	"${DIR_SRC}/runes.c"
	"${DIR_SRC}/server.c"
	"${DIR_SRC}/sp_ai.c"
	"${DIR_SRC}/sp_boss.c"
	"${DIR_SRC}/sp_client.c"
	"${DIR_SRC}/sp_demon.c"
	"${DIR_SRC}/sp_dog.c"
	"${DIR_SRC}/spectate.c"
	"${DIR_SRC}/sp_enforcer.c"
	"${DIR_SRC}/sp_fish.c"
	"${DIR_SRC}/sp_hknight.c"
	"${DIR_SRC}/sp_knight.c"
	"${DIR_SRC}/sp_monsters.c"
	"${DIR_SRC}/sp_ogre.c"
	"${DIR_SRC}/sp_oldone.c"
	"${DIR_SRC}/sp_shalrath.c"
	"${DIR_SRC}/sp_shambler.c"
	"${DIR_SRC}/sp_soldier.c"
	"${DIR_SRC}/sp_tarbaby.c"
	"${DIR_SRC}/sp_wizard.c"
	"${DIR_SRC}/sp_zombie.c"
	"${DIR_SRC}/stats.c"
	"${DIR_SRC}/statsTables.c"
	"${DIR_SRC}/stats_json.c"
	"${DIR_SRC}/stats_xml.c"
	"${DIR_SRC}/subs.c"
	"${DIR_SRC}/teamplay.c"
	"${DIR_SRC}/triggers.c"
	"${DIR_SRC}/vip.c"
	"${DIR_SRC}/vote.c"
	"${DIR_SRC}/weapons.c"
	"${DIR_SRC}/world.c"
	)

# Build native library.
add_library(${PROJECT_NAME} SHARED ${SRC_COMMON} ${SRC_NATIVE})
target_include_directories(${PROJECT_NAME} PRIVATE "include")
set_target_properties(${PROJECT_NAME}
	PROPERTIES PREFIX "" # Strip lib prefix.
	C_VISIBILITY_PRESET hidden # Hide all symbols unless excplicitly marked to export.
	)
if(BOT_SUPPORT)
	target_compile_definitions(${PROJECT_NAME} PRIVATE BOT_SUPPORT=1)
endif()
if(FTESV)
	target_compile_definitions(${PROJECT_NAME} PRIVATE FTESV=1)
endif()
# If not Miscrosoft compilator - then set additional options for linker.
if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
	# 4100 - unused parameter
	# 4210 - using extern keyword in functions
	# 4456 - shadowing locals, broadly used
	# 4459 - shadowing globals, broadly used
	target_compile_options(${PROJECT_NAME} PRIVATE /W4 /wd4100 /wd4210 /wd4456 /wd4459)
else()
	target_compile_options(${PROJECT_NAME} PRIVATE -Wall -std=gnu17)

	check_c_compiler_flag("-Wstrict-prototypes" HAS_CFLAG_STRICT_PROTOTYPES)
	if (HAS_CFLAG_STRICT_PROTOTYPES)
		target_compile_options(${PROJECT_NAME} PRIVATE -Werror=strict-prototypes)
	endif()

	check_c_compiler_flag("-Wstrlcpy-strlcat-size" HAS_STRLCPY_STRLCAT_SIZE)
	if (HAS_STRLCPY_STRLCAT_SIZE)
		target_compile_options(${PROJECT_NAME} PRIVATE -Werror=strlcpy-strlcat-size)
	endif()

	# Do not allow undefined symbols while linking.
	if(APPLE)
		target_link_options(${PROJECT_NAME} PRIVATE "-Wl,-undefined,error")
	else()
		target_link_options(${PROJECT_NAME} PRIVATE "-Wl,--no-undefined")
	endif()
	target_link_libraries(${PROJECT_NAME} PRIVATE m) # Link with libm.
endif()
