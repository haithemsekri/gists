# Huge repo cloning
git clone --depth 1 https://github.com/hardkernel/linux.git linux.git
cd linux.git
git fetch --unshallow
git fetch --tags
git remote set-branches origin '*'
git fetch -v
git branch -r | grep -v '\->' | while read remote; do git branch "${remote#origin/}" "$remote"; done
git remote update
git fetch --all
git pull --all
