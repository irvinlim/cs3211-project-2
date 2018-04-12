#!/bin/sh
rsync -r -a --exclude=".*/" -e ssh --delete . nscc:~/cs3211-project-2
