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
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
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
  type=$1
  sub_url=$2
  shift 2

  extra_args=""
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

# _GET projects
# cat /tmp/_CURL.stdout | jq '.'
# _GET groups
# cat /tmp/_CURL.stdout | jq '.'

# -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


_NEW_GROUP() {
   [[ -z "$1" ]] && echo "FAILED_NEW_GROUP: invalid argument" && return 0
  _GROUP="$1"

  echo "{\"name\":\"$_GROUP\",\"path\":\"$_GROUP\",\"description\":\"$_GROUP Group\",\"visibility\":\"public\",\"parent_id\":null}" > /tmp/_CURL.stdin
  _POST groups /tmp/_CURL.stdin

  _GET groups
  [[ ! -f /tmp/_CURL.stdout ]] && echo "_CURL.stdout Not Found" && return 0
  _GROUP_ID="$(cat /tmp/_CURL.stdout | jq '.' | grep  -B 4 "\"path\": \"$_GROUP\""  | grep "\"id\":" | cut -d ':' -f 2 | cut -d ',' -f 1 | xargs)"
  [[ ! -z "$_GROUP_ID" && "$_GROUP_ID" != "0" && $_GROUP_ID =~ ^-?[0-9]+$ ]] && echo "$_GROUP_ID" || echo "FAILED_NEW_GROUP: cannot create group $_GROUP _GROUP_ID($_GROUP_ID)"
}


_NEW_PROJECT() {
   [[ -z "$4" ]] && echo "FAILED_NEW_PROJECTP: invalid argument" && return 0
  _GROUP="$1"
  _PROJECT="$2"
  _MAIN_BRANCH="$3"
  _URL="$4"

  _GROUP_ID="$(_NEW_GROUP $_GROUP)"
  ! [[ ! -z "$_GROUP_ID" && "$_GROUP_ID" != "0" && $_GROUP_ID =~ ^-?[0-9]+$ ]] &&  echo "FAILED_NEW_PROJECT: cannot create group $_GROUP _GROUP_ID($_GROUP_ID)"

  echo "{\"name\":\"$_PROJECT\",\"path\":\"$_PROJECT\",\"description\":\"OriginSourceUrl:$_URL\",\"visibility\":\"public\",\"namespace_id\":\"$_GROUP_ID\",\"initialize_with_readme\":\"false\",\"default_branch\":\"$_MAIN_BRANCH\"}"  > /tmp/_CURL.stdin
  _POST projects /tmp/_CURL.stdin

  _GET projects
  [[ ! -f /tmp/_CURL.stdout ]] && echo "_CURL.stdout Not Found" && return 0
  _PROJECT_ID="$(cat /tmp/_CURL.stdout | jq '.' | grep  -B 12  "\"http_url_to_repo\": \"http://$GIT_DNS/$_GROUP/$_PROJECT.git\""   | grep "\"id\":" | cut -d ':' -f 2 | cut -d ',' -f 1 | xargs)"
  [[ ! -z "$_PROJECT_ID" && "$_PROJECT_ID" != "0" && $_PROJECT_ID =~ ^-?[0-9]+$ ]] && echo "$_PROJECT_ID" || echo "FAILED_NEW_PROJECT: cannot create project $_PROJECT _PROJECT_ID($_PROJECT_ID)"
}


_GET_GROUPS() {
  _GET groups
  [[ ! -f /tmp/_CURL.stdout ]] && echo "_CURL.stdout Not Found" && return 0

  _GROUPS="$(cat /tmp/_CURL.stdout  | jq '.' | grep '"id"\|"path"\|{\|}')"
  JSON="$(FIX_JSON "{\"GROUPS\": [$_GROUPS]}")"
  [[ ! -z "$1" ]] && echo $JSON | jq ".GROUPS | .[$1]"
  [[ -z "$1" ]] && echo $JSON | jq '.'
}

_GET_PROJECTS() {
  _GET projects
  [[ ! -f /tmp/_CURL.stdout ]] && echo "_CURL.stdout Not Found" && return 0
  _PROJECTS="$(cat /tmp/_CURL.stdout | jq '.' | grep '"id":\|"name":\|"http_url_to_repo":\|"description":\|"namespace":\|{\|}' )"
  JSON="$(FIX_JSON "{\"PROJECTS\": [$_PROJECTS]}")"
  [[ ! -z "$1" ]] && echo $JSON | jq ".PROJECTS | .[$1]"
  [[ -z "$1" ]] && echo $JSON | jq '.'
}

_FIND_GROUPS() {
  [[ -z "$1" ]] && echo "FAILED_FIND_GROUPS: invalid argument" && return 0
  _GROUP="$1"
  echo "$(_GET_GROUPS)" |  jq ".GROUPS[] | select(.path == \"$_GROUP\")"
}


_FIND_PROJECT() {
  [[ -z "$2" ]] && echo "FAILED_FIND_PROJECT: invalid argument" && return 0
  _GROUP="$1"
  _PROJECT="$2"
  echo "$(_GET_PROJECTS)" |  jq ".PROJECTS[] | select(.http_url_to_repo == \"http://$GIT_DNS/$_GROUP/$_PROJECT.git\")"
}


_ADD_MAIN_BRANCH() {
  [[ ! -d ".git" ]] && echo "Not a git repo" && return 0

  local GROUP="$1"
  local PROJECT="$2"
  local SRC_URL="$3"
  local GIT_DIR="$GROUP.$PROJECT"

  MAIN_BRANCH="$(git branch | sed 's/*//g' | xargs)"
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
  i=1000000
  while [ $i -ge 0 ]
  do
    _COMMIT_ID="$(git log --skip=$i --max-count=1 --pretty=oneline | cut -d ' ' -f1 | xargs)"
    if [[ ! -z "$_COMMIT_ID" ]]; then
      echo _COMMIT_ID:$_COMMIT_ID
      git tag xxyyzz-$_COMMIT_ID $_COMMIT_ID
      git push origin xxyyzz-$_COMMIT_ID
    fi
    let "i-=100000"
  done
}


_REMOVE_MIGRATION_TAGS() {
  git for-each-ref --sort=committerdate --format='%(refname)' | grep "xxyyzz" | while read l; do git push --delete origin $(basename /$l); done
}

_MIGRATE_PROJECT() {
  [[ -z "$3" ]] && echo "Require 3 arguments: GROUP REPO SRC_URL" && exit 0

  GROUP="$1"
  PROJECT="$2"
  GIT_DIR="$GROUP.$PROJECT"
  SRC_URL="$3"

  echo $PROJECT
  echo $GROUP

  rm -rf $GIT_DIR

  [[ -f $GIT_DIR.git.unshallow.tar.lz4 ]] && lz4 -dc --no-sparse $GIT_DIR.git.unshallow.tar.lz4 | tar xf -
  [[ ! -d $GIT_DIR ]] && git clone --depth 1 --no-single-branch $SRC_URL $GIT_DIR
  [[ ! -f $GIT_DIR.git.tar.lz4 ]] && tar -cf - $GIT_DIR | lz4 > $GIT_DIR.git.tar.lz4

  cd  $GIT_DIR
  git config --get remote.origin.url
  git config pull.rebase true
  git pull --unshallow &>/dev/null

  cd ..
  [[ ! -f $GIT_DIR.git.unshallow.tar.lz4 ]] && tar -cf - $GIT_DIR | lz4 > $GIT_DIR.git.unshallow.tar.lz4
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


_NEW_GROUP STM32
_NEW_PROJECT STM32 STM32CubeF1 https://github.com/STMicroelectronics/STM32CubeF1.git
_MIGRATE_PROJECT STM32 STM32CubeF1 https://github.com/STMicroelectronics/STM32CubeF1.git



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
_DELETE projects/36
_NEW_PROJECT HARDKERNEL linux main "https://github.com/hardkernel/linux.git"
_MIGRATE_PROJECT HARDKERNEL linux "https://github.com/hardkernel/linux.git"


_NEW_GROUP CODEAURORA
_FIND_PROJECT CODEAURORA linux-imx
_DELETE projects/17
_NEW_PROJECT CODEAURORA linux-imx main "https://source.codeaurora.org/external/imx/linux-imx"
_MIGRATE_PROJECT CODEAURORA linux-imx "https://source.codeaurora.org/external/imx/linux-imx"



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

groupadd -g 1001 hs
useradd -s /bin/bash -r -p b  -u 1001 -g 1001 -m -d /home/hs hs

  109  usermod -s /bin/sh -d /var/opt/gitlab git
  110  kill -9 860
  111  usermod -s /bin/sh -d /var/opt/gitlab git
  112  cd /opt/
  113  unlink gitlab
  114  cp -r gitlab.old gitlab
  115  kill -9 885
  116  usermod -s /bin/sh -d /var/opt/gitlab git

lsblk
fdisk /dev/sdb
mkfs.ext4 /dev/sdb1
e2fsck -f -y /dev/sdb1
e2label /dev/sdb1 githome
mkdir /media/git-home
mount /dev/sdb1 /media/git-home/

cat /etc/fstab
/dev/disk/by-uuid/6102a767-4d58-422f-9537-640835c7c9db /home/git/ ext4 rw,noatime,nouser_xattr,barrier=0,data=ordered,errors=remount-ro 0 1

reboot


   50  apt install rsync
   51  cd /media/git-home/
   52  chown -R git:git .
   53  ls -al
   54  rm -rf lost+found/
   55  ls -al
   56  ls
   57  rm -rf *
   58  ls -al
   59  ls
   60  sync
   61  rm -rf ssh/
   62  cd
   63  umount /media/git-home/
   64  ls /dev/disk/by-uuid/
   65  ls -al  /dev/disk/by-uuid/
   66  nano /etc/fstab
   67  mount -a
   68  mount
   69  reboot
   70  top
   71  cd /home/git/
   72  ls
   73  cat gitlab/etc/gitlab/initial_root_password
   74  cp  gitlab/etc/gitlab/initial_root_password  gitlab/
   75  exit
   76  cd /
   77  find . -type d -name gitlab
   78  ls -al ./opt/gitlab/e
   79  ls -al ./opt/gitlab/
   80  find . -type d -name gitlab
   81  systemctl status gitlab
   82  gitlab-ctl status
   83  gitlab-ctl --version
   84  gitlab-ctl
   85  gitlab-ctl statu
   86  gitlab-ctl status
   87  lz4
   88  apt install lz4
   89  cd /opt/
   90  mv gitlab gitlab.old
   91  ln -sf /home/git/gitlab/opt/gitlab .
   92  cd /etc/
   93  ln -sf /home/git/gitlab/etc/gitlab .
   94  mv gitlab gitlab.old
   95  ln -sf /home/git/gitlab/etc/gitlab .
   96  cd /var/
   97  mv log/
   98  cd lo
   99  cd log/
  100  mc gitlab gitlab.old
  101  mv gitlab gitlab.old
  102  ln -sf /home/git/gitlab/var/log/gitlab .
  103  ls -al /opt/gitlab/
  104  /etc/gitlab/gitlab.rb
  105  nano /etc/gitlab/gitlab.rb
  106  ps -aux | grep 860
  107  killall git
  108  ps -aux | grep 860
  109  usermod -s /bin/sh -d /var/opt/gitlab git
  110  kill -9 860
  111  usermod -s /bin/sh -d /var/opt/gitlab git
  112  cd /opt/
  113  unlink gitlab
  114  cp -r gitlab.old gitlab
  115  kill -9 885
  116  usermod -s /bin/sh -d /var/opt/gitlab git
  117  reboot
  118  ls /var/cache/apt/archives/
  119  find  /var/cache/apt/archives/-name "*.deb"
  120  find  /var/cache/apt/archives/-name -name "*.deb"
  121  find  /var/cache/apt/archives/ -name "*.deb"
  122  su git
  123  cd
  124  su git
  125  ip addr
  126  dhclient
  127  cd /lib
  128  cd systemd/
  129  cd system
  130  grep -r Wait
  131  systemctl mask systemd-networkd-wait-online.service
  132  dhclient
  133  ip addr
  134  cat  /etc/netplan/00-installer-config.yaml
  135  apt purge netplan
  136  apt purge netplan.io
  137  ip addr
  138  ip route
  139  resolvectl
  140  cd /etc/systemd/network
  141  nano ens35.network
  142  systemctl enable --now systemd-networkd
  143  reboot
  144  exit
  145  chsh
  146  mount
  147  ls -al /home/git/
  148  df -h  /home/git/
  149  gitlab-rake "gitlab:password:reset"
  150  nano /etc/hosts
  151  sudo apt install git-core --no-install-recommends
  152  cat /etc/passwd
  153  cat /etc/passwd | grep git
  154  usermod -d  /home/git git
  155  gitlab-ctl stop
  156  usermod -d  /home/git git
  157  gitlab-ctl start
  158  nano /etc/hosts
  161  exit
  162  cd
  163  su hs
  164  exit
  165  history


      1  sudo passwd
    2  cd /lib
    3  cd systemd/
    4  cd system
    5  systemctl disable --now apt-daily*
    6  su
    7  su
    8  free
    9  su
   10  ping 172.22.1.118
   11  ip addr
   12  ping 172.22.1.118
   13  ssh hs@172.22.1.118
   14  scp -r  hs@172.22.1.118:~/.ssh ssh
   15  ls
   16  ls .ssh/
   17  cp ssh/* .ssh/
   18  ssh hs@172.22.1.118
   19  ls .ssh/
   20  cat .ssh/id_ed25519.pub
   21  cd /
   22  touch /media/git-home/hello
   23  ls -al /media/git-home/
   24  rm -rf /media/git-home/hello
   25  rsync -avz . /media/git-home/
   26  cd /
   27  rsync -avz /home/git/ /media/git-home/
   28  mkdir gitlab
   29  cd gitlab/
   30  mkdir -p var/log/ etc/ opt/
   31  cp -r /var/log/gitlab var/log/
   32  cp -r /etc/gitlab etc/
   33  sudo cp -r /etc/gitlab etc/
   34  sudo rm -rf *
   35  sudo mkdir var/log/gitlab etc/gitlab opt/gitlab
   36  sudo mkdir -p var/log/gitlab etc/gitlab opt/gitlab
   37  sudo rsync -avz /etc/gitlab/ etc/gitlab/
   38  sudo rsync -avz /opt/gitlab/ opt/gitlab/
   39  df -h .
   40  sudo rsync -avz /o
   41  sudo rsync -avz /var/log/gitlab/ var/log/gitlab/
   42  ls -al
   43  cd ..
   44  tar -czf gitlab.origin.tar.gz gitlab
   45  rm -rf gitlab.origin.tar.gz
   46  sudo tar -czf gitlab.origin.tar.gz gitlab
   47  sudo tar -cf - gitlab | lz4 > gitlab.origin.tar.gz
   48  sudo tar -cf - gitlab | lz4 > gitlab.origin.tar.lz4
   49  ls -al
   50  exit
   51  pwd
   52  git clone git@git.hsekri.com:linux/qemu.git
   53  ip addr
   54  git clone git@git.hsekri.com:linux/qemu.git
   55  ssh-keygen
   56  rm .ssh/id_rsa*
   57  ssh-keygen
   58  cat .ssh/id_rsa.pub
   59  git clone git@git.hsekri.com:linux/qemu.git
   60  git clone haithem.sekri@git@git.hsekri.com:linux/qemu.git
   61  ssh-keygen ed25519
   62  ssh-keygen -t  ed25519
   63  cat .ssh/id_ed25519
   64  cat .ssh/id_ed25519.pub
   65  git clone git@git.hsekri.com:linux/qemu.git
   66  exit
   67  /home/hs/.ssh/id_rsa
   68  cat
   69  exit
   70  su hs
   71  su
   72  exit
   73  source ~/.git.hsekri.com.token
