# .bashrc

# /--------------------------------------------------------------------------\
# This file is a symbolic link to /home/developer/Software/bashrc
#
# BY EDITING THIS FILE, YOU AGREE, UNDER PENALTY OF SHAME, TO COMMIT CHANGES
# TO THE GIT REPO IN /home/developer/Software/
#
# cd /home/developer/Software/
# git add bashrc
# git commit -m "<Description of change>"
# git push # OPTIONAL ; backs up changes on GitHub
# cd -  # as you were
# \--------------------------------------------------------------------------/

# developer's primary group is observer, so by default files should
# not be group-writable
umask 0022

# Source global definitions
if [ -f /etc/bashrc ]; then
	. /etc/bashrc
fi

# User specific environment
export ANT_HOME=/home/developer/java/build_tools/apache-ant-1.9.16
export JAVA_HOME=/home/developer/java/build_tools/jdk-14.0.2
# JAVA_HOME also set in /home/developer/java/build_tools/netbeans-12.5/netbeans/etc/netbeans.conf
# also linked to /home/developer/bin/java14

if ! [[ "$PATH" =~ "$HOME/.local/bin:$HOME/bin:" ]]
then
    PATH="$HOME/.local/bin:$HOME/bin:$PATH"
fi

# add our software to the front of the path
PATH="$HOME/Software/bin:$PATH"
PATH="$HOME/Software/run:$PATH"

PATH="/usr/pgsql-15/bin:$PATH"  # pg_config should be used from here, not anaconda; needed for Q3C installation

export PATH

#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/anaconda3/lib  # breaks psql

PYTHONPATH="$HOME/Software/Python"
export PYTHONPATH

# Data for sky background calculator used by rubin_sim package used by ETC
export RUBIN_SIM_DATA_DIR="/data/rubin_sim/"

# Uncomment the following line if you don't like systemctl's auto-paging feature:
# export SYSTEMD_PAGER=

# User specific aliases and functions

alias python="/usr/bin/python3.9"
alias pip="/usr/bin/pip3.9"

alias vi="vim"
alias ls="ls -h --color=auto"
alias vimacro="vi ~/.bashrc"
alias sourcemacro="source ~/.bashrc"
alias chaz="cd ~/Software/Python"
alias cs="cd ~/cshapiro"
alias csac="cd ~/cshapiro/acam_skyinfo"
alias ja="cd ~/Software/java/ngps/ngps"
alias repos='echo " "; for d in acam_skyinfo ETC OTM; do echo "-- $d ----------------"; cd $d; git status; echo " "; cd - >/dev/null; done'
alias juno="jupyter notebook --no-browser --log-level=ERROR --notebook-dir=/home/developer/cshapiro"

alias commitchaz='GIT_COMMITTER_NAME="Chaz Shapiro" GIT_COMMITTER_EMAIL="charles.a.shapiro@jpl.nasa.gov" git commit --author="Chaz Shapiro <charles.a.shapiro@jpl.nasa.gov>" '


#alias ngps-observe-devel='/home/developer/Software/java/ngps/ngps/bin/ngps observe'
alias ngps-observe-devel='cd /home/developer/Software/java/ngps/ngps; java14 -jar ./dist/ngps.jar OBSERVE'

# >>> conda initialize >>>
# !! Contents within this block are managed by 'conda init' !!
#__conda_setup="$('/home/developer/anaconda3/bin/conda' 'shell.bash' 'hook' 2> /dev/null)"
#if [ $? -eq 0 ]; then
#    eval "$__conda_setup"
#else
#    if [ -f "/home/developer/anaconda3/etc/profile.d/conda.sh" ]; then
#        . "/home/developer/anaconda3/etc/profile.d/conda.sh"
#    else
#        export PATH="/home/developer/anaconda3/bin:$PATH"
#    fi
#fi
#unset __conda_setup
# <<< conda initialize <<<

