#!/bin/bash

echo "removing old backup files..."
find . -name "*~" -exec rm -i {} \;
