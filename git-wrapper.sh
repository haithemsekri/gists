#!/bin/bash

#cat ~/.git/github.com
#GIT_TOK=""
#GIT_USR=""
#GIT_EMAIL=""
#GIT_DNS="github.com"
#GIT_URL="https://$GIT_USR:$GIT_TOK@$GIT_DNS"

git_wrapper() {
  [[ -z "$1" ]] && echo "No passed argument" && /usr/bin/git --help && return 0
  local git_op="$1"
  shift

  local git_url=""
  local git_args=""

  if [[ "$git_op" == "clone" ]]; then
    for arg in "$@"; do
      if [[ "$arg" == "https://"* ]]; then
        local git_url_dns="$(echo $arg | sed 's/https:\/\///g' | tr ':' '/' | cut -d '/' -f 1)"
        #git_url_path="$(echo $git_url | sed 's/https:\/\///g' | tr ':' '/' | sed "s/$git_url_dns\///g")"
        source ~/.git/$git_url_dns
        git_url="$(echo $arg | sed "s/$git_url_dns/$GIT_USR:$GIT_TOK@$git_url_dns/g")"
      elif [[ "$arg" == "http://"* ]]; then
        local git_url_dns=$(echo $arg | sed 's/http:\/\///g' | tr ':' '/' | cut -d '/' -f 1)
        #git_url_path="$(echo $git_url | sed 's/https:\/\///g' | tr ':' '/' | sed "s/$git_url_dns\///g")"
        source ~/.git/$git_url_dns
        git_url="$(echo $arg | sed "s/$git_url_dns/$GIT_USR:$GIT_TOK@$git_url_dns/g")"
      elif [[ "$arg" == "git@"* ]]; then
        git_url="$arg"
      elif [[ "$arg" == "git://"* ]]; then
        git_url="$arg"
      else
        git_args="$git_args $arg"
      fi
    done
  else
    git_args="$@"
  fi

  echo RUN: /usr/bin/git $git_op $git_url $git_args
  /usr/bin/git $git_op $git_url $git_args
}


git_config_wrapper() {
  [[ ! -d ".git" ]] && return 0

  local git_url="$(/usr/bin/git config --get remote.origin.url)"

  ls ~/.git | while read l; do
    echo $l
    if [[ "$git_url" == *"$l"* ]]; then
      echo $git_url
      source ~/.git/$l
      /usr/bin/git config user.name $GIT_USR
      /usr/bin/git config user.email $GIT_EMAIL
      break
    fi
  done
  /usr/bin/git config branch.master.pushRemote  true
  /usr/bin/git config --get  user.name
  /usr/bin/git config --get  user.email
}

alias git='git_wrapper'

#git clone https://github.com/haithemsekri/gists.git --depth 1
#git_wrapper clone git@github.com:haithemsekri/gists.git --depth 1
#git_wrapper clone git://git.kernel.org/pub/scm/bluetooth/obexd.git --depth 1
#git_wrapper clone http://source.codeaurora.org/external/imx/aosp/platform/external/skia --depth 1
