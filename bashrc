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

# Print the current software symlink and branch
# for interactive shells only:
if [[ $- =~ "i" ]]; then
   echo "NOTE! current software branch:" | awk '{print "\033[7;31m"$0"\033[0m"}'
   pushd $HOME/Software
   git branch
   popd # Go back to last directory
fi

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

### User specific aliases and functions

# Python
alias python="/usr/bin/python3.9"
alias pip="/usr/bin/pip3.9"

# Bash / CentOS
alias vi="vim"
alias ls="ls -h --color=auto"
alias vimacro="vi ~/.bashrc"
alias sourcemacro="source ~/.bashrc"
alias noresize='gsettings set org.gnome.mutter edge-tiling false'

# Chaz workspace
alias chaz="cd ~/Software/Python"
alias cs="cd ~/cshapiro"
alias csac="cd ~/cshapiro/acam_skyinfo"
alias ja="cd ~/Software/java/ngps/ngps"
alias repos='echo " "; for d in acam_skyinfo ETC OTM; do echo "-- $d ----------------"; cd $d; git status; echo " "; cd - >/dev/null; done'
alias juno="jupyter notebook --no-browser --log-level=ERROR --notebook-dir=/home/developer/cshapiro"

alias commitchaz='GIT_COMMITTER_NAME="Chaz Shapiro" GIT_COMMITTER_EMAIL="charles.a.shapiro@jpl.nasa.gov" git commit --author="Chaz Shapiro <charles.a.shapiro@jpl.nasa.gov>" '
#alias ds9last='cat `ls | tail -1` | xpaset FLEX multiframe'
#alias ds9last='last=`ls | tail -1`; echo $last; cat $last | xpaset FLEX multiframe'

# FITS files
alias lastfits='ls *.fits -Art | tail -n 1'
#alias lastfits2='ls *.fits -Art | tail -n 2 | head -n 1'  # next to last
alias ds9last='/home/developer/Software/Python/FITS_tools/ds9last.sh 1'
alias ds9last1='/home/developer/Software/Python/FITS_tools/ds9last.sh 1'
alias ds9last2='/home/developer/Software/Python/FITS_tools/ds9last.sh 2'
#alias ds9lastGo='/home/developer/Software/Python/FITS_tools/ds9-script.py `lastfits`'
#alias ds9last2='/home/developer/Software/Python/FITS_tools/ds9-script.py `lastfits2`'
#alias ds9last='lf=`lastfits`; busy=`lsof -w -c camerad | grep $lf | wc -l` ; if [[ $busy -gt 0 ]]; then echo FILE NOT READY!!!  SO IMPATIENT!  Try ds9last2; fi; ds9lastGo'

#alias ngps-observe-devel='/home/developer/Software/java/ngps/ngps/bin/ngps observe'
alias ngps-observe-devel='cd /home/developer/Software/java/ngps/ngps; java14 -jar ./dist/ngps.jar OBSERVE'
alias ngps-plan-devel='cd /home/developer/Software/java/ngps/ngps; java14 -jar ./dist/ngps.jar PLAN'

# coordinate helpers
alias getcoords_deg='~/Software/run/radec.py `tcs getcoords`'
alias getslit_deg='FPoffsets SCOPE SLIT `getcoords_deg` `tcs getcass | cut -f1 -d " "`'
alias getacam_deg='FPoffsets SCOPE ACAM `getcoords_deg` `tcs getcass | cut -f1 -d " "`'

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

