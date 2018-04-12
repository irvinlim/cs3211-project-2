#!/bin/sh

rsync -ravz --exclude=".*/" --exclude=".*" -e ssh nscc:~/cs3211-project-2/logs/* logs/
rsync -ravz -e ssh nscc:~/cs3211-project-2/*.ppm .
rsync -ravz --exclude=".*/" --exclude=".*" --exclude="logs/*" -e ssh . nscc:~/cs3211-project-2
