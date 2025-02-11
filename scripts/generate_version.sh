#!/bin/bash

# Get current date and time
YEAR=$(date +%Y)
MONTH=$(date +%-m)
DAY=$(date +%-d)
HOUR=$(date +%-H)
MINUTE=$(date +%-M)

# Calculate revision (hours * 60 + minutes)
REVISION=$((HOUR * 60 + MINUTE))

# Create Version.h
cat > "$1" << EOF
#pragma once
#define V_MAJOR ${YEAR}
#define V_MINOR ${MONTH}
#define V_BUILD ${DAY}
#define V_REVISION ${REVISION}
EOF