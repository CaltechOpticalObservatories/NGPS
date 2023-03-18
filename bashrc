# .bashrc

# /--------------------------------------------------------------------------\
# This file is a symbolic link to /home/developer/Software/bashrc
#
# By editing this file, you agree, under penalty of shame, to commit changes
# to the Git repo in /home/developer/Software/
#
# cd /home/developer/Software/
# git add bashrc
# git commit -m "<Description of change>"
# git push # backs up changes on GitHub
# cd -  # as you were
# \--------------------------------------------------------------------------/

# Source global definitions
if [ -f /etc/bashrc ]; then
	. /etc/bashrc
fi

# User specific environment
export ANT_HOME=/home/developer/Software/java/build_tools/apache-ant-1.9.16
export JAVA_HOME=/home/developer/Software/java/build_tools/jdk-14.0.2
if ! [[ "$PATH" =~ "$HOME/.local/bin:$HOME/bin:" ]]
then
    PATH="$HOME/.local/bin:$HOME/bin:$PATH"
fi

# add our software to the front of the path
PATH="$HOME/Software/bin:$PATH"
PATH="$HOME/Software/run:$PATH"

PATH="/usr/pgsql-15/bin:$PATH"  # pg_config should be used from here, not anaconda; needed for Q3C installation

export PATH

PYTHONPATH="$HOME/Software/Python"
PYTHONPATH="$PYTHONPATH:/home/developer/anaconda3/lib/python3.9"
PYTHONPATH="$PYTHONPATH:/home/developer/anaconda3/lib/python3.9/site-packages"
export PYTHONPATH

# Uncomment the following line if you don't like systemctl's auto-paging feature:
# export SYSTEMD_PAGER=

# User specific aliases and functions

alias vi="vim"
alias ls="ls -h --color=auto"
alias vimacro="vi ~/.bashrc"
alias sourcemacro="source ~/.bashrc"

# >>> conda initialize >>>
# !! Contents within this block are managed by 'conda init' !!
__conda_setup="$('/home/developer/anaconda3/bin/conda' 'shell.bash' 'hook' 2> /dev/null)"
if [ $? -eq 0 ]; then
    eval "$__conda_setup"
else
    if [ -f "/home/developer/anaconda3/etc/profile.d/conda.sh" ]; then
        . "/home/developer/anaconda3/etc/profile.d/conda.sh"
    else
        export PATH="/home/developer/anaconda3/bin:$PATH"
    fi
fi
unset __conda_setup
# <<< conda initialize <<<

