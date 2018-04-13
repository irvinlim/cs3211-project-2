#!/bin/sh

echo "Syncing artifacts from remote server..."
rsync -ravz --include="logs/***" --include="output/***" --include="report/***" --exclude="*" -e ssh nscc:~/cs3211-project-2/ .
echo
echo "Syncing source code to remote server..."
rsync -ravz --exclude=".*/" --exclude=".*" --exclude="logs/*" --exclude="output/*" --exclude="report/*" --exclude="animator/*" -e ssh . nscc:~/cs3211-project-2/
echo
echo "Done!"
