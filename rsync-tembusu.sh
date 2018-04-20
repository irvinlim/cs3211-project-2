#!/bin/sh

echo "Syncing artifacts from remote server..."
rsync -ravz --include="data/***" --exclude="*" -e ssh tembusu:~/cs3211-project-2/ .
echo
echo "Syncing source code to remote server..."
rsync -ravz --exclude=".*/" --exclude=".*" --exclude="data/***" --exclude="animator/***" --exclude="docs/***" -e ssh . tembusu:~/cs3211-project-2/
echo
echo "Done!"
