#!/usr/bin/env bash
export Z64ROOT=`pwd`
echo $Z64ROOT
export EDA=/opt/eda
export PATH=$PATH:$EDA/bin
export PATH=$PATH:$Z64ROOT/scripts
COL1="\[$(tput setaf 6)\]"
COL2="\[$(tput setaf 7)\]"
COL3="\[$(tput setaf 8)\]"
RESET="\[$(tput sgr0)\]"
export PS1="${COL1}(zed64)${COL2}\u${COL3}\w >${RESET}"
bash

