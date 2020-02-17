#!/bin/bash
cd .git/hooks
ln -sf ../../bin/githook-astyle.sh ./pre-commit
ln -sf ../../bin/gtags.sh ./gtags
ln -sf ../../bin/post-gtags.sh ./post-commit
ln -sf ../../bin/post-gtags.sh ./post-merge
ln -sf ../../bin/post-gtags.sh ./post-checkout
ln -sf ../../bin/post-rewrite.sh ./post-rewrite
cd -

# git config --global alias.gtags '!.git/hooks/gtags'
