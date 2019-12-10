#!/usr/bin/env bash
eval "gcc -fPIC -shared -o chain_index/chain_index_lib.so chain_index/chain_index_lib.c"
eval "gcc -o chain_index/chainExtract chain_index/chainExtract.c"