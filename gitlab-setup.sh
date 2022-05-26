cd /lib/systemd/system
systemctl disable --now apt-daily*
systemctl disable --now unattended-upgrades.service
apt purge snapd
apt install -y iproute2 python3-pip openssh-server ca-certificates curl openssh-server tzdata git-core lz4 rsync wget curl jq --no-install-recommends
nano /etc/ssh/sshd_config

wget --content-disposition https://packages.gitlab.com/gitlab/gitlab-ce/packages/debian/bullseye/gitlab-ce_14.10.2-ce.0_amd64.deb/download.deb
dpkg -i gitlab-ce_14.10.2-ce.0_amd64.deb


cd /etc/ && tar -cf - gitlab | lz4 > gitlab.origin.tar.lz4
cd /opt/ && tar -cf - gitlab | lz4 > gitlab.origin.tar.lz4

echo gitlabserver > /etc/hostname
cat /etc/hostname
echo "127.0.0.1 gitlab.hsekri.com" >> /etc/hosts
cat /etc/hosts

cat /etc/gitlab/gitlab.rb | grep external_url
external_url 'http://gitlab.hsekri.com'
gitlab-ctl reconfigure
cat /etc/gitlab/initial_root_password
Password: k9hWa/UVeaHzX5G8x4//wGmvHuLBoe2gV56b/iLHxbU=
# http://gitlab.hsekri.com
gitlab-rake "gitlab:password:reset"
USR:haithem.sekri PASS:haithem.sekri.passwd TOKEN: vje-5-G6A8HT2F1hypLY

reboot
gitlab-ctl status
gitlab-ctl stop
cd /etc/ && tar -cf - gitlab | lz4 > gitlab.configured.tar.lz4
cd /opt/ && tar -cf - gitlab | lz4 > gitlab.configured.tar.lz4
cat /etc/passwd | grep git:
git:x:997:997::/var/opt/gitlab:/bin/sh

mkdir /home/git
chown -R git:git /home/git/.
usermod -s /bin/bash -d /home/git git
cat /etc/passwd | grep git:

gitlab-ctl start
gitlab-ctl status

cat ~/.gitlab.hsekri.com.cred

GIT_TOK="vje-5-G6A8HT2F1hypLY"
GIT_USR="haithem.sekri"
GIT_DNS="gitlab.hsekri.com"
GIT_URL="http://$GIT_USR:$GIT_TOK@$GIT_DNS"

mkdir ~/repos
#-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
#!/bin/bash

#https://docs.gitlab.com/ee/api/projects.html#edit-project
#https://docs.gitlab.com/ee/api/groups.html#edit-group-push-rule

main_cmd="$1"
shift
main_args="$@"
shift 1000

source ~/.gitlab.hsekri.com.cred

# cat ~/.gitlab.hsekri.com.token
# GIT_TOK="xxxx"
# GIT_USR="xxxxx"
# GIT_DNS="git.hsekri.com"
# GIT_URL="http://$GIT_USR:$GIT_TOK@$GIT_DNS"

_CURL() {
  local type=$1
  local sub_url=$2
  shift 2

  local extra_args=""
  if [[ -z "$1" ]]; then
    extra_args=""
  elif [[ -f "$1"  ]]; then
    extra_args="-d @$@"
  else
    extra_args="--data $@"
  fi
  rm -rf /tmp/_CURL.stdout /tmp/_CURL.stderr
  curl --output /tmp/_CURL.stdout --insecure --request $type --header "PRIVATE-TOKEN: $GIT_TOK" --header "Content-Type: application/json" --url $GIT_URL/api/v4/$sub_url $extra_args 2>/dev/null 1>/tmp/_CURL.stderr
}

_POST() {
  _CURL POST $@
}

_GET() {
  _CURL GET $@
}

_PUT() {
  _CURL PUT $@
}

_DELETE() {
  _CURL DELETE $@
}


FIX_JSON() {
        echo $@  | \
          grep -v ": {}" | tr  -d ' ' | tr -d '\n' | sed 's/,}/}/g' | jq '.'  | \
          grep -v ": {}" | tr  -d ' ' | tr -d '\n' | sed 's/,}/}/g' | jq '.' | \
          grep -v ": {}" | tr  -d ' ' | tr -d '\n' | sed 's/,}/}/g' | jq '.' | \
          grep -v ": {}" | tr  -d ' ' | tr -d '\n' | sed 's/,}/}/g'
}

# -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


_NEW_GROUP() {
   [[ -z "$1" ]] && echo "FAILED_NEW_GROUP: invalid argument" && return 0
  local _GROUP="$1"

  echo "{\"name\":\"$_GROUP\",\"path\":\"$_GROUP\",\"description\":\"$_GROUP Group\",\"visibility\":\"public\",\"parent_id\":null}" > /tmp/_CURL.stdin
  _POST groups /tmp/_CURL.stdin

  _GET groups
  [[ ! -f /tmp/_CURL.stdout ]] && echo "_CURL.stdout Not Found" && return 0
  _GROUP_ID="$(cat /tmp/_CURL.stdout | jq '.' | grep  -B 4 "\"path\": \"$_GROUP\""  | grep "\"id\":" | cut -d ':' -f 2 | cut -d ',' -f 1 | xargs)"
  [[ ! -z "$_GROUP_ID" && "$_GROUP_ID" != "0" && $_GROUP_ID =~ ^-?[0-9]+$ ]] && echo "$_GROUP_ID" || echo "FAILED_NEW_GROUP: cannot create group $_GROUP _GROUP_ID($_GROUP_ID)"
}


_NEW_PROJECT() {
  [[ -z "$4" ]] && echo "FAILED_NEW_PROJECTP: invalid argument" && return 0
  local _GROUP="$1"
  local _PROJECT="$2"
  local _MAIN_BRANCH="$3"
  local _URL="$4"

  local _GROUP_ID="$(_NEW_GROUP $_GROUP)"
  ! [[ ! -z "$_GROUP_ID" && "$_GROUP_ID" != "0" && $_GROUP_ID =~ ^-?[0-9]+$ ]] &&  echo "FAILED_NEW_PROJECT: cannot create group $_GROUP _GROUP_ID($_GROUP_ID)"

  echo "{\"name\":\"$_PROJECT\",\"path\":\"$_PROJECT\",\"description\":\"OriginSourceUrl:$_URL\",\"visibility\":\"public\",\"namespace_id\":\"$_GROUP_ID\",\"initialize_with_readme\":\"false\",\"default_branch\":\"$_MAIN_BRANCH\"}"  > /tmp/_CURL.stdin
  _POST projects /tmp/_CURL.stdin

  _GET projects
  [[ ! -f /tmp/_CURL.stdout ]] && echo "_CURL.stdout Not Found" && return 0
  local _PROJECT_ID="$(cat /tmp/_CURL.stdout | jq '.' | grep  -B 12  "\"http_url_to_repo\": \"http://$GIT_DNS/$_GROUP/$_PROJECT.git\""   | grep "\"id\":" | cut -d ':' -f 2 | cut -d ',' -f 1 | xargs)"
  [[ ! -z "$_PROJECT_ID" && "$_PROJECT_ID" != "0" && $_PROJECT_ID =~ ^-?[0-9]+$ ]] && echo "$_PROJECT_ID" || echo "FAILED_NEW_PROJECT: cannot create project $_PROJECT _PROJECT_ID($_PROJECT_ID)"
}


_GET_GROUPS() {
  _GET groups
  [[ ! -f /tmp/_CURL.stdout ]] && echo "_CURL.stdout Not Found" && return 0

  local _GROUPS="$(cat /tmp/_CURL.stdout  | jq '.' | grep '"id"\|"path"\|{\|}')"
  local JSON="$(FIX_JSON "{\"GROUPS\": [$_GROUPS]}")"
  [[ ! -z "$1" ]] && echo $JSON | jq ".GROUPS | .[$1]"
  [[ -z "$1" ]] && echo $JSON | jq '.'
}

_GET_PROJECTS() {
  _GET projects
  [[ ! -f /tmp/_CURL.stdout ]] && echo "_CURL.stdout Not Found" && return 0
  local _PROJECTS="$(cat /tmp/_CURL.stdout | jq '.' | grep '"id":\|"name":\|"http_url_to_repo":\|"description":\|"namespace":\|{\|}' )"
  local JSON="$(FIX_JSON "{\"PROJECTS\": [$_PROJECTS]}")"
  [[ ! -z "$1" ]] && echo $JSON | jq ".PROJECTS | .[$1]"
  [[ -z "$1" ]] && echo $JSON | jq '.'
}

_FIND_GROUPS() {
  [[ -z "$1" ]] && echo "FAILED_FIND_GROUPS: invalid argument" && return 0
  local _GROUP="$1"
  echo "$(_GET_GROUPS)" |  jq ".GROUPS[] | select(.path == \"$_GROUP\")"
}


_FIND_PROJECT() {
  [[ -z "$2" ]] && echo "FAILED_FIND_PROJECT: invalid argument" && return 0
  local _GROUP="$1"
  local _PROJECT="$2"
  echo "$(_GET_PROJECTS)" |  jq ".PROJECTS[] | select(.http_url_to_repo == \"http://$GIT_DNS/$_GROUP/$_PROJECT.git\")"
}


_ADD_MAIN_BRANCH() {
  [[ ! -d ".git" ]] && echo "Not a git repo" && return 0

  local GROUP="$1"
  local PROJECT="$2"
  local SRC_URL="$3"
  local GIT_DIR="$GROUP.$PROJECT"

  local MAIN_BRANCH="$(git branch | sed 's/*//g' | xargs)"
  [[ ! -z "$MAIN_BRANCH" ]] && echo "Already set to $MAIN_BRANCH" && return 0

  MAIN_BRANCH="master"
  git checkout $MAIN_BRANCH
  MAIN_BRANCH="$(git branch | sed 's/*//g' | xargs)"
  [[ ! -z "$MAIN_BRANCH" ]] && echo "Already set to master" && return 0

  MAIN_BRANCH="main"
  git checkout $MAIN_BRANCH
  MAIN_BRANCH="$(git branch | sed 's/*//g' | xargs)"
  [[ ! -z "$MAIN_BRANCH" ]] && echo "Already set to main" && return 0

  MAIN_BRANCH="main"
  git checkout -b $MAIN_BRANCH

  rm -rf README.project
  echo "Project:$PROJECT" >> README.project
  echo "OriginSourceUrl:$SRC_URL" >> README.project

  git add README.project
  git config user.name $GIT_USR
  git config user.email $GIT_EMAIL
  git commit -m "Add main branch"
  git push --set-upstream origin $MAIN_BRANCH
  git push origin $MAIN_BRANCH
}

_ADD_MIGRATION_TAGS() {
  local _COMMITS_NR="$(git log   --pretty=oneline  | cut -d ' ' -f1  | wc -l)"
  echo $_COMMITS_NR

  local i=4
  while [ $i -ge 0 ]; do
    _COMMIT="$(git log --skip=$(($_COMMITS_NR * $i / 6)) --max-count=1 --pretty=oneline | cut -d ' ' -f1 | xargs)"
    echo $i:$_COMMIT
    git tag xxyyzz-$_COMMIT $_COMMIT
    git push origin xxyyzz-$_COMMIT
    let "i-=2"
  done

  git tag -l | cut -c1-2 | sort | uniq | while read l; do git push origin tag "$l*"; done
}

_REMOVE_MIGRATION_TAGS() {
  git for-each-ref --sort=committerdate --format='%(refname)' | grep "xxyyzz" | while read l; do git push --delete origin $(basename /$l); done
}

_MIGRATE_PROJECT() {
  [[ -z "$3" ]] && echo "Require 3 arguments: GROUP REPO SRC_URL" && exit 0

  local GROUP="$1"
  local PROJECT="$2"
  local GIT_DIR="$GROUP.$PROJECT"
  local SRC_URL="$3"

  echo $PROJECT
  echo $GROUP

  rm -rf $GIT_DIR

  [[ -f $GIT_DIR.git.unshallow.tar.lz4 ]] && lz4 -dc --no-sparse $GIT_DIR.git.unshallow.tar.lz4 | tar xf -
  [[ ! -d $GIT_DIR ]] && git clone --depth 1 --no-single-branch $SRC_URL $GIT_DIR && tar -cf - $GIT_DIR | lz4 > $GIT_DIR.git.tar.lz4

  cd  $GIT_DIR
  [[ ! -d .git ]] && echo "Not a git directory" && return 1
  git config --get remote.origin.url
  git config pull.rebase true
  git pull
  git pull --unshallow

  cd ..
  [[ ! -f $GIT_DIR.git.unshallow.tar.lz4 ]] && tar -cf - $GIT_DIR | lz4 > $GIT_DIR.git.unshallow.tar.lz4 && rm -rf $GIT_DIR.git.tar.lz4
  cd $GIT_DIR

  echo "OriginSourceUrl:$SRC_URL" > .git/description
  cat .git/description

  git remote remove origin
  git remote add origin $GIT_URL/$GROUP/$PROJECT.git

  _ADD_MAIN_BRANCH $GROUP $PROJECT $SRC_URL

  REPO_SIZE="$(du -s . | cut -d '.' -f1)"
  [[ $REPO_SIZE -gt 1048576 ]] && _ADD_MIGRATION_TAGS

  git push origin --tags
  git push -u origin --all
  git push origin --mirror &>/dev/null
  git push origin '*:*' &>/dev/null

  [[ $REPO_SIZE -gt 1048576 ]] && _REMOVE_MIGRATION_TAGS

  cd ..
  rm -rf $GIT_DIR
  _FIND_PROJECT $GROUP $PROJECT
}


-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
_FIND_GROUPS STM32
_FIND_PROJECT STM32 STM32CubeF1


_NEW_GROUP group1
6
_GET groups/6
cat /tmp/_CURL.stdout | jq '.'
_DELETE groups/6

 _NEW_PROJECT STM32 STM32CubeF1 https://github.com/STMicroelectronics/STM32CubeF1.git
_GET projects/2
cat /tmp/_CURL.stdout | jq '.'

_GET projects

_DELETE projects/2
cat /tmp/_CURL.stdout | jq '.'





_NEW_GROUP LINUX
_NEW_PROJECT LINUX qemu master "git://git.qemu.org/qemu.git"
_GET_PROJECTS
_MIGRATE_PROJECT LINUX qemu "git://git.qemu.org/qemu.git"



_NEW_GROUP CODEAURORA
_MIGRATE_PROJECT CODEAURORA linux-imx "https://source.codeaurora.org/external/imx/linux-imx"


_NEW_GROUP CODEAURORA
_FIND_PROJECT CODEAURORA imx-xen
_DELETE projects/29
_FIND_PROJECT CODEAURORA imx-xen
_NEW_PROJECT CODEAURORA imx-xen main "https://source.codeaurora.org/external/imx/imx-xen"
_FIND_PROJECT CODEAURORA imx-xen
_MIGRATE_PROJECT CODEAURORA imx-xen "https://source.codeaurora.org/external/imx/imx-xen"


_NEW_GROUP LINUX
_FIND_PROJECT LINUX qemu
_DELETE projects/33
_NEW_PROJECT LINUX qemu main "git://git.qemu.org/qemu.git"
_FIND_PROJECT LINUX qemu
_MIGRATE_PROJECT LINUX qemu "git://git.qemu.org/qemu.git"

_NEW_GROUP HARDKERNEL
_FIND_PROJECT HARDKERNEL linux
_DELETE projects/55
_NEW_PROJECT HARDKERNEL linux main "https://github.com/hardkernel/linux.git"
time _MIGRATE_PROJECT HARDKERNEL linux "https://github.com/hardkernel/linux.git"


_NEW_GROUP CODEAURORA
_FIND_PROJECT CODEAURORA linux-imx
_DELETE projects/40
_NEW_PROJECT CODEAURORA linux-imx main "https://source.codeaurora.org/external/imx/linux-imx"
_MIGRATE_PROJECT CODEAURORA linux-imx "https://source.codeaurora.org/external/imx/linux-imx"

_NEW_GROUP STM32
_FIND_PROJECT STM32 STM32CubeF1
_DELETE projects/44
_NEW_PROJECT STM32 STM32CubeF1 main https://github.com/STMicroelectronics/STM32CubeF1.git
_MIGRATE_PROJECT STM32 STM32CubeF1 https://github.com/STMicroelectronics/STM32CubeF1.git


_NEW_GROUP OrangePi
_FIND_PROJECT OrangePi linux-megous
_DELETE projects/57
_NEW_PROJECT OrangePi linux-megous main "https://github.com/megous/linux.git"
time _MIGRATE_PROJECT OrangePi linux-megous "https://github.com/megous/linux.git"

_NEW_GROUP Linux
_FIND_PROJECT Linux xenbits-xen
_DELETE projects/57
_NEW_PROJECT Linux xenbits-xen main "https://xenbits.xen.org/git-http/xen.git"
time _MIGRATE_PROJECT Linux xenbits-xen "https://xenbits.xen.org/git-http/xen.git"





git reset --hard HEAD~5
git push --set-upstream origin master
git reset --hard HEAD@{1}
git push origin --tags
git push -u origin --all

_COMMITS_NR="$(git log   --pretty=oneline  | cut -d ' ' -f1  | wc -l)"
echo $_COMMITS_NR

i=4
while [ $i -ge 0 ]; do
  _COMMIT="$(git log --skip=$(($_COMMITS_NR * $i / 6)) --max-count=1 --pretty=oneline | cut -d ' ' -f1 | xargs)"
  echo $i:$_COMMIT
  git tag xxyyzz-$_COMMIT $_COMMIT
  git push origin xxyyzz-$_COMMIT
  let "i-=2"
done


git tag -l | cut -c1-2 | sort | uniq | while read l; do git push origin tag "$l*"; done
git push origin --tags
git push -u origin --all
git push --set-upstream origin $(git branch | sed 's/*//g' | xargs)
git push origin --mirror
git push origin '*:*'


git checkout 88b9e456b1649722673ffa147914299799dc9041
git reset --hard 88b9e456b1649722673ffa147914299799dc9041
git checkout -b xxyyzz-88b9e456b1649722673ffa147914299799dc9041
git push --set-upstream origin xxyyzz-88b9e456b1649722673ffa147914299799dc9041
#git reset --hard HEAD@{1}

LAST_COMMITS="$(git rev-parse HEAD)"
echo LAST_COMMITS:$LAST_COMMITS
git tag xxyyzz-$LAST_COMMITS $LAST_COMMITS
git push origin xxyyzz-$LAST_COMMITS

git push --set-upstream origin $(git branch | sed 's/*//g' | xargs)
git push origin --tags
git push -u origin --all

_MIGRATE() {
  echo "#------------------------------------------------------------#"
  [[ -z "$4" ]] && echo "Expected 4 arguments: $3" && return 1
  local   local _GROUP="$1"
  local _PROJECT="$2"
  local _BRANCH="$3"
  local _URL="$4"

  echo "<$_GROUP> <$_PROJECT> <$_BRANCH> <$_URL>"
  _NEW_GROUP $_GROUP
  _NEW_PROJECT $_GROUP $_PROJECT $_BRANCH $_URL
  _MIGRATE_PROJECT $_GROUP $_PROJECT $_URL
  cd /home/git/repos
}
cat git-repo-lists.txt | while read l; do cd /home/git/repos; _MIGRATE $l; done
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------

GROUP="CODEAURORA"
PROJECT="imx-xen"
SRC_URL="https://source.codeaurora.org/external/imx/imx-xen"
git clone $SRC_URL $PROJECT.git
cp -r $PROJECT.git  $PROJECT.git.origin

MAIN_BRANCH="$(basename $(git for-each-ref --sort=committerdate --format='%(refname)' | grep tags | tail -1))"
echo $MAIN_BRANCH

echo _NEW_PROJECT $GROUP $PROJECT $MAIN_BRANCH $SRC_URL
_NEW_PROJECT $GROUP $PROJECT $MAIN_BRANCH $SRC_URL
_FIND_PROJECT $GROUP $PROJECT

echo "OriginSourceUrl:$SRC_URL" > .git/description
cat .git/description

git remote remove origin
git remote add origin $GIT_URL/$GROUP/$PROJECT.git

git remote -vv

git push origin $MAIN_BRANCH
git push origin --tags
git push -u origin --all

-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
