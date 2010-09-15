#!/bin/sh

rsync -vuza --delete --exclude-from .gitignore . stats:dev/cse440p1

