#!/usr/bin/env bash

MOD_NPCALLMOUNTS_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/" && pwd )"

source "$MOD_NPCALLMOUNTS_ROOT/conf/conf.sh.dist"

if [ -f "$MOD_NPCALLMOUNTS_ROOT/conf/conf.sh" ]; then
    source "$MOD_NPCALLMOUNTS_ROOT/conf/conf.sh"
fi 