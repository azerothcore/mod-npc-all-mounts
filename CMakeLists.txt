CU_SET_PATH("CMAKE_MOD_NPCALLMOUNTS_DIR" "${CMAKE_CURRENT_LIST_DIR}")

AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/npc_allmounts.cpp")

AC_ADD_SCRIPT_LOADER("AllMountsNPC" "${CMAKE_CURRENT_LIST_DIR}/src/loader_allmounts.h")

AC_ADD_CONFIG_FILE("${CMAKE_CURRENT_LIST_DIR}/conf/npc_allmounts.conf.dist")
