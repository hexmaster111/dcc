#!/bin/bash
tail -f log.txt 2>&1 | perl -ne 'if (/file truncated/) {system 'clear'; print} else {print}'